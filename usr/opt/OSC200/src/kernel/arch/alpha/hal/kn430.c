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
 * Modification History:
 *
 * ajb   07-Oct-1991   Adapted from ka_ruby.c
 *
 */

#include <sys/types.h>			/* for u_int and freinds */

#include <sys/proc.h>			/* for task_t */
#include <sys/lwc.h>			/* for lwc types and priorities */
#include <sys/table.h>			/* for INTR_MAX_LEVEL, INTR_TYPE_* */

#include <vm/vm_kern.h>			/* for kernel_map */

#include <sys/kernel.h>			/* for hz */
#include <mach/machine.h>		/* for machine_slot */
#include <machine/clock.h>		/* for TODRZERO, YRREF */
#include <machine/rpb.h>		/* for definition of Alpha HWRPB */
#include <machine/entrypt.h>		/* for hwrpb_addr */

#include <dec/binlog/errlog.h>		/* Errlog/PAL logout frames */

#include <hal/kn430.h>			/* KN430 CSRs */
#include <hal/dc21064.h>		/* DC21064 CSRs */
#include <data/if_te_data.c>		/* For getinfo for tegc cobra_sw structure */

#include <io/dec/mbox/mbox.h>		/* Mailbox primitives */
#include <io/dec/fbus/fbusreg.h>	/* for FBUS_TIMEOUT */

#include <hal/cpuconf.h>
#include <hal/lmf_smm.h>

#ifdef lint
#include <machine/scb.h>
#include <kern/lock.h>
#include <machine/reg.h>
#include <kern/sched_prim.h>
#include <io/common/devdriver.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/dk.h>
#include <sys/vm.h>
#include <sys/conf.h>
#include <sys/reboot.h>
#include <sys/devio.h>
#include <sys/user.h>
#include <mach/boolean.h>
#include <sys/vmmac.h>
#include <sys/time.h>
#include <machine/cpu.h>
#include <machine/psl.h>
#include <sys/syslog.h>
#endif /* lint */

/*
 * Prestoserve support definitions - Start
 */
#include <sys/presto.h>
#include "presto.h"
 
/* #define PRESTO_DEBUG */
 
#ifdef PRESTO_DEBUG
 
#define PRD(x)		printf x
#define PRD_WR(x)	if (prdebug_write) printf x
#define PRD_WR_OFF	prdebug_write = 0
#define PRD_ZR(x)	if (prdebug_zero) printf x
#define PRD_ZR_OFF	prdebug_zero = 0
#define PRD_CF(x)	if (prdebug_cf) printf x
#define PRD_CF_OFF	prdebug_cf = 0
#define PRD_CF_ON	prdebug_cf = 1
#define PRD_BE(x)	if (prdebug_be) printf x
#define PRD_BE_OFF	prdebug_be = 0
#define PRD_BE_ON	prdebug_be = 1
 	
int prdebug_cf = 1;
int prdebug_be = 1;
int prdebug_write = 1;
int prdebug_zero = 1;

#else PRESTO_DEBUG
#define PRD(x)
#define PRD_WR(x)
#define PRD_ZR(x)
#define PRD_CF(x)
#define PRD_WR_OFF
#define PRD_ZR_OFF
#define PRD_CF_OFF
#define PRD_CF_ON	
#define PRD_BE(x)
#define PRD_BE_OFF
#define PRD_BE_ON
#endif PRESTO_DEBUG
 
int	cobra_nvram_status_v;	/* nvram status value */
int	cobra_nvram_size;	/* Adjusted size value */
caddr_t	cobra_nvram_start_adr;	/* Adjusted start address */
 
extern void bzero();
extern void bcopy();			/* used for moving data into nvram */
 
/*
 * cobra presto control and status register - it is addressed in the first
 * location of the second bank of memory.  CAD <23> is used to address this
 * register.  Not that's bit 25 on the cbus.
 *
 *	    7	     6	    5	4   3	  2	  1	  0
 *	-------------------------------------------------------
 *	| DMODE1 | DMODE2 | 1 | 1 | 1 |	BFAIL |	BDISC | BCHRG |
 *	-------------------------------------------------------
 *
 *	BFAIL - Battery Fail - Read Only - ACTIVE LOW
 *		1 means battery is OK
 *		0 means battery has failed and should be replaced
 *	BDISC - Battery Disconnect - Read/Special Write - Active High
 *		1 means battery disconnect circuit is armed and the 
 *		  battery will be disconnected on power fail
 *		0 means the battery will back up on power fial.
 *		On initial powerup or hardware reset, the battery disconnect 
 *		circuit is disabled.
 *		To enable batthery disconnect circuit a 5 bit sequential code
 *		 "11001" bus be written	to bit 1 of the CSR.  The battery 
 *		disconnect circuit can be disabled by writing any value to the 
 *		control register anytime after BDISC is set.  Software must 
 *		enable the battery disconnect circuit if the NVRAM does not 
 *		contain any Prestoserve data.
 *	BCHRG - Battery Charged - Read Only - ACTIVE HIGH
 *		1 means battery charged
 *		0 means battery charging
 *		If the bit is set at boot time, the Presto data may have been 
 *		lost.  If the cookie is set, try to recover anyway because 
 *		NVRAM is EDC protected.
 *
 *	DMODE1 and DMODE2 are for diagnostics only, MBZ for us.
 *
 *	Battery Failure if BCHRG 0 and BFAIL 0
 */
#define NVRAM_CSR_OFFSET   0x2000000

#define BFAIL		0x4
#define BDISC		0x2
#define BCHRG		0x1

#define BATTERY_STATUS_MASK 		0x7
#define BAT_STATUS_OKAY			0x4	/* the battery is ok */
#define BAT_STATUS_NO_BACK		0x2	/* the battery won't back up */
#define BAT_STATUS_CHARGED		0x1	/* the battery is charged */

/* BAT_STATUS_OKAY | BAT_STATUS_WILL_BACK | BAT_STATUS_CHARGED */

 
/*
 * Prestoserve support definitions - End
 */

/*****************************************************************************
 * EXTERNAL REFERENCES							     *
 *****************************************************************************/

/*
 * Interrupt translation table
 */
extern volatile int *system_intr_cnts_type_transl;

/*
 * Where to direct printfs to.
 */
extern int printstate;

/*
 * Performing probe.
 */
extern int mcheck_expected;

/*
 * Pointer to Alpha Hardware Restart Parameter Block.
 */
extern struct rpb *rpb;

/*
 * Pointers to CSR bases.  Start out physical, converted to virtual by
 * cobra_conf().
 */
extern struct cpu_csr *Cpu_regs[CBUS_CPU_SLOTS];
extern struct io_csr *Io_regs;
extern struct mem_csr *Mem_regs[CBUS_MEM_SLOTS];

/*
 * IIC Bus Controller.
 */
extern struct controller *Iic_ctlr;

/*
 * First task, needed to spawn kernel threads.
 */
extern task_t first_task;

/*
 * Panic indicator -- nonzero if panic has occurred.
 */
extern char *panicstr;

/* ROUTINES */
extern void null_mbox_cmd();
extern void fbus_mbox_cmd();

/*
 * These should be declared in header files.  Defined here for now.
 */
extern int mfpr_whami();
extern void enable_spls();
extern void panic(char *panicstr);
extern int swap_ipl(int new_ipl);
extern void mb();
extern int BADADDR();
extern int ib_config_cont(char *nxv, char *nxp, u_int slot, char *name,
			  struct bus *bus, long scb_vec_addr);
extern void bzero(void *ptr, long size);
extern void bcopy(void *src, void *dest, int len);
extern boolean_t mbox_badaddr(caddr_t addr, int len, mbox_t mbp);
extern int cvt_sec_to_tm(long seconds, struct tm *tm);
extern long cvt_tm_to_sec(struct tm *tm);
extern void mb();
extern void microdelay(int usecs);
extern void halt();
extern char *binlog_alloc(int size, int priority);
extern int binlog_valid(char *buf);
extern void mtpr_mces(long mces);
extern timeout(int (*fun)(), caddr_t arg, int hz);
extern int alpha_scrub_long(vm_offset_t ptr, int retries);
extern int mbox_setup(int cmd, u_int type_as, mbox_t mbp,
		      u_long remaddr, u_long wrtdata);

/*****************************************************************************
 * LOCAL STRUCTURE DEFINITIONS						     *
 *****************************************************************************/

/*
 * The machine check handler relies on two data structures.
 *
 * The kn430_mcheck_control structure is a per-cpu structure which holds
 * control parameters and state for the machine check parse.  Pointers to
 * the error log frames are stored here, so that routines don't have to
 * reparse the error log frame to read a saved register.
 *
 * The "error log scratch buffer" is used to assemble a complete error log
 * packet before binlog_alloc() is called.  This solves two problems:
 *
 *   1. There is no way to extend a binlog packet after allocating it.
 *   2. It is necessary to snapshot the register state early in the parse,
 *	without waiting on spinlocks, to maximize the chance that at least
 *	one CPU will see the complete system state near the time of a
 *	system error interrupt.  Keeping a wired down error log scratch buffer
 *	makes this possible.
 *
 * The packet is written piece by piece into the buffer.  The possible packet
 * formats are determined by the possible parses, which in turn are determined
 * by the SCB vector (only three values).
 *
 * NOTE: Once we are confident that the error parse is correct, and have gained
 *       experience with system error analysis, it may be worthwhile to have
 *	 the parse decide to omit packets for functioning FRUs.  This would
 *	 complicate the parse and error logging, but would save space.
 */

/*
 * The machine check handler is called at two different priority levels,
 * non-maskable (higher than SPLHIGH) and SPLHIGH.  The constant
 * KN430_MCHECK_IPLS is used to size arrays of data that are accessed at all
 * machine check handler IPLs.
 */
#define KN430_MCHECK_IPLS 2

/*
 * KN430 Machine check handler control structure.  Maintains pointers to
 * the start of the error log buffer and to specific frames within the buffer.
 * Each CPU needs two of these structures, one for unmaskable processor
 * uncorrectable error interrupts, and another for other error interrupts
 * which are delivered at SPLEXTREME.  Using separate copies reduces the
 * chance of losing data if a processor error is encountered while handling
 * an error interrupt.
 */
struct kn430_mcheck_control {
	struct el_rec *mctl_elrec;	/* Pointer to error log record. */

	/*
	 * Pointers to frames in the errorlog scratch buffer.  These get
	 * filled in as frames are captured.
	 */
	struct kn430_mcheck_control_frames {
		struct el_cobra_mcheck_event_header	*mcfr_hdr;
		struct el_cobra_frame_mcheck		*mcfr_mcheck;
		struct el_cobra_frame_proc_bcache_corr	*mcfr_pbccorr;
		struct el_cobra_frame_other_cpu		*mcfr_ocpu;
		struct el_cobra_frame_io		*mcfr_io;
		struct el_cobra_frame_memory	*mcfr_mem[CBUS_MEM_SLOTS];
	} mctl_frame;
};

/*
 * Definition of the global kn430_mcheck_control structure.  Need one for
 * each possible CPU on a DEC 4000 system.  Also need one for each possible
 * IPL level (there are two: SPLEXTREME and unmaskable).  The disptach vector
 * determines whether the IPL is unmaskable or SPLEXTREME.
 */
#if NCPUS > 1
struct kn430_mcheck_control
	kn430_mcheck_control[CBUS_CPU_SLOTS][KN430_MCHECK_IPLS];
#define KN430_MCHECK_CONTROL(cpuid, vector) \
	&kn430_mcheck_control[cpuid][KN430_MCHECK_IPL(vector)]
#else /* NCPUS <= 1 */
struct kn430_mcheck_control
	kn430_mcheck_control[KN430_MCHECK_IPLS];
#define KN430_MCHECK_CONTROL(cpuid, vector) \
	&kn430_mcheck_control[KN430_MCHECK_IPL(vector)]
#endif /* NCPUS <= 1 */

/*
 * Structure for maintaining scratch copies of error log frames while they
 * are being built.  Each CPU needs two buffers, since machine checks are
 * nonmaskable.  One buffer is used for machine checks, the other is used for
 * other errors, which are all delivered at SPLEXTREME on the DEC 4000.
 */
struct kn430_scratch_buffer {
	vm_offset_t	sb_ptr;		/* Next unused byte. */
	long		sb_rmng;	/* Bytes remaining. */
	vm_offset_t	sb_start;	/* Start of buffer. */
	long		sb_len;		/* Total length of scratch buffer. */
};

#define KN430_MCHECK_IPL(vector) ((vector == KN430_VECTOR_PROC_FATAL) ? 1 : 0)

#if NCPUS > 1
struct kn430_scratch_buffer 
	kn430_scratch_buffer[CBUS_CPU_SLOTS][KN430_MCHECK_IPLS];
#define KN430_SCRATCH_BUFFER(cpuid, vector) \
	&kn430_scratch_buffer[cpuid][KN430_MCHECK_IPL(vector)]
#else /* NCPUS <= 1 */
struct kn430_scratch_buffer
	kn430_scratch_buffer[KN430_MCHECK_IPLS];
#define KN430_SCRATCH_BUFFER(cpuid, vector) \
	&kn430_scratch_buffer[KN430_MCHECK_IPL(vector)]
#endif /* NCPUS <= 1 */

/*
 * Extension of the error logger's footprint structure.
 *
 * Needed for checkpointing of footprints.  If it were possible to log the
 * footprints at normal system shutdown time, checkpointing would be
 * unnecessary.
 *
 */
struct kn430_edc_fprint_entry {
	struct kn430_edc_fprint cfe_fp;
	int  cfe_flags;			/* State flags. */
};
extern struct kn430_edc_fprint_entry *kn430_fprint;

/*
 * Flags used to track footprint checkpoint state.
 */
#define KN430_FPRINT_CHANGED (1L << 0)	/* Changed since last checkpoint. */
#define KN430_FPRINT_DOMAIN  (1L << 1)	/* Domain grew since checkpoint. */
#define KN430_FPRINT_CHKPTD  (1L << 2)	/* Checkpointed at least once. */


/*****************************************************************************
 * FORWARD DECLARATIONS / CONTENTS					     *
 *****************************************************************************/

extern int cobra_init();
extern int cobra_conf();
static void kn430_read_palcode_version();
extern int cobra_config_cbus(struct bus *cbus);
extern int cbus_probe(struct bus *cbus);
extern int lbusconfl1(struct bus *cbus, struct bus *lbus);
extern int lbusconfl2(struct bus *cbus, struct bus *lbus);
extern long cobra_badaddr
	(caddr_t addr, int len, struct bus_ctlr_common *ptr);
static void kn430_mcheck_init(int cpuid);
extern long kn430_machcheck
	(long vector, struct el_cobra_data_mcheck *lgt);
extern long kn430_proccorr
	(long vector, struct el_cobra_data_proc_bcache_corr *lgt);

static void kn430_parse_proccorr
	(struct kn430_mcheck_control *mctl, 
	 struct el_cobra_data_proc_bcache_corr *lgt,
	 struct kn430_scratch_buffer *sb);
static void kn430_parse_sysmcheck
	(struct kn430_mcheck_control *mctl, 
	 struct el_cobra_data_mcheck *lgt,
	 struct kn430_scratch_buffer *sb);
static void kn430_parse_procmcheck
	(struct kn430_mcheck_control *mctl, 
	 struct el_cobra_data_mcheck *lgt,
	 struct kn430_scratch_buffer *sb);
static long kn430_dont_enable_probing
	(struct el_cobra_mcheck_event_header *elch,
	 int cpuid,
	 int ocpuid);


static long kn430_parse_c3_tag_par
	(u_long bcue, u_long bcuea,
	 struct el_cobra_mcheck_event_header *elch,
	 int cpuid);
static u_long even_parity(u_long bits);
static long kn430_parse_c3_syndromes
	(u_long bcue,
	 struct el_cobra_mcheck_event_header *elch,
	 int cpuid);
static long kn430_parse_bcache_uncorr
	(u_long bcue, struct el_cobra_mcheck_event_header *elch, int cpuid);
static long kn430_parse_ocpu_bcache_uncorr
	(int cpuid,
	 struct el_cobra_frame_mcheck *elcmc,
	 int ocpuid,
	 struct el_cobra_frame_other_cpu *elco,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_c3_rd_par
	(u_long cbe, struct el_cobra_mcheck_event_header *elch, int cpuid);
static long kn430_parse_c3_wd_par
	(u_long cbe, struct el_cobra_mcheck_event_header *elch, int cpuid);
static long kn430_parse_c3_ca_par(u_long cbe,
				  struct el_cobra_mcheck_event_header *elch,
				  int cpuid);
static long kn430_parse_io_gate_sync
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_bus_sync
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_wd_noack
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_wd_par
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_rd_uncorr
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_rd_par
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_ca_noack
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_ca_par
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);

static long kn430_parse_fbus_dma
	(u_long ferr1, struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_fbus_mbx
	(u_long cerr1, struct el_cobra_mcheck_event_header *elch);

static long kn430_parse_lbus_dma
	(u_long lerr1, struct el_cobra_mcheck_event_header *elch);

static long kn430_parse_mem_sync
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_mem_ca_par
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_mem_edc
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_mem_wd_par
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_mem_uncorr
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_io_mem_uncorr
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_parse_cpu_mem_uncorr
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch,
	 u_long fill_addr);

static void kn430_mcheck_init(int cpuid);
static void cobra_init_frames(struct kn430_mcheck_control *mctl);
static struct el_cobra_frame_mcheck *kn430_get_frame_mcheck
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_data_mcheck *lgt,
	 struct kn430_scratch_buffer *sb);
static struct el_cobra_frame_proc_bcache_corr *kn430_get_frame_pbccorr
 	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_data_proc_bcache_corr *lgt,
	 struct kn430_scratch_buffer *sb);

static struct el_cobra_frame_other_cpu *kn430_get_frame_ocpu
        (struct kn430_mcheck_control *mctl, int ocpuid,
	 struct kn430_scratch_buffer *sb);
static struct el_cobra_frame_io *kn430_get_frame_io
	(struct kn430_mcheck_control *mctl,
	 struct kn430_scratch_buffer *sb);
static struct el_cobra_frame_memory *kn430_get_frame_mem
 	(struct kn430_mcheck_control *mctl,
	 int slot,
	 struct kn430_scratch_buffer *sb);

static void kn430_edc_init();
extern void kn430_edc_reset();
static long kn430_handle_edcs
	(struct kn430_mcheck_control *mctl,
	 struct el_cobra_mcheck_event_header *elch);
static long kn430_handle_edc
	(struct el_cobra_frame_memory *elcm,
	 int slot, int high_cmic);
static void kn430_log_edc
	(struct el_cobra_frame_memory *elcm, int slot, 
	 int high_cmic, long timestamp);

static struct kn430_edc_fprint_entry *kn430_find_footprint
	(struct kn430_edc_fprint_entry *fp, long fp_max, u_long signature);

static struct kn430_edc_fprint_entry
	*kn430_alloc_footprint
		(struct kn430_edc_fprint_entry *fp, long fp_max, 
		 long signature, long pa, long timestamp);

static void kn430_init_footprint
	(struct kn430_edc_fprint_entry *fp, long signature, long pa,
	 long timestamp);
static void kn430_update_footprint
	(struct el_cobra_frame_memory *elcm,
	 struct kn430_edc_fprint_entry *fp, int high_cmic, long timestamp);

static int kn430_count_footprints();
extern int kn430_checkpoint_footprints();

static void kn430_suppress_footprint
	(int slot, long pa, int high_cmic,
	 struct kn430_edc_fprint_entry *fp);

static void kn430_disable_board_edcs(int slot, int high_cmic);
static long kn430_start_edc_packet
	(struct el_rec **p_elrec, long fprints,	long rsn, long crd_flags);

static long kn430_edc_timestamp();
static long kn430_mcmd
	(struct el_cobra_frame_memory *elcm,
	int high_cmic);
static long kn430_pa(struct el_cobra_frame_memory *elcm, int high_cmic);
static long kn430_footprint_signature
	(struct el_cobra_frame_memory *elcm, int high_cmic);

static void kn430_edc_create_thread(caddr_t dummy);
static void kn430_edc_thread();

static void kn430_copy_fru_table();


static long kn430_mcheck_dispatch
	(struct kn430_mcheck_control *mctl,
	 struct kn430_scratch_buffer *sb,
	 int severity, long suppress);

static void kn430_log_mcheck
	(struct kn430_mcheck_control *mctl, struct el_rec *elrec);

static vm_offset_t kn430_alloc_elbuf
	(struct kn430_scratch_buffer *sb, long size);
static void *kn430_alloc_elbuf_frame
	(struct kn430_scratch_buffer *sb, long body_size, long fid);
static struct el_rec *kn430_copy_to_errlog(struct kn430_scratch_buffer *sb);
static void kn430_init_mcheck_event_header
	(register struct el_cobra_mcheck_event_header *elch,
	 register int size,
	 register int severity,
	 register int cpuid,
	 register long vector);
static struct kn430_scratch_buffer *kn430_errlog_start
	(struct kn430_mcheck_control *mctl, int cpuid, long vector);

static void kn430_free_logout(u_long mask);

static int kn430_summarize_errors(struct kn430_mcheck_control *mctl);
static void kn430_dump_errlog_packet
	(struct kn430_mcheck_control *mctl, struct el_rec *elrec);
static long cobra_cvt_int_to_hexstr(char *buf, int digits, long value);
static long kn430_dump_el_hdr(register struct el_rec *elrec);
static void kn430_dump_cobra_hdr(struct el_cobra_mcheck_event_header *elch);
static void kn430_dump_io(struct el_cobra_frame_io *elci);
static void kn430_dump_mcheck(struct el_cobra_frame_mcheck *elcmc, int cpuid);
static void kn430_dump_proc_bcache_corr
	(struct el_cobra_frame_proc_bcache_corr *elcpb, int cpuid);
static void kn430_dump_ocpu(struct el_cobra_frame_other_cpu *elco);
static void kn430_dump_memory(struct el_cobra_frame_memory *elcm);
static void kn430_dump_unidentified(struct el_cobra_frame_header *elcf);
static void kn430_dump_tail(long *start, long offset, long rmng);

extern int cobra_consprint();

void ee_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void uart_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void siop_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void toy_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void tgec_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void enet_addr_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void srl_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);
void flash_mbox_cmd(mbox_t mbp, u_int rwflag, u_int wmask_as);

static int kn430_watch_conf();
static int kn430_read_watch(u_long remaddr);
static int kn430_write_watch(u_long remaddr, int wdata);
extern long cobra_readtodr();
extern int cobra_writetodr(long todr);

/*****************************************************************************
 * DEC 4000 PLATFORM INITIALIZATION					     *
 *****************************************************************************/

/*
 * Define mapping of interrupt lines with the type of interrupt.
 */
static int cobra_interrupt_type[INTR_MAX_LEVEL] = {
		INTR_TYPE_NOSPEC,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_SOFTCLK,
		INTR_TYPE_DEVICE,
		INTR_TYPE_DEVICE,
		INTR_TYPE_HARDCLK,
		INTR_TYPE_OTHER,
		INTR_TYPE_OTHER
};

static long kn430_palcode_type, kn430_palcode_version, kn430_palcode_revision;
static long kn430_firmware_major, kn430_firmware_minor;


/*
 * Cobra configuration routine.
 *
 * Called through CPU switch from alpha_init through setup_main, startup, 
 * configure, and machine_configure.
 */
int cobra_conf() 
{
	register struct bus *cbus;   /* internal bus (ibus) is the cbus */
	extern volatile int *system_intr_cnts_type_transl;

	/* Set up global interrupt type array to use our definitions. */
	system_intr_cnts_type_transl = cobra_interrupt_type;

	rpb = (struct rpb *)hwrpb_addr;

	master_cpu = mfpr_whami();
#if NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif /* NCPUS > 1 */
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_ALPHA;
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_DEC_4000;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].clock_freq = hz;

	kn430_mcheck_init(master_cpu);

	/* Read the PALcode version/revision  and report/log it */
	kn430_read_palcode_version();
	printf ("Firmware revision: %d.%d\nPALcode: %s version %d.%02d\n",
		kn430_firmware_major, kn430_firmware_minor,
		kn430_palcode_type == PALvar_OSF1 ? "OSF":"VMS",
		kn430_palcode_version, kn430_palcode_revision);

	/*
	 * Set up virtual addresses of CSRs before lowering IPL, so that the
	 * machine check handler can deal with pending system error interrupts.
	 */
	Io_regs = (io_csrp_t)PHYS_TO_KSEG(Io_regs);
	Cpu_regs[0] = (cpu_csrp_t)PHYS_TO_KSEG(Cpu_regs[0]);
	Cpu_regs[1] = (cpu_csrp_t)PHYS_TO_KSEG(Cpu_regs[1]);
	Mem_regs[0] = (mem_csrp_t)PHYS_TO_KSEG(Mem_regs[0]);
	Mem_regs[1] = (mem_csrp_t)PHYS_TO_KSEG(Mem_regs[1]);
	Mem_regs[2] = (mem_csrp_t)PHYS_TO_KSEG(Mem_regs[2]);
	Mem_regs[3] = (mem_csrp_t)PHYS_TO_KSEG(Mem_regs[3]);

	enable_spls();	/* Ready for spl to be lowered */
    	splextreme();

	if((system_bus = cbus = get_sys_bus("ibus")) == (struct bus *)0)
		panic("cobra_conf: no system bus");
        (*cbus->confl1)(-1, 0, cbus);
        (*cbus->confl2)(-1, 0, cbus);

	/* The SCB is set up.  Allow interrupts to be delivered. */
	(void) spl0();

 	/*
 	 * Set up prestoserve if configured
 	 */
 	if ( NPRESTO > 0 )
 		if (cobra_presto_config())
 			printf("kn430 presto configuration failed\n");
 
	kn430_watch_conf();
	return(0);
}
/*
 * Start Prestoserve platform specific code
 *
 */
/*
 * cobra_presto_config()
 * This routine sets up the presto interface and calls presto_init().
 * If presto_init fails, it returns non-zero.
 *
 * Return Value		0 	- Success
 *			~0 	- Failure
 */
cobra_presto_config()
{
	extern cobra_nvram_status();
	extern cobra_nvram_battery_status();
	extern cobra_nvram_battery_disable();
	extern cobra_nvram_battery_enable();

	extern void cobra_presto_write();
	extern void cobra_presto_zero();

	int rval;

	PRD(("cobra_presto_config\n"));
	/*
	 * Setup the cobra nvram status routines for the presto driver
	 */
	presto_interface0.nvram_status          = cobra_nvram_status;
	presto_interface0.nvram_battery_disable = cobra_nvram_battery_disable;
	presto_interface0.nvram_battery_enable  = cobra_nvram_battery_enable;
	presto_interface0.nvram_battery_status  = cobra_nvram_battery_status;
	
	/*
	 * The following interfaces were added to permit hardware dependent
	 * tailoring of presto transfers.
	 */
	presto_interface0.nvram_ioreg_read  = bcopy ;
	presto_interface0.nvram_ioreg_write = cobra_presto_write ;
	presto_interface0.nvram_block_read  = bcopy ;
	presto_interface0.nvram_block_write = cobra_presto_write ;
	presto_interface0.nvram_ioreg_zero  = cobra_presto_zero ;
	presto_interface0.nvram_block_zero  = cobra_presto_zero ;
	presto_interface0.nvram_min_ioreg   = sizeof(int) ;
	presto_interface0.nvram_ioreg_align = sizeof(int) ;
	presto_interface0.nvram_min_block   = sizeof(int) ;
	presto_interface0.nvram_block_align = sizeof(int) ;

	PRD(("cobra_presto_config - presto_interface0 initialized\n"));
	
	/*
	 * Do the presence check after setting the presto_interface routines
	 * to avoid a panic when a presto board is not present (battery status
	 * will be bad and presto not used)
	 */ 
	if (cobra_nvram_config() == 0 ) {	
		/* presto configured, but no NVRAM */
		printf("kn430: Prestoserver not configured - no nvram module!\n");
		return(1);
	}

	PRD(("cobra_presto_config - nvram module is there\n"));
	/*
	 * Call the presto initialization routine.
	 * 
	 * The first 1K of reserved NVRAM is for diags, console, etc.
	 * The 3rd parameter means mapped, the 4th means cached.
	 * The last parameter is an unsigned 32 bit unique 'system identifier'
	 * 	used by presto during a power up to determine if the NVRAM has been
	 *	moved to a different system.
	 */
	
#ifdef PRESTO_DEBUG
	if (rval = presto_init((cobra_nvram_start_adr + 1024L),
			(cobra_nvram_size - 1024), 
			1, 1, cobra_ssn()))
		PRD(("kn430: presto_init failed. Error 0x%x\n",rval));
	PRD(("cobra_presto_config - presto init succeeded\n"));
	PRD_BE_OFF;
#else
	rval = presto_init((cobra_nvram_start_adr + 1024L),(cobra_nvram_size - 1024), 1, 1, cobra_ssn());
#endif


	return(rval);
}
/* 
 * cobra_presto_cflush() - wrapper for cflush pal call.  The cflush works by
 * flushing the corresponding pfn.
 */
void
cobra_presto_cflush(destin, size)
	caddr_t destin;
	int size;
{
	long physdestin, physend ;
	int page_size;

	page_size = rpb->rpb_pagesize;

	PRD_CF(("cobra_presto_cflush: first time destin 0x%016x size 0x%x\n", destin,size));

	svatophys(destin, &physdestin);
	for (physend = physdestin + size; physend > physdestin; physdestin = physdestin + page_size)
		cflush((physdestin / page_size));	
	
	/*
	 * Need one more flush if this transfer crosses a page boundary
	 */
	if ((physdestin & (page_size-1)) < (physend & (page_size-1)))
		cflush((physdestin / page_size));	

	PRD_CF_OFF;
}

/*
 * Below are the support routines to move data into and out of NVRAM
 */
void
cobra_presto_write(source, destin, size)
	caddr_t	source, destin ;
	int	size;
{
	
	PRD_WR(("cobra_presto_write:"));
	PRD_WR((" 1st time source = 0x%l016x, dest = 0x%l016x, size = 0x%x\n", source, destin, size));
	
	bcopy(source, destin, size);	/* move the data into the nvram */
	cobra_presto_cflush(destin, size);

	PRD_WR_OFF;
}

void 
cobra_presto_zero(adr,size)
	caddr_t	adr;
	int	size;
{
	PRD_ZR(("cobra_presto_zero: 1st time adr = %l016x, size = %x  %d\n", adr, size, size));
	
	bzero(adr,size);
	cobra_presto_cflush(adr,size);

	PRD_ZR_OFF;
}



/*
 * cobra_nvram_config() - set the nvram parameters (global variables) size, physical and
 * virtual starting addr, diagnostic status, nvram battery structure
 *
 * Return Value		0	Failure
 *			~0	Success
 */
cobra_nvram_config()
{
	struct rpb_mdt *memdsc ;
	struct rpb_cluster *clusterdsc ;
	u_long cobra_presto_phystart ;
	int i ;
	extern struct rpb *rpb ;
	extern int cobra_nvram_size ;
	extern caddr_t cobra_nvram_start_adr ;
	
	nvram_batteries0.nv_nbatteries        = 1;	/* one battery	*/
	nvram_batteries0.nv_minimum_ok        = 1;	/* it must be good	*/
	nvram_batteries0.nv_primary_mandatory = 1;	/* primary must be OK */
	nvram_batteries0.nv_test_retries      = 1;	/* One test will do it */
	
	/*
	 * Find the memory descriptor that defines the location, size, and selftest
	 * results for the NVRAM
	 */
	memdsc = (struct rpb_mdt *)((char *)rpb + rpb->rpb_mdt_off);
	for (i = 0; i < (int) memdsc->rpb_numcl; i++) {
		clusterdsc = (struct rpb_cluster *)((char *)(memdsc->rpb_cluster) +
						    (i * sizeof(struct rpb_cluster))) ;
		PRD(("cobra_nvram_config: clusterdsc->rpb_usage = %l016x\n", clusterdsc->rpb_usage));
		if (clusterdsc->rpb_usage == CLUSTER_USAGE_NVRAM) {
			/* found the presto nvram cluster descriptor */
			break;
		}
	}
	if ( i == memdsc->rpb_numcl) {
		printf("cobra_nvram_config: No NVRAM memory descriptor present.\n");
		return(0);
	}

	/* 
	 * Find the self test status
	 * If pages tested < 0 nvram is dirty.
	 * If pages tested != page count, nvram is bad
	 * If pages tested == page count, nvram is good
	 */
	if (clusterdsc->rpb_pfntested == -1) {
		PRD(("cobra_nvram_config: NVRAM dirty, not tested\n"));
		cobra_nvram_status_v = NVRAM_RDONLY;
	} else if (clusterdsc->rpb_pfntested != clusterdsc->rpb_pfncount) {
		PRD(("cobra_nvram_config: NVRAM tested bad.\n"));
		cobra_nvram_status_v = NVRAM_BAD;
	} else if (clusterdsc->rpb_pfntested == clusterdsc->rpb_pfncount){
		PRD(("cobra_nvram_config: NVRAM tested okay.\n"));
		cobra_nvram_status_v = NVRAM_RDWR;
	} else {
		printf("cobra_nvram_config: Selftest results are ambiguous.\n");
		return(0);
	}

	/*
	 * Find the physical address of the NVRAM and assign
	 * its corresponding KSEG address
	 */
	cobra_presto_phystart=(clusterdsc->rpb_pfn * rpb->rpb_pagesize);
	cobra_nvram_start_adr = (caddr_t)PHYS_TO_KSEG(cobra_presto_phystart);
	cobra_nvram_size = clusterdsc->rpb_pfncount * rpb->rpb_pagesize;
	
	PRD(("cobra_nvram_config: cobra_presto_phystart = %l016x\n", cobra_presto_phystart));
	PRD(("cobra_nvram_config: cobra_nvram_start_adr = %l016x\n", cobra_nvram_start_adr));
	PRD(("cobra_nvram_config: cobra_nvram_size = %l016x\n", cobra_nvram_size));
	
	return(~0);
}

/*
 * cobra_nvram_status() - returns status of nvram diagnostics
 * Return Value		NVRAM_RDWR  - nvram clean, read/write diagnostics passed successfully
 *			NVRAM_RDONLY - nvram dirty, checked by read only diagnostics
 *			NVRAM_BAD   - nvram bad, diagnostics failed.
 */
int cobra_nvram_status()
{
	extern int cobra_nvram_status_v;

#ifdef PRESTO_DEBUG
	switch (cobra_nvram_status_v){
	      case NVRAM_RDWR:
		PRD(("cobra_nvram_status: NVRAM_RDWR\n"));
		break;
	      case NVRAM_RDONLY:
		PRD(("cobra_nvram_status: NVRAM_RDONLY\n"));
		break;
	      case NVRAM_BAD:
		PRD(("cobra_nvram_status: NVRAM_BAD\n"));
		break;
	      default:
		PRD(("cobra_nvram_status: bad value, looks like a bug here kate!\n"));
		break;
	}
#endif
	return(cobra_nvram_status_v);
}

/*
 * cobra_nvram_battery_status - called by Prestoserve periodically (once an hour).
 * 	Updates the global battery information structure.  The BATT_ENABLED is different
 *	from the BAT_STATUS_NOBACK.  BATT_ENABLED is not related to software battery
 *	enabling.
 */
int cobra_nvram_battery_status()
{
	int status;

	PRD_BE(("cobra_nvram_battery_status: reading  0x%l016x\n",(cobra_nvram_start_adr + NVRAM_CSR_OFFSET)));
	status = *(cobra_nvram_start_adr + NVRAM_CSR_OFFSET);
	PRD_BE(("cobra_nvram_battery_status: status  0x%1x\n", (status & BATTERY_STATUS_MASK)));
	PRD_BE(("cobra_nvram_battery_status: "));

	/* 
	 * BATT_NONE	0	either no battery or completely bad 
	 * BATT_ENABLED 0x1	battery enabled,
	 *			i.e. will back up NVRAM on power fail
	 * BATT_HIGH	0x2	battery has minimal energy stored,
	 *			i.e. has enough power for Prestoserve use
	 * BATT_OK	0x3	battery is enabled AND has enough power
	 * 			i.e. is usable for Prestoserve
	 * BATT_SELFTEST   0x4  there is a battery, but charge state is unknown
	 * BATT_CHARGING   0x8  battery does not have enough power for Prestoserve use, 
	 *			but is on the way up (as opposed to low, which is on the way
	 *  			down or non-rechargeables)
	 */

	switch(status & BATTERY_STATUS_MASK){

	      case 0x7:		/* BAT_STATUS_OKAY | BAT_STATUS_NO_BACK | BAT_STATUS_CHARGED */
		/* battery ok, battery won't backup, battery charged */
		PRD_BE(("battery ok, battery won't backup, battery charged\n "));
		nvram_batteries0.nv_status[0] = BATT_OK;
		break;

	      case 0x6:		/* BAT_STATUS_OKAY | BAT_STATUS_NO_BACK | ~BAT_STATUS_CHARGED */
		/* battery ok, battery won't backup, battery charging */
		/* don't use it now, but maybe later */
		PRD_BE(("battery ok, battery won't backup, battery charging\n"));
		nvram_batteries0.nv_status[0] = BATT_CHARGING | BATT_ENABLED;
		break;
     
	      case 0x5:		/* BAT_STATUS_OKAY | ~BAT_STATUS_NO_BACK | BAT_STATUS_CHARGED */
		/* battery ok, will backup, battery charged */
		PRD_BE((" battery ok, will backup, battery charged \n"));
		nvram_batteries0.nv_status[0] = BATT_OK;
		break;

	      case 0x4:		/* BAT_STATUS_OKAY | ~BAT_STATUS_NO_BACK | ~BAT_STATUS_CHARGED */
		/* battery ok, will backup, battery charging */
		PRD_BE((" battery ok, will backup, battery charging\n"));
		nvram_batteries0.nv_status[0] = BATT_CHARGING | BATT_ENABLED;
		break;

	      default:
		/* battery failed nothing else matters */
		PRD_BE(("Battery failed.  Must be replaced\n"));
		nvram_batteries0.nv_status[0] = BATT_NONE;
		break;
	}
	return(0);
}

/*
 * cobra_nvram_battery_enable() - arms the battery backup circuit so it
 * will back up the nvram on powerfailure
 */
int 
cobra_nvram_battery_enable()
{
	PRD_BE(("cobra_nvram_battery_enable:\n"));
	if((*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) & BDISC) == 0){
		PRD_BE(("cobra_nvram_battery_enable: already enabled\n"));
		return;
	}

	*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) = 0x2; mb();
	cobra_presto_cflush((cobra_nvram_start_adr + NVRAM_CSR_OFFSET), 1); 

#ifdef PRESTO_DEBUG
	if((*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) & BDISC) == 0){
		PRD_BE(("cobra_nvram_battery_enable: succeeded\n"));
	}
#endif
	return;
}
/*
 * cobra_nvram_battery_enable() - disarms the battery backup circuit so it
 * will not back up the nvram on powerfailure
 */
int
cobra_nvram_battery_disable()
{

	PRD_BE(("cobra_nvram_battery_disable:\n"));
	if((*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) & BDISC) != 0){
		PRD_BE(("cobra_nvram_battery_disable: already disabled\n"));
		return;
	}
	PRD_BE(("cobra_nvram_battery_disable: not disabled\n"));

	*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) = 0x2; mb();
	cobra_presto_cflush((cobra_nvram_start_adr + NVRAM_CSR_OFFSET), 1); 
	*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) = 0x2; mb();
	cobra_presto_cflush((cobra_nvram_start_adr + NVRAM_CSR_OFFSET), 1); 
	*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) = 0x0; mb();
	cobra_presto_cflush((cobra_nvram_start_adr + NVRAM_CSR_OFFSET), 1); 
	*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) = 0x0; mb();
	cobra_presto_cflush((cobra_nvram_start_adr + NVRAM_CSR_OFFSET), 1); 
	*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) = 0x2; mb();
	cobra_presto_cflush((cobra_nvram_start_adr + NVRAM_CSR_OFFSET), 1); 

#ifdef PRESTO_DEBUG
	if((*(cobra_nvram_start_adr + NVRAM_CSR_OFFSET) & BDISC) != 0){
		PRD_BE(("cobra_nvram_battery_disable: succeeded\n"));
		return;
	}
#endif
}

/*
 *	cobra_ssn()
 *
 *	Determine an unsigned 32 bit unique number from the Cobra system
 * 	serial number in the hwrbp.   Convert the serial number from ascii
 * 	to a hex number.  Any letter over 'F' (or f) is converted to 0xf 
 *	modulo the letter. 
 *
 */
cobra_ssn()
{
	extern struct rpb *rpb;
	u_int ssn = 0;
	int i;
	char *cp;

	cp = rpb->rpb_ssn + 9; 

	for (i = 0 ; i < 8 ; i++, cp--){
		if (*cp < '9')
			ssn += (*cp - '0' ) << (i*4);
		else if (*cp < 'G')
			ssn += (*cp - 'A' + 0xa ) << (i*4);
		else if (*cp < 'a')
			ssn += ( *cp % 0xf ) << (i*4);
		else if (*cp < 'g')
			ssn += (*cp - 'a' + 0xa ) << (i*4);
		else 
			ssn += ( *cp % 0xf ) << (i*4);
	}
	PRD(("cobra_ssn: rpb->rpb_ssn = %s , id = %x\n", rpb->rpb_ssn, ssn));
	return(ssn) ;
}
/*
 * End Prestoserve platform specific code
 */

/* Return PALcode version, revision, and type */
/* TODO: Decide whether to use global percpu */
static void kn430_read_palcode_version()
{
	extern struct rpb *rpb;
	struct rpb_percpu *cpu;

	/* get address of the cpu specific data structures for this cpu */
        cpu = (struct rpb_percpu *) ((long)rpb + rpb->rpb_percpu_off);
	kn430_palcode_type     = (cpu->rpb_palrev & 0xff0000L) >> 16;
	kn430_palcode_version  = (cpu->rpb_palrev & 0xff00L) >> 8;
	kn430_palcode_revision = cpu->rpb_palrev & 0xffL;
	kn430_firmware_major = (cpu->rpb_firmrev & 0xff00L) >> 8;
	kn430_firmware_minor = cpu->rpb_firmrev & 0xffL;
}

/*
 * CPU initialization routine.
 *
 * Called through CPU switch from alpha_init through cpu_initialize.  This
 * is called later than the configuration routine.
 */
int cobra_init()
{
 	return(0);
}

/*****************************************************************************
 * DEC 4000 INTERNAL BUS CONFIGURATION					     *
 *****************************************************************************/

/*
 * configure the system
 */
int cobra_config_cbus(struct bus *cbus)
{
	struct bus *lbus, *fbus;
	register mbox_t mbox;

	/* find out what's on the cobra system bus (cbus) */
	cbus_probe(cbus);

	/* configure the local bus with integral i/o */
	if((lbus = get_bus("lbus", 0, cbus->bus_name, cbus->bus_num)) ||
	   (lbus = get_bus("lbus", 0, cbus->bus_name, -1)) ||
	   (lbus = get_bus("lbus", -1, cbus->bus_name, cbus->bus_num)) ||
	   (lbus = get_bus("lbus", -1, cbus->bus_name, -1))) {
		lbus->connect_bus = cbus->bus_name;
		lbus->connect_num = cbus->bus_num;
		/* allocate and set up the lbus mailbox */
		mbox = MBOX_ALLOC();
		lbus->bus_mbox = (u_long *)mbox;
		mbox->bus_timeout = LBUS_TIMEOUT;
		mbox->mbox_cmd = null_mbox_cmd;
		mbox->mbox_reg = (vm_offset_t)&Io_regs->lmbpr;
		if(!(*lbus->confl1)(cbus, lbus))
			panic("cobra_config_bus: lbusconfl1");
		if(!(*lbus->confl2)(cbus, lbus))
			panic("cobra_config_bus: lbusconfl2");
	} else 
		panic("cobra_config_bus: cannot find lbus");

	/* now the future bus */
	/* the fbus is integral to cobra and has its own mailbox.
	 * there are 6 fbus slots available.  
	 * the i/o module is logical slot 0 on the fbus
	 */
	if((fbus = get_bus("fbus", 0, cbus->bus_name, cbus->bus_num)) ||
	   (fbus = get_bus("fbus", 0, cbus->bus_name, -1)) ||
	   (fbus = get_bus("fbus", -1, cbus->bus_name, cbus->bus_num)) ||
	   (fbus = get_bus("fbus", -1, cbus->bus_name, -1))) {
		fbus->bus_num = 0;
		fbus->connect_bus = cbus->bus_name;
		fbus->connect_num = cbus->bus_num;
		/* allocate and set up the fbus mailbox */
		mbox = MBOX_ALLOC();
		fbus->bus_mbox = (u_long *)mbox;
		mbox->bus_timeout = FBUS_TIMEOUT;
	        mbox->mbox_cmd = fbus_mbox_cmd;
		/* hook for err rtn if we need one for fbus */
		mbox->bus_ctlr_ptr = (bus_ctlr_common_t)fbus;
		mbox->mbox_reg = (vm_offset_t)&Io_regs->fmbpr;
		(*fbus->confl1)(cbus, fbus);
		(*fbus->confl2)(cbus, fbus);
	}
	return(1);
}

/*
 * the cbus is the cobra system bus
 * it has 7 slots, with each slot fixed as to the type 
 * of module that can reside in it (i.e. can't put mem in cpu slot)
 *
 * type          phys addr        req/opt
 * i/o module    0x2.1000.0000    required
 * cpu 2         0x2.0800.0000    optional
 * cpu 1         0x2.0000.0000    required
 * mem 1         0x2.4000.0000    optional
 * mem 2         0x2.5000.0000    optional
 * mem 3         0x2.6000.0000    optional
 * mem 4         0x2.7000.0000    optional
 *
 * Notes:
 *       - you must have a cpu in cpu slot 1 (cbus slot 2)
 *       - obviously you must have at least one memory module but 
 *       it can be in any of the 4 memory slots
 *       - the i/o module is required as it carries the console
 */
int cbus_probe(struct bus *cbus)
{
	u_long bcc;
	u_short n_cpu = 1, i;
	u_long mem_config;
	u_long proctype;
	static u_short memsize[] = {16, 64, 0, 0, 32, 128, 0, 0};
	static char *bcachesize[4] = { "0.5", "1", "4", "4" };
	/* processor major/minor numbers, defined in srm */
	static short cpu_major[] = { 0, 3, 4, 5};
	static char *cpu_minor[] = {"p2.1", "p3", "", "s"};
	extern struct cpusw *cpup;	/* pointer to cpusw entry */

	/*
	 * Enable Mailboxes, DMA. (Disable Ethernet and Fbus remote halts,
	 * Fbus Interrupts, and Fbus Power Fail Interrupts).
	 */
	Io_regs->iocsr = IOCSR_ME|IOCSR_FDE|IOCSR_LDE;
	mb();

	/* Got this far -- there must be a cpu and I/O module... */
	Cbus_nodes_alive = CBUS_NODE_CPU(0) | CBUS_NODE_IO;

	/*
	 *	Say what system we are
	 */
	(void)cobra_set_system_string();
	printf("%s\n", cpup->system_string);

	/*
	 * now get cpu rev.
	 *
	 * this is defined in the SRM and found in the hwrpb
	 * ---------------------------------------------------
	 * PROCESSOR TYPE       HWRPB per-cpu SLOT [176]<63:0>
	 * ---------------------------------------------------
	 * <63:32> Major Type
	 *         Value:
	 *               1 = EV-3
	 *               2 = EV-4
	 * <31:0>  Minor Type
	 *         Value:
	 *               0 = Pass 2 or 2.1
	 *               1 = Pass 3
	 *               3 = Simulation
	 */
	proctype = ((struct rpb_percpu *)
		    ((long)rpb + rpb->rpb_percpu_off))->rpb_proctype;
	/* read bcache control information (including size) from cpu module */
	bcc = Cpu_regs[0]->bcc;
	
	printf("cpu 0 EV-%d%s %smb b-cache\n",
	       cpu_major[proctype & 0x00000000ffffffffL],
	       cpu_minor[proctype >> 32],
	       bcachesize[(bcc & BCC_CSZL) >> BCC_CSZL_SHIFT]);

	/* cobra only supports 2 cpu's */
	if(rpb->rpb_numprocs == 2) {
		/* console says #2 is there, verify */
		if((n_cpu += (! BADADDR((long *)Cpu_regs[1], 
					sizeof(long), cbus))) == 2) {
			Cbus_nodes_alive |= CBUS_NODE_CPU(1); 
			proctype = ((struct rpb_percpu *)
				    ((long)rpb +  rpb->rpb_percpu_off +
				     rpb->rpb_slotsize))->rpb_proctype;
			bcc = Cpu_regs[1]->bcc;
			printf("cpu 1 EV-%d %s %smb b-cache\n",
			       cpu_major[proctype & 0x00000000ffffffffL],
			       cpu_minor[proctype >> 32],
			       bcachesize[(bcc & BCC_CSZL) >> BCC_CSZL_SHIFT]);
		}
	}

	/* 
	 * now config memory modules 
	 * there is no device type register, only memory can be here
	 *
	 * probe all 4 memory slots
	 * if a module is there read the module size 
	 * from the config register and print size and slot #
	 *
	 * Reset the CRD filtering, since the console does not do this
	 * accross reboots.
	 */
	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		if(BADADDR((long *)Mem_regs[i], sizeof(long), cbus))
			continue;
		Cbus_nodes_alive |= CBUS_NODE_MEM(i);
		mem_config = Mem_regs[i]->config;
		/*
		 * If the module is not a pass 1 module, make sure that EDC
		 * reporting is not being suppressed through use of the filter
		 * or the CRD Enable bits.
		 */
		if (mem_config & MCONF_REV) {
			Mem_regs[i]->edcctl |= EDCCTL_ENACRD;
			Mem_regs[i]->filter &= ~(EDCF_ENB);
		}
	}
}

/*
 * the lbus is internal to the io module with local i/o.
 * there are 2 variants of the io module, 'DSSI' and 'Fast SCSI'.
 * both variants have 5 scsi controllers and a
 * serial bus (iic) for hw error logging.
 *
 * the 'Fast SCSI' (formerly 'h') io module has 1 tgec
 *     4 of scsi the busses (bus 'a' - 'd') can run fast (up to 10mb/sec)
 *     the fifth scsi bus (bus 'e') can only run up to 4mb/sec
 * the 'DSSI' (formerly 'b') module has 2 tgecs 
 *     all five of the scsi busses can only run about 4mb/sec 
 *     due to dssi circuitry on the module
 *
 * the rpb has a platform specific device data structure that the 
 * console uses to pass information about the device to us for configuration.
 * this data structure is found in kn430.h and MUST 
 * be kept in sync with the console.
 */
int lbusconfl1(struct bus *cbus, struct bus *lbus)
{
	u_long slot;
	long vec, vec_from_allocvec;
	long h_io;
	int i, ntgec, nsiop;
	struct rpb_device *devp;
	struct rpb_config *rpb_config;
	extern struct controller *Iic_ctlr;
	mbox_t l_mb;

	if(cbus == (struct bus *)0)
		panic("lbusconfl1: no ibus");
	lbus->bus_type = BUS_LBUS;
	lbus->alive = ALV_ALIVE;
	conn_bus(cbus, lbus);
	l_mb = (mbox_t)(lbus->bus_mbox);

	nsiop = ntgec = 0;
	/*
	 * this is defined in the SRM and found in the hwrpb
	 * System Variation <15:10> 
	 *                  0 = B I/O Module - kfa40-ba
	 *                          - 2 Ethernet ports
	 *                          - slow (up to 4mb/sec)
	 *                  1 = H I/O Module - kfa40-aa
	 *                          - 1 Ethernet port 
	 *                          - fast (up to 10mb/sec)
	 */
	h_io = (((struct rpb *)hwrpb_addr)->rpb_sysvar >> 10) & 0x3fL;

	/* find the platform dependent table */
	rpb_config = (struct rpb_config *)(hwrpb_addr + rpb->rpb_config_off);
	devp = rpb_config->rpb_device;
	vec_from_allocvec = 0;
	for(i = 0; i < rpb_config->ndevs; devp++, i++) {

		slot = devp->slot;
		vec = (u_int)devp->osf_vector;
		/* Reserve interrupt vectors. 
		 * We must use the vector provided by the console.
		 * We must allocate up to the vector provided by the console.
		 * These are consecutive vectors beginning at 0x800,
		 *   but there may be a series of devices which use the same
		 *   vector.  That is why we cannot simple allocate one vector
		 *   per device entry.
		 */
		while(vec > vec_from_allocvec) {
			if(allocvec(1, &vec_from_allocvec) != KERN_SUCCESS)
				panic("lbusconfl1: can't allocate interrupt vector");
			vec_from_allocvec = vecoffset(vec_from_allocvec);
		}

		switch(devp->dev_type) {

		      case NCR53C710_UNKN:
			/* 
			 * unknown means no disks attached so console
			 * couldn't determine if its dssi or scsi
			 */
		      case NCR53C710_SCSI:
			/* did operator set bus speed via console? */
			if(devp->dev.scsi.speed == 0) {
				/* nope, so set up defaults */
				if(!h_io)     /* b io run's at 3.5mb/sec */
					devp->dev.scsi.speed = 3571;
				/* h module, if bus not fast, run at 5mb/sec */
				else if(! devp->dev.scsi.fast)
					devp->dev.scsi.speed = 5000;
				else    /* crank it up */
					devp->dev.scsi.speed = 10000;
			}

			l_mb->mbox_cmd = siop_mbox_cmd;
			if (ib_config_cont((char *)devp, 0, slot, "siop", lbus,
					  vec)
			    == 0) {
				printf("siop%d at %s%d not configured\n", 
				       slot, lbus->bus_name, lbus->bus_num);
#ifdef notyet
				freevec(1, vec);
#endif /* notyet */
			} else
				nsiop++;
			break;

		      case TGEC:
			l_mb->mbox_cmd = tgec_mbox_cmd;
			if(ib_config_cont((char *)0, (char *)0, slot, 
					  "te", lbus, vec) == 0) {
				printf("te%d at %s%d not configured\n", 
				       slot, lbus->bus_name, lbus->bus_num);
#ifdef notyet
				freevec(1, vec);
#endif /* notyet */
			} else
				ntgec++;
			break;

		      case IIC: 
			/* 
			 * the serial bus is controlled by the iic controller
			 * which is on the lbus and thus accessed by a mailbox.
			 * the mailbox data structure is attached to a
			 * bus or controller data structure, so we allocate
			 * the controller and mailbox data structures here.
			 */
			Iic_ctlr = ((struct controller *)
				    kmem_alloc(kernel_map, 
					       sizeof(struct controller)));
			if(Iic_ctlr == (struct controller *)0)
				panic("lbusconfl1: "
				      "can't allocate i2c controller");
			bzero(Iic_ctlr, sizeof(struct controller));
			MBOX_GET(lbus, Iic_ctlr);
			((mbox_t)Iic_ctlr->ctlr_mbox)->mbox_cmd = srl_mbox_cmd;
			Iic_ctlr->ctlr_name = "iic";
			Iic_ctlr->bus_name = "lbus";
			Iic_ctlr->bus_num = lbus->bus_num;
			Iic_ctlr->slot = slot;
			Iic_ctlr->alive = ALV_PRES; /* for sizer */
			conn_ctlr(lbus, Iic_ctlr);
			break;
		}
	}

	l_mb->mbox_cmd = null_mbox_cmd;
	return(ntgec || nsiop);
}

int lbusconfl2(struct bus *cbus, struct bus *lbus)
{
	return(1);
}

/*****************************************************************************
 * KN430 BAD ADDRESS PROBE ROUTINE					     *
 *****************************************************************************/

/*
 * Counter for communicating BADADDR probe failures.
 */
static long kn430_badaddr;

/*
 * This is the 'bad address' probe routine for Cobra. It is passed an
 * address and len and will return 0 if the address exists and nonzero
 * if the address is nonexistant.
 * 
 * The probe routine sets mcheck_expected before doing the probe.
 *
 * The machine check handler checks mcheck_expected, and sets
 * kn430_badaddr if it is set.
 */
long cobra_badaddr
	(caddr_t addr,
	 int len,
	 struct bus_ctlr_common *ptr) /* bus/ctlr this csr access is for */
{
	unsigned int	int_datum;
	unsigned long	long_datum;
	register int	cpuid;
	register boolean_t stat;
	register long	bum_address;
	register struct mbox *mbp = (mbox_t)0;
	
	if (mcheck_expected) panic("cobra_badaddr: mcheck_expected");

	draina();
	mcheck_expected = TRUE;
	mb();

        kn430_badaddr = FALSE;
	cpuid = kn430_cpuid();
	
	if (mbp = (mbox_t)ptr->mbox) {
		if (stat = mbox_badaddr(addr, len, mbp)) {
			/* 
			 * cobra delivers mchecks through the mailbox
			 * so when we probe another bus (fbus) we have
			 * to cleanup
			 */
			Cpu_regs[cpuid]->sic &= SIC_EIC; /* Clear mcheck if it didn't exist */
			Io_regs->cerr1 &= CERR1_FME;
			Io_regs->iocsr |= IOCSR_FMR;
			mb();
			Io_regs->iocsr &= ~IOCSR_FMR;
			mb();              
			if (Io_regs->cerr1 != 0) panic("cobra_reset_fmbpr");
		}
		bum_address = (stat != 0);
	} else {
		if (len == 8) {
			/* round to quadword boundary */
			addr = (caddr_t)((u_long)addr & ~7UL);
			long_datum = *(long *)addr;
		} else {  /* Treat other lengths as smallest granularity */
			/* round to longword boundary */
			addr = (caddr_t)((u_long)addr & ~3UL);
			int_datum = *(int *)addr;
		}
		mb();
		bum_address = kn430_badaddr;
	}
	mb();
	mb();

	mcheck_expected = FALSE;
	return(bum_address);
}

/*****************************************************************************
 * KN430 PROCESSOR AND DEC 4000 SYSTEM  ERROR HANDLING			     *
 *									     *
 * The error handling routines are called through the cpu switch, from	     *
 * mach_error.								     *
 *									     *
 *****************************************************************************/

/*
 * The severity of a machine check determines what we can do:
 *
 *   System Fatal    - The kernel state is corrupt.  Crash (if you can...).
 *   Processor Fatal - This processor is toast.  For now, crash...
 *   Process Fatal   - A process is corrupt.  Blow it away.  Currently no
 * 		       way to tell the difference between process and system
 *		       fatal -- have to be able to discern user addresses.
 *   Survivable      - Either the error was corrected, or we didn't expect
 *		       to keep running under this error.  Since we're still
 *		       running, it must have been a transient.
 *   Probe	     - The machine check was the result of a bad address probe.
 */
#define MCHECK_SEV_PROBE 0
#define MCHECK_SEV_SURVIVABLE 1
#define MCHECK_SEV_SYSTEM_FATAL 2
#define MCHECK_SEV_PROCESS_FATAL 3
#define MCHECK_SEV_PROCESSOR_FATAL 4

/*
 * Flags to control machine check handler behavior.
 *
 * kn430_suppress_null_intr
 *	If FALSE, null system error interrupts on the primary are sent to the
 *	binary error log.  If TRUE, all null system error interrupts are
 *	suppressed.
 *
 * kn430_syslog_enabled
 *	If TRUE, a summary message is printed with a printf.  The message ends
 *	up in syslog as well as on the console.
 *
 * kn430_csrdump_enabled
 *	If TRUE, CSRs will be dumped if no summary information was printed,
 *	binlog_alloc() couldn't provide a buffer for the CSRs, or the following
 *	control flag is TRUE.
 *
 * kn430_csrdump_always
 *	If TRUE, the CSRs are always dumped to the console using printfs.  If
 *	FALSE, CSR dumps only occur if a summary was not printed.
 */
long kn430_suppress_null_intr = FALSE;	/* Suppress null system err ints */
long kn430_syslog_enabled = TRUE;	/* Send to console/syslog */
long kn430_csrdump_enabled = TRUE;	/* Dump CSRs if needed. */
long kn430_csrdump_always = FALSE;	/* Always print complete CSR dump */


/*
 * Parameters for B-Cache CRD Suppression.
 *
 * If more than one B-Cache CRD is seen over kn430_bcache_suppress_interval,
 * or if more than kn430_bcache_suppress_threshold CRDs are reported, reporting
 * of all B-Cache CRDs will be disabled.
 */
long kn430_bcache_suppress_interval = (5*60); /* 5 minutes, in seconds */
long kn430_bcache_suppress_threshold = 20;
long kn430_bcache_crds = 0;
long kn430_bcache_last_crd = 0;



/*
 * Format of maximum size error log packet.  This structure is just used to
 * accurately document the packet format so that KN430_ELBUF_MAX is
 * maintainable.
 */
struct kn430_errlog_packet_format {
	struct el_cobra_mcheck_event_header cepf_event_hdr;
	union { /* variable portion */
		struct {
			struct el_cobra_frame_proc_bcache_corr cepf_pb;
		} cepf_proccorr;
		struct {
			struct el_cobra_frame_mcheck cepf_mcheck;
			struct el_cobra_frame_other_cpu cepf_ocpu;
			struct el_cobra_frame_memory cepf_mem[CBUS_MEM_SLOTS];
			struct el_cobra_frame_io cepf_io;
		} cepf_sysmcheck;
		struct {
			struct el_cobra_frame_mcheck cepf_mcheck;
			struct el_cobra_frame_other_cpu cepf_ocpu;
			struct el_cobra_frame_memory cepf_mem[CBUS_MEM_SLOTS];
			struct el_cobra_frame_io cepf_io;
		} cepf_procmcheck;
	} cepf_vp;
	char cepf_pad[64];  /* Padding for small overruns */
	struct el_cobra_frame_header cepf_terminator;
};

#define KN430_ELBUF_MAX sizeof(struct kn430_errlog_packet_format)

/*
 * Machine check handler initialization.  
 * Set up scratch buffers, print debug information.
 */
static void kn430_mcheck_init(register int cpuid)
{
	register vm_offset_t buf, rmng;
	register struct kn430_scratch_buffer *sb;
	register long i;
	static int vector[] = { 0, KN430_VECTOR_PROC_FATAL };

	/*
	 * Allocate error log scratch buffers for this CPU.  These are used
	 * to grab snapshots of system CSRs when a processor or system error
	 * interrupt is delivered.
	 *
	 * Write something to the scratch area right away.  This pulls in the
	 * page, preventing a double machine check error which occurs if an
	 * attempt is made to fault in the page at splextreme().
	 * The proper way to handle this in the long run is to wire the
	 * page after kmem_alloc.
	 */
	rmng = NBPG;
	buf = kmem_alloc(kernel_map, rmng);

	for (i = 0; i < sizeof(vector) / sizeof(vector[0]); i++) {
		sb = KN430_SCRATCH_BUFFER(cpuid, vector[i]);
		sb->sb_start = buf;
		*((long *)buf) = 0;	/* Pull in the first page... */
		sb->sb_len = KN430_ELBUF_MAX;
		sb->sb_ptr = (vm_offset_t)0;
		sb->sb_rmng = 0;
		buf += KN430_ELBUF_MAX;
		*((long *)buf) = 0;	/* Pull in the last page... */
		rmng -= KN430_ELBUF_MAX;
	}

	/*
	 * The rest of this is done only once, on the primary.
	 */
	if (cpuid != master_cpu) return;

	kn430_edc_init();
}

/*
 * KN430 Processor Correctible Error Handling.
 *
 * This entry is called through the CPU switch for processor correctible
 * error interrupts (0x630).
 */
long kn430_proccorr
	(register long vector,
	 register struct el_cobra_data_proc_bcache_corr *lgt)
{
	register struct kn430_mcheck_control *mctl;
	register struct kn430_scratch_buffer *sb;
	register int cpuid;
	register long timestamp;

	/*
	 * Capture the error and parse it.
	 */
	cpuid = kn430_cpuid();
	mctl = KN430_MCHECK_CONTROL(cpuid, vector);
	sb = kn430_errlog_start(mctl, cpuid, vector);
	kn430_parse_proccorr(mctl, lgt, sb);
	kn430_mcheck_dispatch(mctl, sb, MCHECK_SEV_SURVIVABLE, FALSE);

	return 0;
}

/*
 * kn430_parse_proccorr (Processor Correctable Error Interrupt flow) -- 0x630.
 *
 */
static void kn430_parse_proccorr
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_data_proc_bcache_corr *lgt,
	 register struct kn430_scratch_buffer *sb)
{
	register struct el_cobra_frame_proc_bcache_corr *elcpb;
	register struct el_cobra_mcheck_event_header *elch;
	register int cpuid;
	register long timestamp;

	/*
	 * Get the Processor Corrected Error Frame.
	 */
	cpuid = kn430_cpuid();
	elcpb = kn430_get_frame_pbccorr(mctl, lgt, sb);
	kn430_free_logout(MCES_MCK|MCES_PCE);

	/*
	 * Decide whether to suppress further CRD reporting.  Indicate that
	 * CRD reporting was suppressed by jamming a value into the PAL error
	 * code field of the header.  Once CRD reporting has been disabled,
	 * summarize the circumstances to the console.
	 */
	kn430_bcache_crds++;
	timestamp = kn430_edc_timestamp();
	if ((kn430_bcache_crds > kn430_bcache_suppress_threshold)
	    || ((timestamp - kn430_bcache_last_crd) 
		< kn430_bcache_suppress_interval)) {
		Cpu_regs[cpuid]->bcc &= ~BCC_ECEI;
		elcpb->elcpb_data.elcpb_lhdr.elcl_error_type = 
			COBRA_PAL_ECC_C_LAST;
		printf("Excessive B-Cache CRD interrupts on CPU %d.\n"
		       "  Total of %d CRD interrupts received.\n"
		       "  B-Cache CRD reporting has been disabled.\n",
		       cpuid, kn430_bcache_crds);
	}

	/*
	 * Set up the header.
	 */
	elch = mctl->mctl_frame.mcfr_hdr;
	elch->elch_fru1 = ((cpuid == 0) ? COBRA_FRU_CPU1 : COBRA_FRU_CPU2);
	elch->elch_sev = COBRA_SEV_ALARM;
	elch->elch_fcode = COBRA_FCODE_EV_0_BC_CORR + cpuid;
	COBRA_ERROR_FLAG_SET(elch->elch_flags,
			     (COBRA_CPU0I_EV_C_CORR + 
			      (cpuid * COBRA_CPU_SHIFT)));
}

/*
 * KN430 Machine Check Handler.
 *
 * This entry is called through the CPU switch for processor and system
 * uncorrectible error interrupts (0x660 and 0x670).  The PFMS specifies
 * different parses for the two flavors of interrupts.  This code can be
 * merged if/when the PFMS parse trees are merged.
 */
long kn430_machcheck
	(register long vector,
	 register struct el_cobra_data_mcheck *lgt)
{
	register struct kn430_mcheck_control *mctl;
	register struct kn430_scratch_buffer *sb;
	register int severity = MCHECK_SEV_SYSTEM_FATAL;
	register int cpuid;
	register long suppress = FALSE; /* Suppress logging */
	register struct el_cobra_mcheck_event_header *elch;

	cpuid = kn430_cpuid();
	mctl = KN430_MCHECK_CONTROL(cpuid, vector);
	sb = kn430_errlog_start(mctl, cpuid, vector);
	elch = mctl->mctl_frame.mcfr_hdr;

	switch (vector) {
	case KN430_VECTOR_SYS_FATAL: 

		/*
		 * System Error Interrupt.  May be survivable, depending on
		 * which error flags were set.
		 */
		kn430_parse_sysmcheck(mctl, lgt, sb);

		/*
		 * Suppress null error interrupts on the secondary, and also
		 * suppress them on the primary if requested.
		 */
		if (COBRA_ERROR_FLAGS_EMPTY(elch->elch_flags)
		    && ((cpuid != master_cpu) || kn430_suppress_null_intr)) {
			suppress = TRUE;
			severity = MCHECK_SEV_SURVIVABLE;
		}
		else {
			/*
			 * Copy the error flags and clear out flags for errors
			 * which are survivable.  The machine check parse took
			 * care of any required cleanup.  After clearing out
			 * survivable errors, see if all error flags are clear,
			 * and downgrade severity to survivable.
			 */
			register u_long tempflags[2];
			register long i;

			bcopy(elch->elch_flags, tempflags, sizeof(tempflags));

			/*
			 * Memory correctable errors are survivable if
			 * correction is not disabled for the board which
			 * reported the error.
			 */
			for (i = 0; i < CBUS_MEM_SLOTS; i++) {
				if (COBRA_ERROR_FLAG_VAL(tempflags,
							 COBRA_MEM0_CORR + i)
				    && !COBRA_ERROR_FLAG_VAL(tempflags,
							     COBRA_MEM0_CORRDIS
							     + i)) {
					COBRA_ERROR_FLAG_CLR(tempflags,
							     COBRA_MEM0_CORR
							     + i);
				}
			}

			/* Duplicate Tag Parity Errors are survivable */
			COBRA_ERROR_FLAG_CLR(tempflags, COBRA_CPU_DT_PAR);

			if (COBRA_ERROR_FLAGS_EMPTY(tempflags)) {
				suppress = TRUE;
				severity = MCHECK_SEV_SURVIVABLE;
			}
		}
		break;

	case KN430_VECTOR_PROC_FATAL: 
		/* 
		 * Processor Machine-Check Abort.  We're going down...
		 */
		kn430_parse_procmcheck(mctl, lgt, sb);
		break;
	default:
		panic("kn430_machcheck");
		break;
	}

	/*
	 * If a probe is in progress, don't log the event.  Parsing the event
	 * cleared the CSRs.  It is, however, necessary to reenable allocation,
	 * since the machine check parse treats the NXM as fatal and doesn't
	 * reenable probing.  Also clear the SIC here, to prevent a null
	 * error interrupt.
	 *
	 * TODO: Should be more paranoid about this, and only do it
	 *	 if exactly the right error flags are set during the
	 *	 parse (to avoid dismissing real errors which were
	 *	 delivered at precisely the same time).
	 */
	if (mcheck_expected) {
		kn430_badaddr = TRUE;

		/*
		 * Clear the pending system error interrupt.
		 */
		Cpu_regs[cpuid]->sic &= SIC_SEIC;
		Cpu_regs[cpuid]->sic &= SIC_EIC;

		/*
		 * Reenable allocation, if it was enabled before the machine
		 * check.
		 */
		Cpu_regs[cpuid]->bcc |= 
			(BCC_EA & mctl->mctl_frame.mcfr_mcheck->
			 elcmc_data.elcmc_bcc);
		return 0;
	}

	kn430_mcheck_dispatch(mctl, sb, severity, suppress);
	return 0;
}

/*
 * kn430_parse_sysmcheck (System Machine-Check Abort Error Flow 0x660)
 *
 */
static void kn430_parse_sysmcheck
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_data_mcheck *lgt,
	 register struct kn430_scratch_buffer *sb)
{
	register struct el_cobra_mcheck_event_header *elch;
	register struct el_cobra_frame_mcheck *elcmc;
	register struct el_cobra_frame_other_cpu *elco;
	register struct el_cobra_frame_io *elci;
	register int cpuid, ocpuid;
	register long i;
	register long reenable_allocation;
	register long scrubbed;

	elch = mctl->mctl_frame.mcfr_hdr;


	/*********************************************************************
	 * Snapshot entire system into error log scratch buffer.	     *
	 *********************************************************************
	 * The current PFMS calls for all frames to be snapshotted.  Could get
	 * smarter about this once we're confident that errors are being
	 * recognized correctly.
	 */

	cpuid = kn430_cpuid();
	elcmc = kn430_get_frame_mcheck(mctl, lgt, sb);
	kn430_free_logout(MCES_MCK);
	ocpuid = KN430_OTHER_CPUID(cpuid);
	if (CBUS_CPU_ALIVE(ocpuid)) {
		elco = kn430_get_frame_ocpu(mctl, ocpuid, sb);
	}
	else elco = (struct el_cobra_frame_other_cpu *)0;
	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		if (CBUS_MEM_ALIVE(i)) kn430_get_frame_mem(mctl, i, sb);
	}
	elci = kn430_get_frame_io(mctl, sb);

	/*********************************************************************
	 * Check for PAL detected errors.  Later PALs will always disable    *
	 * bcache allocation before interrupting.  Re-enable allocation if   *
	 * there is nothing wrong with the bcache.			     *
	 *********************************************************************/
	
	reenable_allocation = TRUE;

	/*
	 * Tag Parity Error. ($ND:3)
	 */
	if ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_C3_TAG_PAR)
	    || ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_UNIMP)
		&& (elcmc->elcmc_data.elcmc_bcue & (BCUE_PE|BCUE_MPE)))) {
		kn430_parse_c3_tag_par(elcmc->elcmc_data.elcmc_bcue, 
				       elcmc->elcmc_data.elcmc_bcuea, 
				       elch, cpuid);
		reenable_allocation = FALSE;
	}

	/*
	 * Duplicate Tag Store parity error.  ($ND:14)
	 *
	 * Test Mask:		C3-DTER<33|1>
	 * Missed Error Mask:	C3-DTER<32|0>
	 * Clear And Mask:	C3-DTER<33,1>
	 * Clear Or Mask:	none
	 *
	 * Note: We only test for the first error, because missed error bits
	 *       can be left set to disable interrupts from this source.
	 * Note: If this is the second error, we should not clear the error,
	 * 	 thus allowing the second error bit to set, disabling the
	 *	 interrrupt.
	 */
	if (elcmc->elcmc_data.elcmc_dter & DTER_PE) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_CPU_DT_PAR);
		/*
		 * Currently test for first error only.  Need to clear the
		 * error if want to get interrupted for subsequent errors.
		 *    Cpu_regs[cpuid]->dter |= (DTER_PE|DTER_MPE);
		 * Don't disable allocation for this error.
		 */
	}

	/*
	 * This CPU detected B-Cache Uncorrectable error.  ($ND:2)
	 */
	if (kn430_parse_bcache_uncorr(elcmc->elcmc_data.elcmc_bcue, 
				      elch, cpuid)) {
		reenable_allocation = FALSE;
	}

	if (kn430_parse_c3_syndromes(elcmc->elcmc_data.elcmc_bcue, 
				     elch, cpuid)) {
		reenable_allocation = FALSE;
	}

	/*
	 * Reenable allocation if no B-cache errors were found.
	 */
	if (reenable_allocation) {
		Cpu_regs[cpuid]->bcc |= BCC_EA;
	}

	/*********************************************************************
	 * Examine this CPU for errors.					     *
	 *********************************************************************/

	kn430_parse_c3_wd_par(elcmc->elcmc_data.elcmc_cbe, elch, cpuid);
	kn430_parse_c3_ca_par(elcmc->elcmc_data.elcmc_cbe, elch, cpuid);

	/*
	 * C3 detected B-Cache correctable error on this CPU.  ($ND:26B)
	 *
	 * Test Mask:		C3-CSR1<(49&35)|(17&3)>
	 * Missed Error Mask:	C3-CSR1<34|2>
	 * Clear And Mask:	C3-CSR1<49,35,34,17,3,2>
	 * Clear Or Mask:	none
	 *
	 * The failing data should be scrubbed from the B-Cache by PALcode.
	 * The simplest way to do this is to push it back to memory by
	 * victimizing the cache block, ideally reading an address offset by
	 * 4MB to insure that it works with CPUs with 1 and 4MB B-Caches.
	 *
	 * When PALcode scrubs the location, the missed_error bits may be set
	 * as a side errect.  PAL should not clear the error bits in C3-CSR1.
	 * PAL should set error_type = MCHK$C_C3_CORR.
	 *
	 * Only clear this error when they are in your own cpu's C3.
	 * This error is non-fatal; log and return.
	 *
	 * TODO: Test to be sure this is non-fatal.
	 */
	if ((elcmc->elcmc_data.elcmc_bcce & (BCCE_ISBCL|BCCE_CEL))
	    || (elcmc->elcmc_data.elcmc_bcce & (BCCE_ISBCH|BCCE_CEH))
	    || (elcmc->elcmc_data.elcmc_bcce & BCCE_MCE)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags, 
				     (COBRA_CPU0I_C3_C_CORR + 
				      cpuid * COBRA_CPU_SHIFT));
	}

	/*********************************************************************
	 * If other CPU is handling "SYSTEM" error, stop parsing.	     *
	 *********************************************************************/

	if (cpuid != master_cpu) return;

	/*********************************************************************
	 * Examine other CPU (if present) for errors.			     *
	 *********************************************************************/

	if (elco) {
		kn430_parse_c3_wd_par(elco->elco_data.elco_cbe, elch, ocpuid);
		kn430_parse_c3_ca_par(elco->elco_data.elco_cbe, elch, ocpuid);
		kn430_parse_bcache_uncorr(elco->elco_data.elco_bcue, 
					  elch, ocpuid);
		if (elco->elco_data.elco_bcue & (BCUE_PE|BCUE_MPE)) {
			kn430_parse_c3_tag_par(elco->elco_data.elco_bcue, 
					       elco->elco_data.elco_bcuea, 
					       elch, ocpuid);
		}
	}

	/*********************************************************************
	 * Examine I/O Module for errors.				     *
	 *********************************************************************/

	kn430_parse_io_gate_sync(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_bus_sync(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_wd_noack(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_wd_par(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_rd_uncorr(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_rd_par(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_ca_noack(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_ca_par(elci->elci_data.elci_cerr1, elch);

	kn430_parse_fbus_dma(elci->elci_data.elci_ferr1, elch);
	kn430_parse_fbus_mbx(elci->elci_data.elci_cerr1, elch);

	kn430_parse_lbus_dma(elci->elci_data.elci_lerr1, elch);

	/*********************************************************************
	 * Examine memories for errors.					     *
	 *********************************************************************/

	kn430_parse_mem_sync(mctl, elch);
	kn430_parse_mem_ca_par(mctl, elch);
	kn430_parse_mem_wd_par(mctl, elch);
	if (kn430_parse_mem_uncorr(mctl, elch)) {
		kn430_parse_io_mem_uncorr(mctl, elch);
	}
	if (kn430_parse_mem_edc(mctl, elch)) {
		scrubbed = kn430_handle_edcs(mctl, elch);
	}

	return;
}

/*
 * kn430_parse_procmcheck (System Machine-Check Abort Error Flow) (0x670)
 */
static void kn430_parse_procmcheck
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_data_mcheck *lgt,
	 register struct kn430_scratch_buffer *sb)
{
	register struct el_cobra_mcheck_event_header *elch;
	register struct el_cobra_frame_mcheck *elcmc;
	register struct el_cobra_frame_other_cpu *elco;
	register struct el_cobra_frame_io *elci;
	register int cpuid, ocpuid;
	register long i;

	elch = mctl->mctl_frame.mcfr_hdr;
	
	/*********************************************************************
	 * Snapshot entire system into error log scratch buffer.	     *
	 *********************************************************************
	 * The current PFMS calls for all frames to be snapshotted.  Could get
	 * smarter about this once we're confident that errors are being
	 * recognized correctly.
	 */

	cpuid = kn430_cpuid();
	elcmc = kn430_get_frame_mcheck(mctl, lgt, sb);
	kn430_free_logout(MCES_MCK);
	ocpuid = 1 - cpuid;
	if (CBUS_CPU_ALIVE(ocpuid)) {
		elco = kn430_get_frame_ocpu(mctl, ocpuid, sb);
	}
	else elco = (struct el_cobra_frame_other_cpu *)0;
	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		if (CBUS_MEM_ALIVE(i)) {
			kn430_get_frame_mem(mctl, i, sb);
		}
	}
	elci = kn430_get_frame_io(mctl, sb);

	/*********************************************************************
	 * Check for errors handled by PALCODE.				     *
	 *********************************************************************/

	/*
	 * PAL Detected Cache Fill EDC error.
	 * (PAL detects by checking BIU_STAT<8>.)
	 * Current PAL does not set error_code, so check for the bit here.
	 */
	if ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_EV_C_UNCORR)
	    || ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_UNIMP)
		&& (elcmc->elcmc_data.elcmc_biu_stat & BIU_FILL_ECC))) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0_EV_C_UNCORR +
				      cpuid * COBRA_CPU_SHIFT));
	}

	/*
	 * PAL Detected B-Cache Tag Parity Error.
	 * (PAL detects by checking BIU_STAT<2>).
	 */
	if ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_EV_T_PAR)
	    || ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_UNIMP)
		&& (lgt->elcmc_biu_stat & BIU_TPERR))) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0_EV_T_PAR +
				      cpuid * COBRA_CPU_SHIFT));
	}

	/*
	 * PAL Detected B-Cache Tag Control Parity Error.
	 * (PAL detects by checking BIU_STAT<3>).
	 */
	if ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_EV_TC_PAR)
	    || ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_UNIMP)
		&& (lgt->elcmc_biu_stat & BIU_TCPERR))) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0_EV_TC_PAR +
				      cpuid * COBRA_CPU_SHIFT));
	}

	/*
	 * Cobra Bus C_ERR.
	 */
	if ((elcmc->elcmc_data.elcmc_biu_stat & BIU_HERR)
	    || (KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_HERR)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     COBRA_CPU_EV_HARD_ERROR);
	}

	/*
	 * PAL handled C3 detected Tag Parity Error.
	 */
	if ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_C3_TAG_PAR)
	    || ((KN430_MCHECK_ERROR_TYPE(elcmc) == COBRA_PAL_UNIMP)
		&& (lgt->elcmc_bcue & (BCUE_PE|BCUE_MPE)))) {
		kn430_parse_c3_tag_par(lgt->elcmc_bcue, lgt->elcmc_bcuea, 
				       elch, cpuid);
	}

	/*********************************************************************
	 * Check for errors not handled by PALCODE.  May need to change "==" *
	 * to "&" in the tests below, if PAL treats elch_flags as a mask and *
	 * delivers multiple errors.                                         *
	 *********************************************************************/

	/*
	 * EV Captured Syndrome of 0x1F. ($ND:27)
	 * 
	 * To test for EV captured 0x1f syndrome, examine the 7 bit syndrome
	 * fields FILL_SYNDROME<13:07> and FILL_SYNDROME<6:0>.
	 *
	 * It's alright to test the syndromes even if they're not latched,
	 * since they only equal 0x1F if latched on an error.
	 */
	if (((elcmc->elcmc_data.elcmc_fill_syndrome & FILL_L) 
	     == FILL_SYNDROME_POISONED)
	    || (((elcmc->elcmc_data.elcmc_fill_syndrome & FILL_H) 
		 >> FILL_H_SHIFT) 
		== FILL_SYNDROME_POISONED)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     COBRA_CPU_EV_SYN_1F);
		/*
		 * No need to clear this out -- PAL cleared it when it read
		 * FILL_ADDR.
		 */
	}


	kn430_parse_c3_syndromes(elcmc->elcmc_data.elcmc_bcue, elch, cpuid);
	kn430_parse_bcache_uncorr(elcmc->elcmc_data.elcmc_bcue, elch, cpuid);

	/*
	 * C3 Detected EV Data Bus Hard Error (EV -> C3).
	 *
	 * Test Mask:		C3-BCUE<(35&!49)|(3&!17)>
	 * Missed Error Mask:	C3-BCUE<34|2>
	 * Clear And Mask:	C3-BCUE<34,2>
	 * Clear Or Mask:	none
	 */
	if (((elcmc->elcmc_data.elcmc_bcue & (BCUE_UEL|BCUE_ISBCL)) 
	     == BCUE_UEL)
	    || ((elcmc->elcmc_data.elcmc_bcue & (BCUE_UEH|BCUE_ISBCH)) 
		== BCUE_UEH)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0_C3_EV + 
				      cpuid * COBRA_CPU_SHIFT));
		Cpu_regs[cpuid]->bcue &= (BCUE_UE|BCUE_MUE);
	}


	/*
	 * C/A Not Acked.
	 *
	 * Test Mask:		C3-CBE<14>
	 * Missed Error Mask:	none
	 * Clear And Mask:	C3-CBE<14>
	 * Clear Or Mask:	none
	 *
	 * Should this error actually turn out to be a C/A parity error,
	 * C3-CBEAL and CBEAH registers could be compared with the memory
	 * and I/O command trap registers to see if there is a correlation
	 * and possibly, which bit has failed.  (This can be done from the
	 * logged data).
	 */
	if (elcmc->elcmc_data.elcmc_cbe & CBE_CANA) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0_C3_CA_NOACK +
				      (cpuid ? COBRA_CPU_SHIFT : 0)));
		Cpu_regs[cpuid]->cbe &= CBE_CANA;
	}

	kn430_parse_c3_ca_par(elcmc->elcmc_data.elcmc_cbe, elch, cpuid);
	kn430_parse_c3_rd_par(elcmc->elcmc_data.elcmc_cbe, elch, cpuid);

	/*
	 * Cobra Bus Write Data No-Ack.
	 *
	 * Test Mask:		C3-CBE<15>
	 * Missed Error Mask:	none
	 * Clear And Mask:	C3-CBE<15>
	 * Clear Or Mask:	none
	 *
	 * This could occur due to data corruption on the CBus, or if we got a
	 * double bit error in the B-Cache tag during an exchange placing the
	 * target address outside the currently configured address space.
	 */
	if (elcmc->elcmc_data.elcmc_cbe & CBE_WDNA) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags, 
				     (COBRA_CPU0_C3_WD_NOACK +
				      cpuid * COBRA_CPU_SHIFT));
		Cpu_regs[cpuid]->cbe &= CBE_WDNA;
	}

	kn430_parse_c3_wd_par(elcmc->elcmc_data.elcmc_cbe, elch, cpuid);


	/*********************************************************************
	 * Memory Errors.						     *
	 *********************************************************************/

	if (kn430_parse_mem_uncorr(mctl, elch)) {
		kn430_parse_cpu_mem_uncorr(mctl, elch, 
					   elcmc->elcmc_data.elcmc_fill_addr);
	}
	kn430_parse_mem_wd_par(mctl, elch);
	kn430_parse_mem_sync(mctl, elch);
	kn430_parse_mem_ca_par(mctl, elch);


	/*********************************************************************
	 * Examine other CPU (if present) for errors.			     *
	 *********************************************************************/

	if (elco) {
		if (kn430_parse_bcache_uncorr(elco->elco_data.elco_bcue, elch,
					     cpuid)) {
			kn430_parse_ocpu_bcache_uncorr(cpuid, elcmc,
						       ocpuid, elco,
						       elch);
		}

		if (elco->elco_data.elco_bcue & (BCUE_PE|BCUE_MPE)) {
			kn430_parse_c3_tag_par(elco->elco_data.elco_bcue, 
					       elco->elco_data.elco_bcuea, 
					       elch, ocpuid);
		}

		kn430_parse_c3_ca_par(elco->elco_data.elco_cbe, elch, ocpuid);
		kn430_parse_c3_wd_par(elco->elco_data.elco_cbe, elch, ocpuid);
	}


	/*********************************************************************
	 * Examine I/O Module for errors.				     *
	 *********************************************************************/

	kn430_parse_io_ca_par(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_wd_par(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_gate_sync(elci->elci_data.elci_cerr1, elch);
	kn430_parse_io_bus_sync(elci->elci_data.elci_cerr1, elch);

	if (!kn430_dont_enable_probing(elch, cpuid, ocpuid)) {
		Cpu_regs[cpuid]->bcc |= BCC_EA;
	}
}

static long kn430_dont_enable_probing
	(register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid,
	 register int ocpuid)
{
	return TRUE;
/*
 * Don't ever re-enable probing, since currently all processor errors are
 * fatal.  Enabling the B-Cache introduces another component which can cause
 * the coredump to fail.
 *
 * If the set of recoverable processor errors becomes non-null, start with the
 * expression below to decide when to re-enable probing.
 */
#ifdef notdef
	return (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
				     COBRA_CPU0I_C3_T_PAR +
				     cpuid * COBRA_CPU_SHIFT)
		|| COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					COBRA_CPU0I_C3_TC_PAR +
					cpuid * COBRA_CPU_SHIFT)
		|| COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					COBRA_CPU0_EV_T_PAR +
					cpuid * COBRA_CPU_SHIFT)
		|| COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					COBRA_CPU0_EV_TC_PAR +
					cpuid * COBRA_CPU_SHIFT)
		|| ((COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					  COBRA_CPU0_EV_C_UNCORR +
					  cpuid * COBRA_CPU_SHIFT)
		     || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					     COBRA_CPU0I_C3_C_UNCORR +
					     cpuid * COBRA_CPU_SHIFT))
		    && !(COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					      COBRA_CPU0I_C3_TC_PAR +
					      ocpuid * COBRA_CPU_SHIFT)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_CPU0I_C3_T_PAR +
						 ocpuid * COBRA_CPU_SHIFT)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_CPU0I_C3_C_UNCORR +
						 ocpuid * COBRA_CPU_SHIFT)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_CPU0_C3_RD_PAR +
						 ocpuid * COBRA_CPU_SHIFT)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM0_UNCORR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM1_UNCORR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM2_UNCORR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM3_UNCORR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM0_SYNC_ERROR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM1_SYNC_ERROR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM2_SYNC_ERROR)
			 || COBRA_ERROR_FLAG_VAL(elch->elch_flags,
						 COBRA_MEM3_SYNC_ERROR))));
#endif /* notdef */
}

/*
 * Check for Tag or Tag Control Parity Error.  ($ND:3)
 *
 * There is no visibility to the tag control field.  If the tag parity is OK,
 * assume that the error was a Tag Control Parity Error.
 *
 * Test Mask:		C3-BCUE<33|1>
 * Missed Error Mask:	C3-BCUE<32|0>
 * Clear And Mask:	C3-BCUE<33,32,1,0>
 * Clear Or Mask:	none (V1.23 spec is garbled)
 *
 * NOTE: V1.23 of the quick release document indicates that the OS should NOT
 * 	 clear the BCUE bits; PAL should already have done so:  "This list is
 *	 not complete.  The C3 will in fact set numerous other error bits.
 *	 Chasing a list from Niten."
 */
static long kn430_parse_c3_tag_par
	(register u_long bcue,
	 register u_long bcuea,
	 register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid)
{
	static long found;

	found = FALSE;
	/*
	 * No need to check MPE here, because we can't do anything with the
	 * information anyway.
	 */
	if (bcue & BCUE_PE) {
		found = TRUE;
		if (bcue & BCUE_PEL) {
			if (even_parity(bcuea & (BCUEA_TVL|BCUEA_TPL))) {
				/* Tag Parity Error. */
				COBRA_ERROR_FLAG_SET
					(elch->elch_flags,
					 (COBRA_CPU0I_C3_T_PAR +
					  (cpuid ? COBRA_CPU_SHIFT : 0)));
			}
			else {
				/* Tag Control Parity Error. */
				COBRA_ERROR_FLAG_SET
					(elch->elch_flags,
					 (COBRA_CPU0I_C3_TC_PAR +
					  (cpuid ? COBRA_CPU_SHIFT : 0)));
			}
		}
		if (bcue & BCUE_PEH) {
			found = TRUE;
			if (even_parity(bcuea & (BCUEA_TVH|BCUEA_TPH))) {
				/* Tag Parity Error. */
				COBRA_ERROR_FLAG_SET
					(elch->elch_flags,
					 (COBRA_CPU0I_C3_T_PAR +
					  (cpuid ? COBRA_CPU_SHIFT : 0)));
			}
			else {
				/* Tag Control Parity Error. */
				COBRA_ERROR_FLAG_SET
					(elch->elch_flags,
					 (COBRA_CPU0I_C3_TC_PAR +
					  (cpuid ? COBRA_CPU_SHIFT : 0)));
			}
		}
	}
	return found;
}

/*
 * Return the even parity bit for a longword.  If the bits in the longword
 * include the generated parity bit, then this routine returns 1 in the case
 * of bad even parity.
 */
static u_long even_parity(register u_long bits)
{
	register u_long parity;
	register u_long *mask;
	register long i;
#define PARITY_TABLE_ENTRIES 5
	static u_long parity_mask[PARITY_TABLE_ENTRIES] =
		{
			0x5555555555555555L, 0x1111111111111111L, 
			0x0101010101010101L, 0x0001000100010001L,
			0x0000000100000001L
			};

	parity = bits;
	mask = &parity_mask[0];
	for(i = 0; i < PARITY_TABLE_ENTRIES; i++, mask++) {
		parity = ((parity & *mask) ^ ((parity >> (1L << i)) & *mask));
	}
	return ((parity >> (1L << PARITY_TABLE_ENTRIES)) ^ (parity & 0x1L));
}

/*
 * Check for C3-detected syndrome of 0x1f.  ($ND:28)
 *
 * Test Mask:		
 * Missed Error Mask:	
 * Clear And Mask:	
 * Clear Or Mask:	
 *
 * Note: To test for C3 capture 0x1f syndrome examine the 7 bit
 *	 syndrome fields C3-BCUE<24:18>,C3-BCUE<56:50>,C3-BCUE<31:25>,
 *	 C3-BCUE<63:57>
 * 
 * It's alright to test the syndromes even if they're not latched,
 * since they only equal 0x1F if latched on an error.
 */
static long kn430_parse_c3_syndromes
	(register u_long bcue,
	 register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid)
{
	static long found;

	if ((((bcue & BCUE_EDCSYN0) >> BCUE_EDCSYN0_SHIFT)
	     == FILL_SYNDROME_POISONED)
	    || (((bcue & BCUE_EDCSYN1) >> BCUE_EDCSYN1_SHIFT)
		== FILL_SYNDROME_POISONED)
	    || (((bcue & BCUE_EDCSYN2) >> BCUE_EDCSYN2_SHIFT)
		== FILL_SYNDROME_POISONED)
	    ||(((bcue & BCUE_EDCSYN3) >> BCUE_EDCSYN3_SHIFT)
	       == FILL_SYNDROME_POISONED)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     COBRA_CPU_C3_SYN_1F);
		Cpu_regs[cpuid]->bcue &= (BCUE_PE|BCUE_MPE|BCUE_UE|BCUE_MUE);
		found = TRUE;
	}
	else {
		found = FALSE;
	}
	return found;
}

/*
 * B-Cache Uncorrectable error detected by B-Cache controller.  ($ND:2)
 *
 * Test Mask:		C3-BCUE<49&35)|(17&3)> BC Source and Uncorr Error
 * Missed Error Mask:	C3-BCUE<34|2>
 * Clear And Mask:	C3-BCUE<35,34,3,2>
 * Clear Or Mask:	none
 *
 * This error can occur when the C3 gets an uncorrectable error reading dirty
 * data from the B-Cache.
 *
 * $ND:2A: In the processor m-check flow, should an error be found in
 * 	   the other CPU's B-Cache, we want to check if it is related to the
 *	   cycle which failed for us.
 */
static long kn430_parse_bcache_uncorr
	(register u_long bcue,
	 register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid)
{
	register long detected = FALSE;

	if ((bcue & BCUE_MUE)
	    || ((bcue & (BCUE_UEL|BCUE_ISBCL)) == (BCUE_UEL|BCUE_ISBCL))
	    || ((bcue & (BCUE_UEH|BCUE_ISBCH)) == (BCUE_UEH|BCUE_ISBCH))) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0I_C3_C_UNCORR +
				      cpuid * COBRA_CPU_SHIFT));
		Cpu_regs[cpuid]->bcue &= (BCUE_UE|BCUE_MUE);
	}
	return detected;
}

/*
 * Correlate other CPU's B-Cache Uncorrectable error to this CPU's
 * B-Cache Uncorrectable Error.					      ($ND:2A)
 */
static long kn430_parse_ocpu_bcache_uncorr
	(register int cpuid,
	 register struct el_cobra_frame_mcheck *elcmc,
	 register int ocpuid,
	 register struct el_cobra_frame_other_cpu *elco,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;
	register u_long cad;
	register vm_offset_t pa;

	/*
	 * The idea behind the PFMS's parse subtree for this section is to 
	 * set MIXED_ERRORS if some other error kept part of the information
	 * from being latched.  Otherwise, set OCPU_ADD_MATCH if the other
	 * CPU saw an error at the matching address.  The following code
	 * does this in a slightly more straightforward way.
	 */

	/*
	 * Get the CAD bits latched by the other CPU's B-Cache
	 * controller.  If not latched, decide whether to set MIXED_ERRORS.
	 */
	if ((elco->elco_data.elco_bcue & BCUE_UEL)
	    && (elco->elco_data.elco_bcue & BCUE_ISBCL)) {
		cad = elco->elco_data.elco_bcuea & 0xFFFFFFFF;
	}
	else if ((elco->elco_data.elco_bcue & BCUE_UEH)
		 && (elco->elco_data.elco_bcue & BCUE_ISBCH)) {
		cad = elco->elco_data.elco_bcuea >> 32;
	}			
	else {
		if (elco->elco_data.elco_bcue & BCUE_MUE) {
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_EVC_MIXED_ERRORS);
		}
		return detected;
	}

	/*
	 * Adjust the CAD bits, based on cache size, to yield the PA
	 * of the cache line which caused the error.
	 */
	switch ((elco->elco_data.elco_bcc & BCC_CSZL) >> BCC_CSZL_SHIFT) {
	case 1: /* 1 MB Cache */
		pa = (((cad & 0x3fff) << 5) | (cad & 0x7ff00000));
		break;
	case 2: /* 4 MB Cache */
	case 3: /* 4 MB Cache */
		pa = (((cad & 0xfff) << 5) | (cad & 0x7fc00000));
		break;
	default:
		/* 
		 * No point in panicking since this is diagnostic
		 * only.
		 */
		return detected;
		/* panic("kn430_ocpu_bcache_uncorr"); */
	}
		
	/*
	 * Compare the other CPU's failing B-Cache address to this CPU's 
	 * failing B-Cache address.  If this CPU didn't see an error, then we
	 * must be seeing a mixed error.
	 */
	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
				 (COBRA_CPU0I_C3_C_UNCORR +
				  cpuid * COBRA_CPU_SHIFT))) {
		if (pa == (elcmc->elcmc_data.elcmc_fill_addr 
			   & (CBUS_LK << 5))) {
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_EVC_C3_OCPU_ADD_MATCH);
		}
	}
	else {
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_EVC_MIXED_ERRORS);
	}

	return detected;
}

/*
 * Check for CPU C3 Read Data Parity Error. ($ND:9)
 *
 * Test Mask:		C3-CBE<38|6>
 * Missed Error Mask:	C3-CBE<39|7>
 * Clear And Mask:	C3-CBE<39,38,7,6>
 * Clear Or Mask:	none
 *
 * Note: C3-CBE<45:42,13:10> identify failing LW.  These are ro bits, no need
 *	 to clear.
 * Note: Bad data has gotten to the processor and the B-Cache.
 */
static long kn430_parse_c3_rd_par
	(register u_long cbe,
	 register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid)
{
	register long detected = FALSE;

	if (cbe & (CBE_RDPE|CBE_MRDPE)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_CPU0_C3_RD_PAR + 
				      cpuid * COBRA_CPU_SHIFT));
		Cpu_regs[cpuid]->cbe |= (CBE_RDPE|CBE_MRDPE);
	}

	return detected;
}

/*
 * CPU as Responder saw Cobra Bus Write Data Parity Error.  ($ND:12)
 *
 * Test Mask:		C3-CBE<36|4>
 * Missed Error Mask:	C3-CBE<37|5>
 * Clear And Mask:	C3-CBE<37,36,5,4>
 * Clear Or Mask:	none
 *
 */
static long kn430_parse_c3_wd_par
	(register u_long cbe,
	 register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid)
{
	register long detected = FALSE;

	if (cbe & (CBE_WDPE|CBE_MWDPE)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags, 
				     COBRA_C3_0_WD_PAR + cpuid);
		Cpu_regs[cpuid]->cbe &= (CBE_WDPE|CBE_MWDPE);
	}

	return detected;
}

/*
 * CPU saw C/A Parity Error.  ($ND:8)
 *
 * Test Mask:		C3-CBE<34|2>
 * Missed Error Mask:	C3-CBE<35|3>
 * Clear And Mask:	C3-CBE<35,34,3,2>
 * Clear Or Mask:	none
 *
 * If all nodes present on the system see the error, the fault is most likely
 * with the CBus Commander or Backplane.  If only one sees the error, it is
 * probable that the fault is in the module seeing the error.
 *
 * It may be possible to figure out which bit in the C/A went bad by comparing
 * the C/A data trapped in the Memory and I/O CSRs with the address we thought
 * was sent out.
 *
 * NOTE: Each callout of $ND:8 in the parse tree is handled separately.  To
 *	 test how many boards saw the error, check the software flags.
 */
static long kn430_parse_c3_ca_par
	(register u_long cbe,
	 register struct el_cobra_mcheck_event_header *elch,
	 register int cpuid)
{
	register long detected = FALSE;

	if (cbe & (CBE_CAPE|CBE_MCAPE)) {
		COBRA_ERROR_FLAG_SET(elch->elch_flags,
				     (COBRA_C3_0_CA_PAR + cpuid));
		Cpu_regs[cpuid]->cbe &= (CBE_CAPE|CBE_MCAPE);
	}

	return detected;
}

/*
 * I/O Module Gate Array Synchronization Error.  ($ND:21)
 *
 * Test Mask:		IO-CERR1<46|14>
 * Missed Error Mask:	none
 * Clear And Mask:	IO-CERR1<46,14>
 * Clear Or Mask:	none
 *
 * Our initial assumption is that this error is fatal.
 */
static long kn430_parse_io_gate_sync
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & CERR1_CSE) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_INT_SCSSTALL);
		Io_regs->cerr1 &= CERR1_CSE;
	}

	return detected;
}

/*
 * I/O Module CBus Synchronization Error.  ($ND:22)
 *
 * Test Mask:		IO-CERR1<17>
 * Missed Error Mask:	none
 * Clear And Mask:	IO-CERR1<17>
 * Clear Or Mask:	none
 *
 * Our initial assumption is that this error is fatal.
 */
static long kn430_parse_io_bus_sync
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & CERR1_BSE) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_INT_BUSSYNC);
		Io_regs->cerr1 &= CERR1_BSE;
	}

	return detected;
}

/*
 * I/O as commander, CBus Write Data Noack.  $(ND:16)
 *
 * Test Mask:		IO-CERR1<48|16>
 * Missed Error Mask:	none
 * Clear And Mask:	IO-CERR1<48|16>
 * Clear Or Mask:	none
 *
 * Note that there is a difference in terminology between the CPU spec and the
 * I/O spec.  When a responder receives write data which has bad parity, it
 * does not acknowledge the CBus cycle.  Thus a "Write Data No-Ack" is the same
 * thing as a "Write Data Parity Error".
 */
static long kn430_parse_io_wd_noack
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	/*
	 */
	if (cerr1 & CERR1_CWDPE) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_EXT_WD_NOACK);
		Io_regs->cerr1 &= CERR1_CWDPE;
	}

	return detected;
}

/*
 * I/O as responder, detected Cobra Bus write data parity error.  ($ND:11)
 *
 * Test Mask:		IO-CERR1<36|4>
 * Missed Error Mask:	IO-CERR1<37|5>
 * Clear And Mask:	IO-CERR1<45:42,37,36,13:10,5,4>
 * Clear Or Mask:	none
 */
static long kn430_parse_io_wd_par
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & (CERR1_WDPE|CERR1_MWDPE)) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_WD_PAR);
		Io_regs->cerr1 &= (CERR1_WDPE|CERR1_MWDPE|CERR1_DPESUM);
	}

	return detected;
}

/*
 * I/O as commander, uncorrectable read data (CUCERR).  ($ND:20)
 *
 * Test Mask:		IO-CERR1<32|0>
 * Missed Error Mask:	none
 * Clear And Mask:	IO-CERR1<32|0>
 * Clear Or Mask:	none
 *
 * Should check both memory and any CPU for the source of the error.  The
 * memory might be fine, but either CPU could have provided bad data from
 * B-Cache, or poisoned the transaction with CUCERR due to a tag parity error.
 * (This is not handled by the machine check parse, but rather by fault
 * analysis tools).
 */
static long kn430_parse_io_rd_uncorr
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & CERR1_URE) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_EXT_CB_UNCORR);
		Io_regs->cerr1 &= CERR1_URE;
	}

	return detected;
}

/*
 * I/O as commander, Cobra Bus Read Parity Error.  ($ND:17)
 *
 * Test Mask:		IO-CERR1<38,6>
 * Missed Error Mask:	IO-CERR1<39|7>
 * Clear And Mask:	IO-CERR1<39,38,7,6,45:42,13:10>
 * Clear Or Mask:	none
 *
 * Note: These are system fatal.
 */
static long kn430_parse_io_rd_par
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & (CERR1_RDPE|CERR1_MRDPE)) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_EXT_RD_PAR);
		Io_regs->cerr1 &= (CERR1_RDPE|CERR1_MRDPE|CERR1_DPESUM);
	}

	return detected;
}

/*
 * I/O as commander, Cobra Bus C/A Noack.  ($ND:18)
 *
 * Test Mask:		IO-CERR1<33,1>
 * Missed Error Mask:	none
 * Clear And Mask:	IO-CERR1<33,1>
 * Clear Or Mask:	none
 *
 * I/O module cycles that access the C-Bus:
 *	Mailbox Engine memory access
 *	F-Bus DMA memory access
 *	L-Bus DMA memory access
 *
 * Drivers must check to see if any device on the LBus saw a C/A parity error
 * go by.
 */
static long kn430_parse_io_ca_noack
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & CERR1_CANA) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_EXT_CA_NOACK);
		Io_regs->cerr1 &= CERR1_CANA;
	}

	return detected;
}

/*
 * I/O as responder saw Cobra Bus C/A Parity Error.  ($ND:8)
 *
 * Test Mask:		IO_CERR1<34|2>
 * Missed Error Mask:	IO_CERR1<35|3>
 * Clear And Mask:	IO_CERR1<41,40,35,34,9,8,3,2>
 * Clear Or Mask:	none
 *
 * If all nodes present on the system see the error, the fault is most likely
 * with the CBus Commander or Backplane.  If only one sees the error, it is
 * probable that the fault is in the module seeing the error.
 *
 * It may be possible to figure out which bit in the C/A went bad by comparing
 * the C/A data trapped in the Memory and I/O CSRs with the address we thought
 * was sent out.
 *
 * NOTE: Each callout of $ND:8 in the parse tree is handled separately.  To
 *	 test how many boards saw the error, check the software flags.
 */
static long kn430_parse_io_ca_par
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & (CERR1_CAPE|CERR1_MCAPE)) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, COBRA_IO_CA_PAR);
		Io_regs->cerr1 &= (CERR1_CAPE|CERR1_MCAPE|CERR1_CAPESUM);
	}

	return detected;
}

/*
 * Parity error on F-Bus+ during DMA.  ($ND:23)
 *
 * Test Mask:		IO_FERR1<33|32|1|0>
 * Missed Error Mask:	none
 * Clear And Mask:	IO_FERR1<33,32,1,0>
 * Clear Or Mask:	none
 *
 * These are system fatal, as there is no way to determine which driver or
 * process owns the bad data.
 */
static long kn430_parse_fbus_dma
	(register u_long ferr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (ferr1 & (FERR1_DPE|FERR1_APE)) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, 
				     COBRA_IO_INT_FB_DMA_PAR);
		Io_regs->ferr1 &= (FERR1_DPE|FERR1_APE);
	}

	return detected;
}

/*
 * Parity error on F-Bus+ during mailbox access.  ($ND:24)
 *
 * Test Mask:		IO_CERR1<15>
 * Missed Error Mask:	none
 * Clear And Mask:	IO_CERR1<15>
 * Clear Or Mask:	none
 *
 * These are system fatal, as there is no way to determine the owner of the
 * bad data.
 */
static long kn430_parse_fbus_mbx
	(register u_long cerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (cerr1 & CERR1_FME) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, 
				     COBRA_IO_INT_FB_MB_PAR);
		Io_regs->cerr1 &= CERR1_FME;
	}

	return detected;
}

/*
 * Parity error on L-Bus+ during dma access.  $(ND:25)
 *
 * Test Mask:		IO_LERR1<32|0>
 * Missed Error Mask:	none
 * Clear And Mask:	IO_LERR1<32,0>
 * Clear Or Mask:	none
 * 
 * Any L-Bus parity error is system fatal, as there is no way to identify
 * the owner of the corrupted data.
 */
static long kn430_parse_lbus_dma
	(register u_long lerr1,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long detected = FALSE;

	if (lerr1 & LERR1_DPE) {
		detected = TRUE;
		COBRA_ERROR_FLAG_SET(elch->elch_flags, 
				     COBRA_IO_INT_LB_DMA_PAR);
		Io_regs->cerr1 &= LERR1_DPE;
	}

	return detected;
}

/*
 * Memory module synchronization error.  ($ND:29)
 *
 * Test Mask:		MEM_ERR1(CSR0)<33|1>
 * Missed Error Mask:	none
 * Clear And Mask:	MEM_ERR1(CSR0)<33|1>
 * Clear Or Mask:	none
 *
 * This error indicates that the two gate arrays have lost synchronization.
 * If we see this, the memory is very sick...
 */
static long kn430_parse_mem_sync
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		elcm = mctl->mctl_frame.mcfr_mem[i];
		if (elcm && (elcm->elcm_data.elcm_merr & MERR_SYNC)) {
			detected |= 1 << i;
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_MEM0_SYNC_ERROR + i);
			Mem_regs[i]->error &= MERR_SYNC;
		}
	}
	return detected;
}

/*
 * Memory module saw C/A parity error.  ($ND:8)
 *
 * Test Mask:		MEM_ERR1(CSR0)<34|2>
 * Missed Error Mask:	MEM_ERR1(CSR0)<35|3>
 * Clear And Mask:	MEM_ERR1(CSR0)<35,34,3,2>
 * Clear Or Mask:	none
 *
 * If all nodes present on the system see the error, the fault is most likely
 * with the CBus Commander or Backplane.  If only one sees the error, it is
 * probable that the fault is in the module seeing the error.
 *
 * It may be possible to figure out which bit in the C/A went bad by comparing
 * the C/A data trapped in the Memory and I/O CSRs with the address we thought
 * was sent out.
 *
 * NOTE: Each callout of $ND:8 in the parse tree is handled separately.  To
 *	 test how many boards saw the error, check the software flags.
 */
static long kn430_parse_mem_ca_par
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		elcm = mctl->mctl_frame.mcfr_mem[i];
		if (elcm 
		    && (elcm->elcm_data.elcm_merr & (MERR_CAPE|MERR_MCAPE))) {
			detected = 1 << i;
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_MEM0_CA_PAR + i);
			Mem_regs[i]->error &= MERR_CAPE|MERR_MCAPE|MERR_ES;
		}
	}
	return detected;
}

/*
 * Memory Correctable EDC Error.  ($ND:15)
 *
 * Test Mask:		MEM_ERR1(CSR0)<50|18>
 * Missed Error Mask:	MEM_ERR1(CSR0)<51|19>
 * Clear And Mask:	MEM_ERR1(CSR0)<51,50,19,18>
 * Clear Or Mask:	none
 *
 * Due to a problem in the Memory Gate Arrays, multi CPU configurations can
 * receive mis-corrected data from a normally correctable error.  As a
 * workaround, the console disables error correction on the memories that have
 * these gate arrays if two CPUs are present.
 *
 * If EDC correction is disabled, set software flags to indicate this and the
 * system will come down.
 *
 * It is important to check that there are no other errors latched, for example
 * Uncorrectable Read Error, C/A or Write Data Parity Errors, as these result
 * in the command trap 1 (CSR 1) register being overwritten.
 *
 * Command trap 1 (CSR 1) bits <31:3> hold the failing physical address
 * bits <33:5>.
 *
 * The fault analysis tools need to check for errors which could wipe out
 * CSR 1.  Uncorrectable errors have higher precedence than correctable errors,
 * and can overwrite the latched correctable error information.
 */
static long kn430_parse_mem_edc
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		elcm = mctl->mctl_frame.mcfr_mem[i];
		if (elcm) {
			if (elcm->elcm_data.elcm_medcc & EDCCTL_DISEDCC) {
				COBRA_ERROR_FLAG_SET(elch->elch_flags,
						     COBRA_MEM0_CORRDIS + i);
			}
			if (elcm->elcm_data.elcm_merr 
			    & (MERR_EDCCE|MERR_MEDCCE)) {
				detected |= 1 << i;
				COBRA_ERROR_FLAG_SET(elch->elch_flags,
						     COBRA_MEM0_CORR + i);
				Mem_regs[i]->error &= (MERR_EDCCE|MERR_MEDCCE
						       |MERR_ES);
			}
		}
	}
	return detected;
}

/*
 * Memory as responder, detected CBus write data parity error.  ($ND:13)
 *
 * Test Mask:		MEM_ERR1(CSR0)<36|4>
 * Missed Error Mask:	MEM_ERR1(CSR0)<37|5>
 * Clear And Mask:	MEM_ERR1(CSR0)<37,36,5,4>
 * Clear Or Mask:	none
 */
static long kn430_parse_mem_wd_par
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		elcm = mctl->mctl_frame.mcfr_mem[i];
		if (elcm 
		    && (elcm->elcm_data.elcm_merr & (MERR_WDPE|MERR_MWDPE))) {
			detected |= 1 << i;
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_MEM0_WD_PAR + i);
			Mem_regs[i]->error &= MERR_WDPE|MERR_MWDPE|MERR_ES;
		}
	}
	return detected;
}

/*
 * Memory Uncorrectable error.  ($ND:1)
 *
 * Test Mask:		MEM_ERR1(CSR0)<48|16>
 * Missed Error Mask:	MEM_ERR1(CSR0)<49|17>
 * Clear And Mask:	MEM_ERR1(CSR0)<51,50,49,48,19,18,17,16>
 *			(includes correctable error bits because if this is a
 *			 second error, we will lose the state when we clear the
 *			 command trap registers).
 * Clear Or Mask:	none
 *
 */
static long kn430_parse_mem_uncorr
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		elcm = mctl->mctl_frame.mcfr_mem[i];
		if (elcm 
		    && (elcm->elcm_data.elcm_merr 
			& (MERR_EDCUE|MERR_MEDCUE))) {
			detected |= 1 << i;
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_MEM0_UNCORR + i);
			Mem_regs[i]->error &= (MERR_EDCUE|MERR_MEDCUE
					       |MERR_EDCCE|MERR_MEDCCE);
		}
	}
	return detected;
}

/*
 * Correlate Memory Uncorrectable error to CPU module.  ($ND:1A)
 *
 * The FILL_ADDR register holds the PA associated with errors reported by
 * BIU_STAT[14..8].  Its contents are meaningful only when FILL_EDC or
 * FILL_DPERR is set.  Reads of FILL_ADDR unlock FILL_ADDR, BIU_STAT[14..8],
 * and FILL_SYNDROME.
 *
 * In the 21064, FILL_ADDR<33..5> identify the 32-byte cache block which the
 * CPU was attempting to read when the error occurred.
 */
static long kn430_parse_cpu_mem_uncorr
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch,
	 register u_long fill_addr)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		if ((elcm = mctl->mctl_frame.mcfr_mem[i])
		    && ((MCMD_PA(elcm->elcm_data.elcm_mcmd1) & ~0x1F) == 
			(fill_addr & (CBUS_LK << 5)))) {
			COBRA_ERROR_FLAG_SET(elch->elch_flags,
					     COBRA_EVC_C3_MEM_R_ERROR);
			detected |= 1 << i;
		}
	}
	return detected;
}

/*
 * Correlate Memory Uncorrectable error to I/O module.  ($ND:1B)
 *
 * If the two modules captured the same physical address, the error was caused
 * by the memory module.
 */
static long kn430_parse_io_mem_uncorr
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register u_long io_cerr2, io_cerr3;
	register struct el_cobra_frame_memory *elcm;
	register long detected = FALSE;

	/*
	 * COBRA_IO_EXT_CB_UNCORR can only get set if mcfr_io is nonzero.
	 */
	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
				 COBRA_IO_EXT_CB_UNCORR)) {
		io_cerr2 = mctl->mctl_frame.mcfr_io->elci_data.elci_cerr2;
		io_cerr3 = mctl->mctl_frame.mcfr_io->elci_data.elci_cerr3;
		for(i = 0; i < CBUS_MEM_SLOTS; i++) {
			if ((elcm = mctl->mctl_frame.mcfr_mem[i])
			    && (MCMD_PA(io_cerr2) == 
				MCMD_PA(elcm->elcm_data.elcm_mcmd1))
			    && (MCMD_PA(io_cerr3) == 
				MCMD_PA(elcm->elcm_data.elcm_mcmd2))) {
				COBRA_ERROR_FLAG_SET(elch->elch_flags,
						     COBRA_EVC_IO_MEM_R_ERROR);
				detected |= 1 << i;
			}
		}
	}
	return detected;
}

static long kn430_mcheck_dispatch
	(register struct kn430_mcheck_control *mctl,
	 register struct kn430_scratch_buffer *sb,
	 register int severity,
	 register long suppress)
{
	if (!suppress || (severity > MCHECK_SEV_SURVIVABLE)) {
		kn430_log_mcheck(mctl, kn430_copy_to_errlog(sb));
	}

	/*
	 * Take the appropriate action based on the severity of the error.
	 */
	switch (severity) {
	case MCHECK_SEV_SURVIVABLE:
	case MCHECK_SEV_PROBE:
		break;
	case MCHECK_SEV_PROCESS_FATAL:
		panic("process-fatal machine check");
		/* NOTREACHED */
		break;
	case MCHECK_SEV_PROCESSOR_FATAL:

	case MCHECK_SEV_SYSTEM_FATAL:
	default:
		panic("system-fatal machine check");
		/* NOTREACHED */
		break;
	}
	
	/*
	 * Indicate that the machine check handler was in place.
	 */
	return 0;
}

static void kn430_log_mcheck
	(register struct kn430_mcheck_control *mctl,
	 register struct el_rec *elrec)
{
	register long summarized;

	/*
	 * Summarize the error to the console (and syslog).
	 */
	if (kn430_syslog_enabled) {
		summarized = kn430_summarize_errors(mctl);
	}
	else {
		summarized = 0;
	}

	/*
	 * If the error was not summarized, CSR dumps are being forced, or the
	 * packet was not written to the error log, dump the entire packet.
	 */
	if ((kn430_csrdump_enabled && (!summarized || !elrec))
	    || kn430_csrdump_always) {
		kn430_dump_errlog_packet(mctl, elrec);
	}

	/*
	 * Release the error log frame.
	 */
	if (elrec) {
		binlog_valid((char *)elrec);
	}
}

/*****************************************************************************
 * DEC 4000 CSR CAPTURE							     *
 *****************************************************************************
 *
 * CSRs are captured in a scratch buffer at the start of the machine check
 * parse.  Pointers are kept in the mctl structure so that the machine check
 * frame does not have to be reparsed to get at specific CSR copies.
 */

static void cobra_init_frames(register struct kn430_mcheck_control *mctl)
{
	register struct kn430_mcheck_control_frames *mcfr;
	register long i;

	mcfr = &mctl->mctl_frame;

	mcfr->mcfr_hdr = (struct el_cobra_mcheck_event_header *)0;
	mcfr->mcfr_mcheck = (struct el_cobra_frame_mcheck *)0;
	mcfr->mcfr_pbccorr = (struct el_cobra_frame_proc_bcache_corr *)0;
	mcfr->mcfr_ocpu = (struct el_cobra_frame_other_cpu *)0;
	mcfr->mcfr_io = (struct el_cobra_frame_io *)0;
	for(i = 0; i < CBUS_MEM_SLOTS; i++) {
		mcfr->mcfr_mem[i] = (struct el_cobra_frame_memory *)0;
	}
}

static struct el_cobra_frame_mcheck *kn430_get_frame_mcheck
 	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_data_mcheck *lgt,
	 register struct kn430_scratch_buffer *sb)
{
 	register struct kn430_mcheck_control_frames *mcfr;
 	register long size;
	register struct el_cobra_frame_mcheck *elcmc;

 	mcfr = &mctl->mctl_frame;
	size = MAX(sizeof(struct el_cobra_data_mcheck),
		   lgt->elcmc_lhdr.elcl_size);
	
	elcmc = kn430_alloc_elbuf_frame(sb, size, COBRA_FID_MCHECK);
	if (elcmc) bcopy(lgt, &elcmc->elcmc_data, size);
	mcfr->mcfr_mcheck = elcmc;
 	return elcmc;
};

static struct el_cobra_frame_proc_bcache_corr *
kn430_get_frame_pbccorr
 	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_data_proc_bcache_corr *lgt,
	 register struct kn430_scratch_buffer *sb)
{
 	register struct kn430_mcheck_control_frames *mcfr;
 	register long size;
	register struct el_cobra_frame_proc_bcache_corr *elcpb;

 	mcfr = &mctl->mctl_frame;
	size = MAX(sizeof(struct el_cobra_data_proc_bcache_corr),
		   lgt->elcpb_lhdr.elcl_size);
	elcpb = kn430_alloc_elbuf_frame(sb, size, COBRA_FID_P_BC_CORR); 
	if (elcpb) bcopy(lgt, &elcpb->elcpb_data, size);
	mcfr->mcfr_pbccorr = elcpb;
	return elcpb;
};

static struct el_cobra_frame_other_cpu *kn430_get_frame_ocpu
 	(register struct kn430_mcheck_control *mctl,
	 register int ocpuid,
	 register struct kn430_scratch_buffer *sb)
{
 	register struct kn430_mcheck_control_frames *mcfr;
	register struct el_cobra_frame_other_cpu *elco;

 	mcfr = &mctl->mctl_frame;
	elco = kn430_alloc_elbuf_frame(sb,
				       sizeof(struct
					      el_cobra_frame_other_cpu), 
				       COBRA_FID_OCPU);
	if (elco) {
		register struct cpu_csr *ocpu = Cpu_regs[ocpuid];

		elco->elco_data.elco_bcc = ocpu->bcc;
		elco->elco_data.elco_bcce = ocpu->bcce;
		elco->elco_data.elco_bccea = ocpu->bccea;
		elco->elco_data.elco_bcue = ocpu->bcue;
		elco->elco_data.elco_bcuea = ocpu->bcuea;
		elco->elco_data.elco_dter = ocpu->dter;
		elco->elco_data.elco_cbctl = ocpu->cbctl;
		elco->elco_data.elco_cbe = ocpu->cbe;
		elco->elco_data.elco_cbeal = ocpu->cbeal;
		elco->elco_data.elco_cbeah = ocpu->cbeah;
		elco->elco_data.elco_pmbx = ocpu->pmbx;
		elco->elco_data.elco_ipir = ocpu->ipir;
		elco->elco_data.elco_sic = ocpu->sic;
		elco->elco_data.elco_adlk = ocpu->adlk;
		elco->elco_data.elco_madrl = ocpu->madrl;
	}
	mcfr->mcfr_ocpu = elco;
 	return elco;
};

static struct el_cobra_frame_io *kn430_get_frame_io
 	(register struct kn430_mcheck_control *mctl,
	 register struct kn430_scratch_buffer *sb)
{
	register struct el_cobra_frame_io *elci;
 	register struct kn430_mcheck_control_frames *mcfr;
 	register long size;


 	mcfr = &mctl->mctl_frame;
	elci = kn430_alloc_elbuf_frame(sb,
				       sizeof(struct el_cobra_frame_io),
				       COBRA_FID_IO);
	if (elci) {
		register struct io_csr *io;

		io = (struct io_csr *)PHYS_TO_KSEG(IO_BASE);
		elci->elci_data.elci_iocsr = io->iocsr;
		elci->elci_data.elci_cerr1 = io->cerr1;
		elci->elci_data.elci_cerr2 = io->cerr2;
		elci->elci_data.elci_cerr3 = io->cerr3;
		elci->elci_data.elci_lmbpr = io->lmbpr;
		elci->elci_data.elci_fmbpr = io->fmbpr;
		elci->elci_data.elci_diagcsr = io->diagcsr;
		elci->elci_data.elci_ferr1 = io->ferr1;
		elci->elci_data.elci_ferr2 = io->ferr2;
		elci->elci_data.elci_lint = io->io_lint;
		elci->elci_data.elci_lerr1 = io->lerr1;
		elci->elci_data.elci_lerr2 = io->lerr2;
 	}
 	mcfr->mcfr_io = elci;
 	return elci;
};

static struct el_cobra_frame_memory *kn430_get_frame_mem
 	(register struct kn430_mcheck_control *mctl,
	 register int slot,
	 register struct kn430_scratch_buffer *sb)
{
	register struct el_cobra_frame_memory *elcm;
 	register struct kn430_mcheck_control_frames *mcfr;

 	mcfr = &mctl->mctl_frame;
	elcm = kn430_alloc_elbuf_frame(sb,
				       sizeof(struct el_cobra_frame_memory),
				       COBRA_FID_MEMORY);
	if (elcm) {
		register struct mem_csr *mem;

		mem = Mem_regs[slot];

		elcm->elcm_data.elcm_merr = mem->error;   
		elcm->elcm_data.elcm_mcmd1 = mem->ctrap1;  
		elcm->elcm_data.elcm_mcmd2 = mem->ctrap2;  
		elcm->elcm_data.elcm_mconf = mem->config;  
		elcm->elcm_data.elcm_medc1 = mem->edcstat1;
		elcm->elcm_data.elcm_medc2 = mem->edcstat2;
		elcm->elcm_data.elcm_medcc = mem->edcctl;  
		elcm->elcm_data.elcm_msctl = mem->stream;  
		elcm->elcm_data.elcm_mref = mem->refresh; 
		elcm->elcm_data.elcm_module = ((mem->config & MCONF_SLOTL) 
					       >> MCONF_SLOTL_SHIFT);
	}
	
 	mcfr->mcfr_mem[slot] = elcm;
 	return elcm;
};

/*****************************************************************************
 * DEC4000 Memory EDC Scrub and Statistics Gathering			     *
 *									     *
 * Record footprints of memory CRDs.  Report CRD summary information, and    *
 * suppress CRD reporting if getting swamped with interrupts.		     *
 *****************************************************************************/

/*
 * Parameters for Footprint and CRD Suppression
 *
 * The DEC 4000 memory subsystem provides a CRD filter which may be used to
 * suppress reporting of a particular syndrome by one CMIC.  It is possible
 * to use this filter to suppress the events matching one footprint.  This
 * might be needed if there is a stuck bit but otherwise memory is functioning
 * well.
 *
 * CRD reporting can also be suppressed for an entire CMIC (or the entire
 * board by suppressing both CMICs), by setting bits in the board's config
 * register.  This can be done if there are multiple stuck bits with different
 * syndromes.
 *
 * Footprint suppression occurs when a footprint has been matched
 * KN430_FPRINT_MATCH_THRESHOLD times.  A footprint is logged immediately
 * when it is suppressed.  Otherwise, footprints are held indefinitely, until
 * a system panic or until the next checkpoint interval.  At the checkpoint
 * interval, only "interesting" footprints are written out, to conserve
 * log space.
 */

/* Number of times to _retry_ if first scrub attempt fails. */
#define KN430_SCRUB_RETRIES 31

/* Number of Memory EDC footprints to collect before flushing them out. */
#define KN430_FPRINTS_MAX 16

/* Reporting of a footprint is suppressed when this threshold is reached. */
#define KN430_FPRINT_MATCH_THRESHOLD 64

/*
 * CRD Footprint Checkpoint Interval (seconds)
 *
 * Every N seconds, check the footprint structures and write any which have
 * changed since last checked.  Do not deallocate the footprints.  This is
 * necessary because we cannot reliably flush the footprints during normal
 * system shutdown.
 */
long kn430_edc_checkpoint_interval = (6*60*60); /* 6 hours */

/*
 * Lock for accessing footprint information.
 */
decl_simple_lock_data(, kn430_fprint_lock)
struct kn430_edc_fprint_entry *kn430_fprint;

/*
 * Flag to indicate that CRD information was lost.  Currently this can only
 * happen if an error log packet could not be allocated.  Will only be logged
 * if a subsequent packet is allocated successfully.
 */
long kn430_lost_edc_packet = 0;

/*
 * Array of FRU information for memory modules (gets copied into memory
 * EDC packets).
 */
struct el_cobra_fru_mem kn430_meminfo[CBUS_MEM_SLOTS];

/*
 * Initialize KN430 CRD reporting data structures.
 *
 */
static void kn430_edc_init()
{
	/*
	 * Allocate and initialize an array of footprint structures.
	 */
	kn430_fprint = ((struct kn430_edc_fprint_entry *)
			  kmem_alloc(kernel_map,
				     (sizeof(struct kn430_edc_fprint_entry)
				      * KN430_FPRINTS_MAX)));
	if (!kn430_fprint) panic("kn430_edc_init");
	
	simple_lock_init(&kn430_fprint_lock);
	kn430_edc_reset();

	/* Register ourselves so we will be called when threads are initialized 
	 * and we can then start our CRD footprint thread 
	 */

	kd_thread_register(kn430_edc_create_thread,0);
}

/*
 * Initialize KN430 CRD reporting data structures.  For testing purposes, this
 * is callable externally.  Should only be called from a quiescent state.
 */
void kn430_edc_reset()
{
	register struct kn430_edc_fprint_entry *fp;
	register long i, now;
	register int s;

	s = splextreme();
	simple_lock(&kn430_fprint_lock);
	for(i = 0, fp = kn430_fprint; 
	    i < KN430_FPRINTS_MAX; 
	    i++, fp++) {
		fp->cfe_fp.kn430_mc_static_flags = 0;
	}
	simple_unlock(&kn430_fprint_lock);

	kn430_lost_edc_packet = 0;
	splx(s);
}

/*
 * Check for Memory ECC notifications, and scrub the locations.
 * 
 * The Physical Address of the location is needed in order to perform the
 * scrub.  This is obtained from the latched CAD bits in the appropriate CSR.
 *
 * Note that the bits latched only represent the EDC address if the EDC
 * bit is on, but the MEMx_CORR flag gets set if the MEDC bit is on but the
 * EDC bit is off.
 */

static long kn430_handle_edcs
	(register struct kn430_mcheck_control *mctl,
	 register struct el_cobra_mcheck_event_header *elch)
{
	register long i;
	register struct el_cobra_frame_memory *elcm;
	register long scrubbed = FALSE;

	/*
	 * Go through all four memory slots.  Only populated slots could have
	 * caused a COBRA_MEMx_CORR flag to be set.
	 */
	for(i = 0; i < CBUS_MEM_SLOTS; i++) {

		/* Was a correctible error detected for this module? */
		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_MEM0_CORR + i)) {


			/* Get the frame containing the memory CSRs. */
			elcm = mctl->mctl_frame.mcfr_mem[i];


			/* Try to scrub and record any latched PA/syndrome. */
			if (elcm->elcm_data.elcm_merr & MERR_EDCCEL) {
				if (kn430_handle_edc(elcm, i, 0)) {
					scrubbed |= 1 << i;
				}
			}
			if (elcm->elcm_data.elcm_merr & MERR_EDCCEH) {
				if (kn430_handle_edc(elcm, i, 1)) {
					scrubbed |= 1 << i;
				}
			} 
		}
	}
	return scrubbed;
}

/*
 * Attempt to scrub, filter, and record a latched EDC error.  Disable
 * EDC reporting for the board while scrubbing, to prevent further EDC
 * reports for this error.
 */
static long kn430_handle_edc
	(register struct el_cobra_frame_memory *elcm,
	 register int slot,
	 register int high_cmic) 			    /* 0(L) or 1(H) */
{
	register long scrubbed;
	register long pa;		  /* Physical address of error	    */
	register vm_offset_t va;	  /* KSEG address of error	    */
	register u_long edcctl;
	
	/* Find the PA and VA for the error */
	pa = kn430_pa(elcm, high_cmic);
	va = PHYS_TO_KSEG(pa);

	/* Disable the board's CRD reporting. */
	edcctl = Mem_regs[slot]->edcctl;
	Mem_regs[slot]->edcctl = edcctl & ~EDCCTL_ENACRD;

	/* Try to scrub the location. */
	scrubbed = alpha_scrub_long(va, KN430_SCRUB_RETRIES);

	/* Re-enable the board's CRD reporting. */
	Mem_regs[slot]->edcctl = edcctl | EDCCTL_ENACRD;

	kn430_log_edc(elcm, slot, high_cmic, kn430_edc_timestamp());
	return scrubbed;
}

/*
 * Log an EDC to a footprint entry.
 */

static void kn430_log_edc
	(register struct el_cobra_frame_memory *elcm,
	 register int slot,
	 register int high_cmic, 		     /* 0(L) or 1(H)	    */
	 register long timestamp)
{
	register long signature;
	register struct kn430_edc_fprint_entry *fp;

	signature = kn430_footprint_signature(elcm, high_cmic);

	fp = kn430_find_footprint(kn430_fprint, KN430_FPRINTS_MAX, signature);

	if (!fp) fp = kn430_alloc_footprint(kn430_fprint, KN430_FPRINTS_MAX,
					    signature,
					    kn430_pa(elcm, high_cmic),
					    timestamp);
		
	if (fp) {
		kn430_update_footprint(elcm, fp, high_cmic, timestamp);
	}
	else {
		kn430_disable_board_edcs(slot, high_cmic);
	}
}

static struct kn430_edc_fprint_entry *kn430_find_footprint
	(register struct kn430_edc_fprint_entry *fp,
	 register long fp_max,
	 register u_long signature)
{
	register long i;
	register int s;

	/* 
	 * Try to find a matching footprint.
	 */
	s = splextreme();
	simple_lock(&kn430_fprint_lock);
	for(i = 0; i < fp_max; i++, fp++) {
		if ((fp->cfe_fp.kn430_mc_static_flags & KN430_MC_SFLG_BUSY)
		    && (fp->cfe_fp.kn430_mc_signature == signature)) {
			break;
		}
	}
	simple_unlock(&kn430_fprint_lock);
	splx(s);

	if (i >= fp_max) fp = NULL;
	return fp;
}

struct kn430_edc_fprint_entry *kn430_alloc_footprint
	(register struct kn430_edc_fprint_entry *fp,
	 register long fp_max,
	 register long signature,
	 register long pa,
	 register long timestamp)
{
	register long i;
	register int s;

	s = splextreme();
	simple_lock(&kn430_fprint_lock);
	for(i = 0; i < fp_max; i++, fp++) {
		if (!(fp->cfe_fp.kn430_mc_static_flags 
		      & KN430_MC_SFLG_BUSY)) {
			break;
		}
	}
	simple_unlock(&kn430_fprint_lock);
	splx(s);

	if (i >= fp_max) {
		fp = NULL; 
	}
	else {
		kn430_init_footprint(fp, signature, pa, timestamp);
	}

	return fp;
}

static void kn430_init_footprint
	(register struct kn430_edc_fprint_entry *fp,
	 register long signature,
	 register long pa,
	 register long timestamp)
{
	register int s;

	s = splextreme();
	simple_lock(&kn430_fprint_lock);

	fp->cfe_flags = 0;
	fp->cfe_fp.kn430_mc_signature = signature;
	fp->cfe_fp.kn430_mc_time = timestamp;
	fp->cfe_fp.kn430_mc_lasttime = timestamp;
	fp->cfe_fp.kn430_mc_addr_low = pa;
	fp->cfe_fp.kn430_mc_addr_high = pa;
	fp->cfe_fp.kn430_mc_addr_cum = pa;
	fp->cfe_fp.kn430_mc_scrub_blksz = 0x20;
	fp->cfe_fp.kn430_mc_static_flags = KN430_MC_SFLG_BUSY;
	fp->cfe_fp.kn430_mc_reason = KN430_MC_RSN_DOMAIN_GREW;
	fp->cfe_fp.kn430_mc_caller_flags = 0;
	fp->cfe_fp.kn430_mc_match_cnt = 0;

	simple_unlock(&kn430_fprint_lock);
	splx(s);
}

static void kn430_update_footprint
	(register struct el_cobra_frame_memory *elcm,
	 register struct kn430_edc_fprint_entry *fp,
	 register int high_cmic,	  /* 0(L) or 1(H) */
	 register long timestamp)
{
	register vm_offset_t old_cum, new_cum;
	register int slot;
	register vm_offset_t pa;

	slot = elcm->elcm_data.elcm_module;
	pa = kn430_pa(elcm, high_cmic);
	
	/*
	 * Update the footprint.  If the domain grew, mark this in the flags
	 * so that it is written at the next checkpoint.  Currently we don't
	 * checkpoint if the match count was incremented but nothing else
	 * interesting happened.
	 *
	 * NOTE: The PFMS calls for an XOR against the original match address,
	 *	 then an OR with the CUM register.  Doing an XOR against the
	 *	 previous LOW value is equivalent (proof by induction left
	 *	 to the reader).
	 */
	simple_lock(&kn430_fprint_lock);
	fp->cfe_fp.kn430_mc_lasttime = timestamp;
	old_cum = fp->cfe_fp.kn430_mc_addr_cum;
	new_cum = old_cum | (fp->cfe_fp.kn430_mc_addr_low % pa);
	if (new_cum != old_cum) {
		fp->cfe_flags |= KN430_FPRINT_DOMAIN;
		fp->cfe_flags |= KN430_FPRINT_CHANGED;
	}
	if (pa < fp->cfe_fp.kn430_mc_addr_low) {
		fp->cfe_fp.kn430_mc_addr_low = pa;
	}
	if (pa > fp->cfe_fp.kn430_mc_addr_high) {
		fp->cfe_fp.kn430_mc_addr_high = pa;
	}
	fp->cfe_fp.kn430_mc_match_cnt++;

	/*
	 * If this footprint has been matched more than once, mark it as
	 * changed so that it gets checkpointed at the next scheduled interval.
	 * It's not likely to be a random occurrence if it matched more than 
	 * once.  Don't schedule it to be checkpointed if it has already been
	 * checkpointed at least once -- it will be picked up when it
	 * exceeds KN430_FPRINT_MATCH_THRESHOLD.
	 */
	if ((fp->cfe_fp.kn430_mc_match_cnt > 1) 
	    && !(fp->cfe_flags & KN430_FPRINT_CHKPTD)) {
		fp->cfe_flags |= KN430_FPRINT_CHANGED;
	}

	/*
	 * If this footprint has been matched enough times, suppress it,
	 * mark it as changed, and invoke a checkpoint to get it to the log
	 * immediately.
	 */
	if (fp->cfe_fp.kn430_mc_match_cnt > KN430_FPRINT_MATCH_THRESHOLD) {
		fp->cfe_flags |= KN430_FPRINT_CHANGED;
		simple_unlock(&kn430_fprint_lock);
		kn430_suppress_footprint(slot, pa, high_cmic, fp);
		kn430_checkpoint_footprints();
	}
	else simple_unlock(&kn430_fprint_lock);
}

/*
 * kn430_count_footprints
 *
 * Counts the number of footprints which need checkpointing.
 */
static int kn430_count_footprints()
{
	register int fprints;
	register struct kn430_edc_fprint_entry *fp, *fp_end;
	register int s;

	fprints = 0;
	fp_end = kn430_fprint + KN430_FPRINTS_MAX;
	s = splextreme();
	simple_lock(&kn430_fprint_lock);
	for(fp = kn430_fprint; fp < fp_end; fp++) {
		if ((fp->cfe_fp.kn430_mc_static_flags & KN430_MC_SFLG_BUSY)
		    && (fp->cfe_flags & ~KN430_FPRINT_CHKPTD)) {
			fprints++;
		}
	}
	simple_unlock(&kn430_fprint_lock);
	splx(s);
	return fprints;
}

/*
 * kn430_checkpoint_footprints
 *
 * Counts how many footprints need checkpointing (by checking each footprint's
 * flags).  Allocates a packet for that number of footprints (or less if there
 * isn't enough space in the error log buffer).  Writes out the footprints,
 * then checks to see if done.
 *
 * Returns the number of footprints remaining to be checkpointed.
 * 
 */
int kn430_checkpoint_footprints()
{
	struct el_rec *elrec;
	register struct el_edc_header *hdr;
	register long cfps;		/* Footprints needing checkpointing */
	register long fps;		/* Footprints remaining in packet */
	register struct kn430_edc_fprint_entry *fp, *fp_end;
	register struct kn430_edc_fprint *fpout;
	register int s;

	/* Count footprints to checkpoint. */
	cfps = kn430_count_footprints();

	/* Only checkpoint if there are footprints to do. */
	if (cfps > 0) {

		/* Allocate a frame to hold the footprints. */
		fps = kn430_start_edc_packet(&elrec, cfps, 0, 0);
		
		if (elrec) {
			hdr = (struct el_edc_header *)&elrec->el_body;

			/* Write out footprints. */
			fpout = ((struct kn430_edc_fprint *)
				 ((long)hdr + hdr->emb_fprint_offset));
			s = splextreme();
			simple_lock(&kn430_fprint_lock);
			fp_end = kn430_fprint + KN430_FPRINTS_MAX;
			for(fp = kn430_fprint; 
			    (fp < fp_end) && (fps > 0);
			    fp++) {
				if ((fp->cfe_fp.kn430_mc_static_flags
				     & KN430_MC_SFLG_BUSY)
				    && (fp->cfe_flags & 
					~KN430_FPRINT_CHKPTD)) {
					fp->cfe_flags = KN430_FPRINT_CHKPTD;
					splx(s);
					bcopy(&fp->cfe_fp, fpout, 
					      sizeof(struct kn430_edc_fprint));
					fpout++;
					fps--;
					splextreme();
				}
			}
			simple_unlock(&kn430_fprint_lock);
			splx(s);

			/* Validate the error log entry */
			binlog_valid((char *)elrec);
		}
	}
	return kn430_count_footprints();
}

static void kn430_suppress_footprint
	(register int slot,
	 register long pa,
	 register int high_cmic,
	 register struct kn430_edc_fprint_entry *fp)
{
	register int cshft;
        register long signature;
	register long suppressed = FALSE;
	register unsigned long new_filter, old_filter, mask;
	register unsigned long *csr;
	register int s;

	signature = fp->cfe_fp.kn430_mc_signature;
	cshft = (high_cmic ? (EDCSTAT2_SYNH_SHIFT - EDCSTAT2_SYNL_SHIFT) : 0);

	/*
	 * Report excessive footprint match.
	 */
	printf("Excessive memory ECC corrections for module %d, footprint:\n"
	       "  PA: 0x%l09x  SYNDROME: 0x%l012x  BANK: %d  CMIC: %s\n",
	       slot, pa,
	       (signature & MFP_SYNDROME) >> MFP_SYNDROME_SHIFT,
	       (signature & MFP_BANK) >> MFP_BANK_SHIFT,
	       cshft ? "high" : "low");

	/*
	 * Set up ECC filter settings.  Put the new filter settings in the
	 * proper bits of new_filter (all other bits 0).  Get the complete
	 * old filter in old_filter.
	 */
	s = splextreme();
	csr = &Mem_regs[slot]->filter;
	mask = (EDCF_SYNDROMEL | EDCF_BANKL | EDCF_ENBL) << cshft;
	new_filter = (((signature & MFP_SYNDROME) >> MFP_SYNDROME_SHIFT)
		      << EDCF_SYNDROMEL_SHIFT);
	new_filter |= (((signature & MFP_BANK) >> MFP_BANK_SHIFT)
		       << EDCF_BANKL_SHIFT);
	new_filter |= EDCF_ENBL;
	new_filter <<= cshft;
	old_filter = *csr;

	/*
	 * If not already suppressing an error for this CMIC, suppress this
	 * error.
	 *
	 * Note that pass 2 CMICs appear not to filter errors, hence the check
	 * for setting the same filter value.
	 */
	if (!((old_filter & (EDCF_ENBL << cshft))
	      && ((old_filter & mask) != new_filter))) {
		*csr = new_filter | (old_filter & ~mask);
		suppressed = TRUE;
	}
	splx(s);

	/*
	 * If the filter was already in use, print a message and disable the
	 * entire slice.
	 */
	if (((old_filter & (EDCF_ENBL << cshft))
	     && ((old_filter & mask) != new_filter))) {
		printf("Module %d CMIC %s ECC filter is already in use\n"
		       "  PA: 0x%l09x  SYNDROME: 0x%l012x"
		       "  BANK: %d\n",
		       slot, cshft ? "high" : "low",
		       pa,
		       (old_filter & EDCF_SYNDROMEL) >> EDCF_SYNDROMEL_SHIFT,
		       (old_filter & EDCF_BANKL) >> EDCF_BANKL_SHIFT);
		kn430_disable_board_edcs(slot, high_cmic);
	}
}

static void kn430_disable_board_edcs
	(register int slot,
	 register int high_cmic)
{
	register int s;

	s = splextreme();

	/*
	 * Disable the board.
	 */
	Mem_regs[slot]->edcctl &= ~(high_cmic ? 
				    EDCCTL_ENACRDH : 
				    EDCCTL_ENACRDL);
	splx(s);

	printf("Memory module %d ECC %s longword reporting disabled\n",
	       slot, high_cmic ? "odd" : "even");
}

/*
 * kn430_start_edc_packet
 *
 * Allocate a binary error log packet to hold a CRD report frame with the
 * specified number of footprints.  Fill in the header and memory FRU
 * descriptor table, and return a pointer to the packet.
 *
 * If there isn't enough room to allocate a packet for the requested number
 * of footprints, attempts will be made to allocate smaller packets.  This
 * routine returns the number of footprints which the packet can hold, which
 * will be equal to or less than the number of footprints requested.
 */
static long kn430_start_edc_packet
	(register struct el_rec **p_elrec,
	 register long fprints,
	 register long rsn,
	 register long crd_flags)
{
	register struct el_rec *elrec;
	register long size, boards, i;
	register struct el_edc_header *hdr;
	register struct kn430_memdsc *md;
	register long newfprints;

	/* Figure out how many boards are installed. */
	for(i = 0, boards = 0; i < CBUS_MEM_SLOTS; i++) {
		if (CBUS_MEM_ALIVE(i)) {
			boards++;
		}
	}

	/* Calculate size of error log entry. */
	size = (sizeof(struct el_edc_header)
		+ (sizeof(struct kn430_memdsc) * boards)
		+ (sizeof(struct kn430_edc_fprint) * fprints));

	/* 
	 * Allocate an error log buffer to hold the footprints.  If the error
	 * logger doesn't have a big enough buffer, try trimming the number of
	 * footprints.
	 */
	do {
		elrec = (struct el_rec *)binlog_alloc(size, EL_PRIHIGH);
		if (!elrec) {
			newfprints = fprints / 2;
			size = size - ((fprints - newfprints)
				       * sizeof(struct kn430_edc_fprint));
			fprints = newfprints;
		}
	} while (!elrec && (fprints >= 1));
	if (elrec) {
		LSUBID(elrec, ELCT_MEM, ELMETYP_CRD, ELMCNTR_DEC4000,
		       0/* num */, 0/* unitnum */, 0/* errcode */);
		hdr = (struct el_edc_header *)&elrec->el_body;
		hdr->emb_revision = EMB_CURRENT_REVISION;
		hdr->emb_edc_flags = crd_flags | kn430_lost_edc_packet;
		kn430_lost_edc_packet = 0;
		hdr->emb_log_reason = rsn;
		hdr->emb_part.emb_total = 1;
		hdr->emb_part.emb_this = 1;
		hdr->emb_hwrpb_badpgs = EMB_HWRPB_BADPGS_UNKNOWN;
		hdr->emb_memdsc_size = (sizeof(struct kn430_memdsc) 
					* boards);
		hdr->emb_memdsc_offset = sizeof(struct el_edc_header);
		hdr->emb_num_fprints = fprints;
		hdr->emb_fprint_size = sizeof(struct kn430_edc_fprint);
		hdr->emb_fprint_offset = (hdr->emb_memdsc_size +
					  hdr->emb_memdsc_offset);
		
		/*
		 * Set up memdsc entries.
		 *
		 * The information int he memdsc entries is redundant, since
		 * the same information is logged in the binary FRU data packet
		 * at system startup.  Use the slot number to correlate the
		 * footprint information to the FRU information.
		 *
		 * We would have to parse the FRU table to fill in this
		 * descriptor -- not worth the bother if this is the only
		 * use.
		 */
		md = ((struct kn430_memdsc *)
		      ((long)hdr + hdr->emb_memdsc_offset));
		for(i = 0; i < CBUS_MEM_SLOTS; i++) {
			if (CBUS_MEM_ALIVE(i)) {
				md->kn430_md_node.kn430_mdn_byte_count = 0;
				md->kn430_md_node.kn430_mdn_selftest = 
					kn430_meminfo[i].cfru_ehdr.cfru_test;
				md->kn430_md_node.kn430_mdn_slot = i;
				md->kn430_md_node.kn430_mdn_res03 = 0;
				strncpy(md->kn430_md_sernum,
					kn430_meminfo[i].cfru_serial_number,
					sizeof(md->kn430_md_sernum));
				md->kn430_md_res0e = 0;
				strncpy(md->kn430_md_partnum,
					kn430_meminfo[i].cfru_part_number,
					sizeof(md->kn430_md_partnum));
				md->kn430_md_res19 = 0;
				md->kn430_md_hw_rev = 
					kn430_meminfo[i].cfru_hw_rev;
				md->kn430_md_res1c[0] = 0;
				md->kn430_md_res1c[1] = 0;
				md->kn430_md_res1c[2] = 0;
				md->kn430_md_config = 
					kn430_meminfo[i].cfru_mem_config;
				md++;
			}
		}
	}
	else {
		kn430_lost_edc_packet |= EMB_FLG_LOST_INFO;
	}
	*p_elrec = elrec;
	return fprints;
}

/*
 * Return a timestamp for a CRD footprint.
 */
static long kn430_edc_timestamp()
{
	return time.tv_sec;
}


static long kn430_mcmd
	(register struct el_cobra_frame_memory *elcm,
	 register int high_cmic)
{
	register long mcmd;

	mcmd = elcm->elcm_data.elcm_mcmd1;
	return mcmd;
}

static long kn430_pa
	(register struct el_cobra_frame_memory *elcm,
	 register int high_cmic)
{
	register long pa;

	pa = MCMD_PA(kn430_mcmd(elcm, high_cmic));
	return pa;
}

static long kn430_footprint_signature
	(register struct el_cobra_frame_memory *elcm,
	 register int high_cmic)
{
	register long mcmd;
	register long signature;
	register int cshft;		  /* CMIC shift: 0(Low) or 32(High) */
	register int slot;		  /* Memory slot (0..3).	    */
	register u_long mask;

	mcmd = kn430_mcmd(elcm, high_cmic);

	/* 
	 * Generate signatore for this error footprint.
	 *
	 * The signatrue is made up of several fields:
	 *	MFP_SLOT	Slot this memory module is in.
	 *	MFP_HIGH_CMIC	High or Low CMIC.
	 *	MFP_BANK	Bank the error was in.
	 *	MFP_SYNDROME	Syndrome for the error.
	 *
	 * The bank logic looks at the configuration register to see if two or
	 * four banks are present.  If four banks are present, physical address
	 * bits 18 and 19 indicate the bank.  If two banks are present, 
	 * physical address bit 18 indicates whether bank 0 or 1 is in use.
	 *
	 */

	cshft = (high_cmic ? (EDCSTAT2_SYNH_SHIFT - EDCSTAT2_SYNL_SHIFT) : 0);
	signature = ((elcm->elcm_data.elcm_medc2 & (EDCSTAT2_SYNL << cshft))
		     >> (EDCSTAT2_SYNL_SHIFT + cshft)) << MFP_SYNDROME_SHIFT;

	if (high_cmic) signature |= MFP_HIGH_CMIC;

	/*
	 * Depending on how many banks there are, either 1 or two PA bits
	 * are used for bank select.
	 */
	mask = ((elcm->elcm_data.elcm_mconf & (MCONF_BANKSL << cshft)) ?
		0x3UL : 0x1UL);
	signature |= ((((mcmd & MCMD_BANK)
			>> MCMD_BANK_SHIFT) & mask) << MFP_BANK_SHIFT);

	slot = (elcm->elcm_data.elcm_mconf & MCONF_SLOTL) >> MCONF_SLOTL_SHIFT;
       	signature |= slot << MFP_SLOT_SHIFT;

	return signature;
}

/*
 * kn430_edc_create_thread
 *
 * This routine serves as a calling point, through kd_thread_start, which
 * guarantees execution will not occur before threads are available.
 * 
 * Called from init_main after threads are initialized.
 */

static void kn430_edc_create_thread(caddr_t dummy)
{
	extern task_t first_task;

	(void) kernel_thread(first_task, kn430_edc_thread);
}


long kn430_edc_thread_loops = 0;
long kn430_edc_thread_enabled = 1;

void kn430_edc_thread()
{ 
	int fprints_rmng;
	static int backoff = 4;

	for (; kn430_edc_thread_enabled;) {

		kn430_edc_thread_loops++;

		fprints_rmng = kn430_checkpoint_footprints();

		/* 
		 * If all the footprints were checkpointed, wait until the
		 * next checkpoint interval.  If there wasn't enough room in
		 * the error log buffer for all of the footprints, wait
		 * one second with exponential backup.
		 */
		assert_wait((vm_offset_t)kn430_edc_thread, FALSE);
		if (fprints_rmng) {
			thread_set_timeout(backoff * hz);
			backoff <<= 1;
			if (backoff > kn430_edc_checkpoint_interval) {
				backoff = kn430_edc_checkpoint_interval;
			}
		}
		else {
			backoff = 1;
			thread_set_timeout(kn430_edc_checkpoint_interval * hz);
		}
		thread_block();
	}
}

/*****************************************************************************
 * KN430 ERROR LOG ROUTINES						     *
 *									     *
 * These routines manage and populate the error log scratch buffer.	     *
 *****************************************************************************/

/*
 * Copy the Field Replaceable Unit (FRU) table from the HWRPB into the binary
 * error log.  Don't bother to complain if it isn't there.
 */
static void kn430_copy_fru_table()
{
	extern struct rpb *rpb;			 /* Pointer to HWRPB */
	register u_long cpuid = kn430_cpuid();	 /* Index of this CPU. */
 	register int pkt_size, fru_size, fru_entry_size;
	register struct el_cobra_fru_header *cfru; /* HWRPB FRU table. */
	register struct el_cobra_frame_fru *fru; /* Error log FRU frame. */
	register struct kn430_scratch_buffer *sb;
	register struct el_cobra_mcheck_event_header *elch;
	register struct el_cobra_frame_header *eof;
	register struct el_rec *elrec;
	register struct el_cobra_fru_entry_header *fe;
	register int rmng;
	register long ptr;

	/* Quietly return if no FRU table present. */
	if (rpb->rpb_fru_off == 0L) return;

	/* Find the FRU table. */
	cfru = ((struct el_cobra_fru_header *)
		((long)rpb + (long)rpb->rpb_fru_off));


	/* Parse the FRU table. */
	rmng = cfru->cfru_size - sizeof(struct el_cobra_fru_header);
	ptr = (long)cfru + sizeof(struct el_cobra_fru_header);
	do {
		fe = (struct el_cobra_fru_entry_header *)ptr;
		if ((fe->cfru_id >= CFRUID_MEM0) 
		    && (fe->cfru_id <= CFRUID_MEM3)) {
			kn430_meminfo[fe->cfru_id - CFRUID_MEM0] =
				*((struct el_cobra_fru_mem *)fe);
		}
		else if ((fe->cfru_id == CFRUID_END)
			 && (fe->cfru_fid == CFRU_FRAME_END)) {
			rmng -= sizeof(long);
			break;
		}
		ptr += fe->cfru_count * sizeof(long);
		rmng -= fe->cfru_count * sizeof(long);
	} while (rmng > 0);


	/* Adjust the size. */
	fru_size = cfru->cfru_size - rmng;
	fru_entry_size = fru_size + sizeof(struct el_cobra_frame_header);

	/* Try to allocate the error log packet. */
	pkt_size = (sizeof(struct el_cobra_mcheck_event_header)
		    + fru_entry_size
		    + sizeof(struct el_cobra_frame_header));
	elrec = (struct el_rec *)binlog_alloc(pkt_size, COBRA_SEV_INFO);

	/* Return silently if a buffer could not be allocated. */
	if (elrec == (struct el_rec *)BINLOG_NOBUFAVAIL) return;


	/* Fill in binary error log header. */
	LSUBID(elrec, ELCT_STATE, ELST_CONFIG, ELMACH_COBRA, cpuid, 0, 0);


	/* Fill in machine check event header. */
	elch = (struct el_cobra_mcheck_event_header *)&elrec->el_body;
	kn430_init_mcheck_event_header(elch, pkt_size, COBRA_SEV_INFO, cpuid,
				       0);

	/* Fill in FRU entry. */
	fru = ((struct el_cobra_frame_fru *)
	       ((long)elch + sizeof(struct el_cobra_mcheck_event_header)));
	fru->elcu_hdr.elcf_size = fru_size;
	fru->elcu_hdr.elcf_fid = COBRA_FID_FRU;
	bcopy(cfru, &fru->elcu_data, fru_size);
	fru->elcu_data.cfru_header.cfru_size = fru_size;

	/* Fill in end-of-frame marker. */
	eof = (struct el_cobra_frame_header *)((long)fru + fru_entry_size);
	eof->elcf_size = 0;
	eof->elcf_fid = COBRA_FID_END_FRAME;

	/* Validate the completed record. */
	binlog_valid((char *)elrec);
}

/*
 * cobra_alloc_elbuf
 *
 * Allocate space in the machine-specific error logging scratch buffer.  The
 * machine check parser allocates space for packets in this buffer.  The
 * scratch buffer is used to sequentially build up a complete error log
 * packet, before doing an ealloc and sending the packet.
 *
 * This is needed because there is no way to "shrink" a packet once it has
 * been ealloced.
 */
static vm_offset_t kn430_alloc_elbuf
	(register struct kn430_scratch_buffer *sb,
	 register long size)
{
	register vm_offset_t ptr;
	register long rmng;

	/*
	 * If there is enough space left in the buffer, allocate the frame.
	 * If not, barf.
	 */
	rmng = sb->sb_rmng - size;
	if (rmng < 0) {
		ptr = 0;
	}
	else {
		ptr = sb->sb_ptr;
		sb->sb_ptr += size;
		sb->sb_rmng = rmng;
	}

	return ptr;
}

/*
 * kn430_alloc_elbuf_frame
 *
 * Allocate a data frame in the machine-specific error logging scratch buffer.
 * Fill in the frame header and return the pointer to the beginning of the
 * frame body.
 */
static void *kn430_alloc_elbuf_frame
	(register struct kn430_scratch_buffer *sb,
	 register long body_size,
	 register long fid)
{
	register struct el_cobra_frame_header *elch;
	register long size;

	/*
	 * Compare the structure size to the logout size, and choose the
	 * larger of the two.  Add the size of the header.
	 */
	size = body_size + sizeof(struct el_cobra_frame_header);

	/*
	 * Allocate space for the frame and header.
	 */
        elch = (struct el_cobra_frame_header *)kn430_alloc_elbuf(sb, size);

	/*
	 * Fill in the header.
	 */
	if (elch != (struct el_cobra_frame_header *)0) {
		elch->elcf_size = body_size;
		elch->elcf_fid = fid;
	}
	else {
		body_size = -1;
	}

	/*
	 * Return the pointer to the frame.
	 */
	return (void *)elch;
}

/*
 * kn430_copy_to_errlog.
 *
 * Terminate the packet which was being built, and pass it off to the error
 * logger.
 */
static struct el_rec *kn430_copy_to_errlog
	(register struct kn430_scratch_buffer *sb)
{
	register long size, sev, type, class, ctldev, num, unitnum, errcode;
	register struct el_cobra_mcheck_event_header *elch;
	register struct el_rec *elrec;

	/* 
	 * The packet header has information which needs to be copied
	 * into the error logger's header.
	 */
	elch = (struct el_cobra_mcheck_event_header *)sb->sb_start;

	/*
	 * Put the terminator on the end of the packet.  Continue even if the
	 * terminator won't fit; hope UERF handles this gracefully.
	 */
	kn430_alloc_elbuf_frame(sb, 0, COBRA_FID_END_FRAME);

	size = sb->sb_ptr - sb->sb_start;
	elch->elch_size = size;

	/* 
	 * Allocate the event logger buffer.  Give up silently if no room.
	 */
	sev = elch->elch_sev;
	elrec = (struct el_rec *)binlog_alloc(size, sev);
	if (elrec != (struct el_rec *)BINLOG_NOBUFAVAIL) {

		/*
		 * Get some of the error logger's header info from the frame 
		 * header.  Vector determines the format of the frame.  The
		 * format must be uniquely identified by the tuple
		 * (class, type, ctldev).
		 */
		class = ELCT_EXPTFLT;
		switch (elch->elch_vector) {
		case KN430_VECTOR_PROC_CORR:
			type = ELEXTYP_PROC_CORR;
			break;
		case KN430_VECTOR_SYS_FATAL:
			type = ELEXTYP_HARD;
			break;
		case KN430_VECTOR_FRU:
			class = ELCT_STATE;
			type = ELST_CONFIG;
			break;
		case KN430_VECTOR_PROC_FATAL:
		default:
			type = ELEXTYP_MCK;
			break;
		}
		ctldev = ELMACH_COBRA;
		unitnum = elch->elch_fru1;
		num = elch->elch_cpuid;
		errcode = elch->elch_fcode;

		/* 
		 * Fill in the event logger's header.
		 */
		LSUBID(elrec, class, type, ctldev, num, unitnum, errcode);

		/* 
		 * Copy the frame in.
		 */
		bcopy(elch, &elrec->el_body, size);
	}
	return elrec;
}

/*
 * Initialize the machine check event header.
 */
static void kn430_init_mcheck_event_header(elch, size, severity, cpuid, vector)
	register struct el_cobra_mcheck_event_header *elch;
	register int size;
	register int severity;
	register int cpuid;
	register long vector;
{
	elch->elch_size = size;
	elch->elch_rev = COBRA_ERROR_FRAME_REV_INITIAL;
	elch->elch_vector = vector;
	elch->elch_fru1 = COBRA_FRU_INVALID;
	elch->elch_fru2 = COBRA_FRU_INVALID;
	elch->elch_res0a = 0;
	elch->elch_sev = severity;
	elch->elch_cpuid = cpuid;
	elch->elch_count = 1;
	elch->elch_thresh = 0;
	elch->elch_fcode = 0;
	elch->elch_res16 = 0;
	elch->elch_flags[0] = 0;
	elch->elch_flags[1] = 0;
	elch->elch_res28 = 0;
}

/*
 * Start building the error frame.  Allocate and initialize the Cobra error
 * log header right away.
 */
static struct kn430_scratch_buffer *kn430_errlog_start
	(register struct kn430_mcheck_control *mctl,
	 register int cpuid,
	 register long vector)
{
	register struct el_cobra_mcheck_event_header *elch;
	register struct kn430_scratch_buffer *sb;

	/*
	 * Initialize the scratch buffer as empty.  If processing a
	 * processor fatal error, use a separate buffer, since those
	 * errors aren't masked even at SPLHIGH.
	 */
	sb = KN430_SCRATCH_BUFFER(cpuid, vector);
	sb->sb_ptr = sb->sb_start;
	sb->sb_rmng = sb->sb_len;

	/*
	 * Allocate the cobra error log header and give it some default values.
	 * Since the buffer was just initialized above, this first allocation
	 * will not fail -- no need to test for null pointers.
	 */
	elch = (struct el_cobra_mcheck_event_header *)
		kn430_alloc_elbuf(sb, 
				  sizeof(struct el_cobra_mcheck_event_header));

	kn430_init_mcheck_event_header(elch, 0, COBRA_SEV_FATAL, cpuid, 
				       vector);

	/*
	 * Initialize the snapshot frame pointers.
	 */
	cobra_init_frames(mctl);

	/*
	 * Set the pointer to the header.
	 */
	mctl->mctl_frame.mcfr_hdr = elch;

	return sb;
}

/*
 * kn430_free_logout
 *
 * Free the logout frame so that PAL can record another error.
 */
static void kn430_free_logout(register u_long mask)
{
	mtpr_mces(mask);
}

/*****************************************************************************
 * KN430 CONSOLE OUTPUT ROUTINES					     *
 *****************************************************************************/

static int kn430_summarize_errors(register struct kn430_mcheck_control *mctl)
{
	register long summarized;
	register int cpuid, memid;
	register struct el_cobra_mcheck_event_header *elch;
	register int temp;

	summarized = 0;
	elch = mctl->mctl_frame.mcfr_hdr;

	/*
	 * Look for cpu-specific errors.
	 */
	for(cpuid = 0; cpuid < CBUS_MAX_CPU_SLOTS; cpuid++) {
		register long offset;
		register struct el_cobra_cpu_csrs *elcsr;
		register struct el_ev4_csrs *el4;

		/*
		 * Offset for looking at other bits.
		 */
		offset = cpuid * COBRA_CPU_SHIFT;

		/*
		 * Pointers to CPU board CSRs.
		 */
		if (cpuid == elch->elch_cpuid) {
			elcsr = ((struct el_cobra_cpu_csrs *)
				 (&mctl->mctl_frame.mcfr_mcheck->
				  elcmc_data.elcmc_bcc));
			el4 = ((struct el_ev4_csrs *)
				 (&mctl->mctl_frame.mcfr_mcheck->
				  elcmc_data.elcmc_exc_addr));
		}
		else {
			elcsr = ((struct el_cobra_cpu_csrs *)
				 (&mctl->mctl_frame.mcfr_ocpu->
				  elco_data.elco_bcc));
			el4 = ((struct el_ev4_csrs *)0);
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_C3_CA_NOACK + offset)) {
			printf("CPU%d Bus Command No-Ack "
			       "(CBEAL:%l016x, CBEAH:%l016x)\n", cpuid,
			       elcsr->elcsr_cbeal, elcsr->elcsr_cbeah);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_C3_WD_NOACK + offset)) {
			printf("CPU%d Bus Write Data No-ack "
			       "(CBEAL:%l016x, CBEAH:%l016x)\n",
			       cpuid, elcsr->elcsr_cbeal, 
			       elcsr->elcsr_cbeah);
			summarized += 1;
		}
		
		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_C3_RD_PAR + offset)) {
			printf(((elcsr->elcsr_cbe & CBE_RDPE) ?
				"CPU%d Bus Read Parity Error "
				"(CBEAL:%l016x, CBEAH:%l016x)\n" :
				"CPU%d Bus Read Parity Error\n"),
			       cpuid, elcsr->elcsr_cbeal, 
			       elcsr->elcsr_cbeah);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_EV_C_UNCORR + offset)) {
			printf("CPU%d B-Cache Uncorrectable Error "
			       "(BIU_STAT:%l016x, FILL_ADDR:%l016x)\n",
			       cpuid, el4->el4_biu_stat, 
			       el4->el4_fill_addr);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_EV_TC_PAR + offset)) {
			printf("CPU%d Cache Tag Control Parity Error "
			       "(BIU_STAT:%l016x, BIU_ADDR:%l016x)\n",
			       cpuid, el4->el4_biu_stat, 
			       el4->el4_biu_addr);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_EV_T_PAR + offset)) {
			printf("CPU%d Cache Tag Parity Error "
			       "(BIU_STAT:%l016x, BIU_ADDR:%l016x)\n",
			       cpuid, el4->el4_biu_stat,
			       el4->el4_biu_addr);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0_C3_EV + offset)) {
			printf("CPU%d to C3 Data Error "
			       "(BCUE:%l016x, BCUEA:%l016x)\n", cpuid,
			       elcsr->elcsr_bcue, elcsr->elcsr_bcuea);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0I_C3_C_UNCORR + offset)) {
			printf(((elcsr->elcsr_bcue & BCUE_UE) ?
				"CPU%d Cache Uncorrectable Error"
				"(C3 Detected)\n"
				"(BCUE:%l016x, BCUEA:%l016x)\n" :
				"CPU%d Cache Uncorrectable Error\n"
				"(C3 Detected)\n"),
			       cpuid, elcsr->elcsr_bcue, 
			       elcsr->elcsr_bcuea);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0I_C3_TC_PAR + offset)) {
			printf("CPU %d Cache Tag Control Parity Error (2)\n",
			       cpuid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0I_C3_T_PAR + offset)) {
			printf("CPU %d Cache Tag Parity Error (2)\n",
			       cpuid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0I_C3_C_CORR + offset)) {
			printf("CPU %d Cache Correctable (C3 Detected)\n",
			       cpuid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_CPU0I_EV_C_CORR + offset)) {
			printf("CPU %d Cache Correctable (CPU Detected)\n",
			       cpuid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_C3_0_CA_PAR + cpuid)) {
			printf("CPU %ld Bus Command/Address Parity Error\n",
			       cpuid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_C3_0_WD_PAR + cpuid)) {
			printf("CPU %d Bus Write Data Parity Error\n",
			       cpuid);
			summarized += 1;
		}
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_CPU_EV_SYN_1F)) {
		printf("CPU %d reported syndrome 0x1F\n",
		       elch->elch_cpuid);
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_CPU_C3_SYN_1F)) {
		printf("C3 reported Syndrome 0x1F\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_CPU_DT_PAR)) {
		printf("Duplicate Tag Store Parity Error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_CPU_EV_HARD_ERROR)) {
		printf("CPU %d cycle aborted with Hard Error\n",
		       elch->elch_cpuid);
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_EVC_C3_MEM_R_ERROR)) {
		printf("CPU %d error caused by memory\n", elch->elch_cpuid);
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_EVC_IO_MEM_R_ERROR)) {
		printf("I/O error caused by memory\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
				 COBRA_EVC_C3_OCPU_ADD_MATCH)) {
		printf("CPU %d error caused by other CPU\n",
		       elch->elch_cpuid);
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_EVC_MIXED_ERRORS)) {
		printf("Mixed errors (no correlation)\n");
		summarized += 2; /* Forces treatment as mixed error */
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_EXT_CA_NOACK)) {
		printf("I/O detected bus command/address No-Ack\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_EXT_WD_NOACK)) {
		printf("I/O detected bus write data No-Ack\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_EXT_RD_PAR)) {
		printf("I/O detected bus read parity error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_EXT_CB_UNCORR)) {
		printf("Data delivered to I/O is corrupted\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_INT_LB_DMA_PAR)) {
		printf("I/O -> L-Bus DMA Parity Error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_INT_FB_DMA_PAR)) {
		printf("I/O - F-Bus DMA Parity Error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_INT_FB_MB_PAR)) {
		printf("I/O - F-Bus Mail Box Access Parity Errror\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_INT_BUSSYNC)) {
		printf("I/O Chip - System Bus Synchronization Error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_INT_SCSSTALL)) {
		printf("I/O Chip Synchronization Error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_CA_PAR)) {
		printf("I/O Bus Command/Address Parity Error\n");
		summarized += 1;
	}

	if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,COBRA_IO_WD_PAR)) {
		printf("I/O Write Data Parity Error\n");
		summarized += 1;
	}


	/*
	 * Look for memory-specific errors.
	 */
	for(memid = 0; memid < CBUS_MEM_SLOTS; memid++) {
		struct el_cobra_frame_memory *elcm;

		elcm = ((struct el_cobra_frame_memory *)
			mctl->mctl_frame.mcfr_mem[memid]);

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_MEM0_CA_PAR + memid)) {
			printf("Memory %ld Bus Command/Address Parity Error\n",
			       memid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_MEM0_WD_PAR + memid)) {
			printf("Memory %ld Write Data Parity Error\n", memid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_MEM0_UNCORR + memid)) {
			printf("Memory %ld Uncorrectable Error\n", memid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_MEM0_CORR + memid)) {
			printf("Memory %ld Correctable Error\n", memid);
			summarized += 1;
		}

		if (COBRA_ERROR_FLAG_VAL(elch->elch_flags,
					 COBRA_MEM0_SYNC_ERROR + memid)) {
			printf("Memory %d Synchronization Error\n", memid);
			summarized += 1;
		}
	}

	/*
	 * Return 1 if successfully summarized, and 0 if not.  Summary is
	 * considered unsuccessful if more than one event was identified.
	 * Remove this requirement by just returning summarized, once the
	 * summary messages are deemed to have enough detail.
	 */
	return (summarized >= 1);
}

static void kn430_dump_errlog_packet
	(register struct kn430_mcheck_control *mctl,
	 register struct el_rec *elrec)
{
	register struct el_cobra_mcheck_event_header *elch;
	register struct el_cobra_frame_header *elcf;
	register long rmng;
	register int cpuid;

	elch = mctl->mctl_frame.mcfr_hdr;
	cpuid = elch->elch_cpuid;

	/*
	 * Dump the OS error logging stuff.
	 */
	if (elrec) {
		rmng = kn430_dump_el_hdr(elrec);
		elch = (struct el_cobra_mcheck_event_header *)&elrec->el_body;
	}
	else {
		rmng = elch->elch_size;
	}

	/*
	 * Dump the Cobra Header.
	 */
	kn430_dump_cobra_hdr(elch);
	elcf = (struct el_cobra_frame_header *)(elch + 1);
	rmng -= sizeof(struct el_cobra_mcheck_event_header);

	while ((rmng > 0) && (elcf->elcf_fid || elcf->elcf_size)) {
		switch (elcf->elcf_fid) {
		case COBRA_FID_IO:
			kn430_dump_io((struct el_cobra_frame_io *)elcf);
			break;
		case COBRA_FID_MCHECK:
			kn430_dump_mcheck((struct el_cobra_frame_mcheck *)elcf,
					  cpuid);
			break;
		case COBRA_FID_P_BC_CORR:
			kn430_dump_proc_bcache_corr
				(((struct el_cobra_frame_proc_bcache_corr *)
				  elcf),
				 cpuid);
			break;
		case COBRA_FID_OCPU:
			kn430_dump_ocpu
				((struct el_cobra_frame_other_cpu *)elcf);
			break;
		case COBRA_FID_MEMORY:
			kn430_dump_memory
				((struct el_cobra_frame_memory *)elcf);
       			break;
		default:
			kn430_dump_unidentified(elcf);
			break;
		}
		rmng -= (sizeof(struct el_cobra_frame_header)
			 + elcf->elcf_size);
		elcf = ((struct el_cobra_frame_header *)
			((long)elcf + sizeof(struct el_cobra_frame_header)
			 + elcf->elcf_size));
       	}
}

/*
 * Simple routine to convert an integer to a null-terminated hex string.
 * No bound checking is performed.
 */
static long cobra_cvt_int_to_hexstr
	(register char *buf,
	 register int digits,
	 register long value)
{
	register char *ptr;
	static char hex_table[] = 
		{'0','1','2','3','4','5','6','7',
		 '8','9','a','b','c','d','e','f'};

	ptr = buf + digits;
	*(ptr--) = '\0';
	do {
		*(ptr--) = hex_table[value & 0x0f];
		value = value >> 4;
	} while (ptr >= buf);
}

static long kn430_dump_el_hdr(register struct el_rec *elrec)
{
	static char *pri_names[] = 
		{"SEVERE","0x02","HIGH","0x04","LOW"};
					   
	register char *pristr, pribuf[8];
	register long pri;
	pri = elrec->elrhdr.rhdr_pri;
	if (pri >= 0 && pri < sizeof(pri_names) / sizeof(pri_names[0])) {
		pristr = pri_names[pri];
	}
	else {
		pribuf[0] = '0';
		pribuf[1] = 'x';
		cobra_cvt_int_to_hexstr(&pribuf[2], 2, pri);
		pristr = pribuf;
	}
	printf("\nError Log Header:\n"
	       "  RECLEN:%04X SEQNUM:%04X TIME:%08X PRI:%s\n"
	       "  SID:%08X SYSTYPE:%l016X MPERR:%08X MPNUM:%08X\n",
	       elrec->elrhdr.rhdr_reclen, elrec->elrhdr.rhdr_seqnum,
	       elrec->elrhdr.rhdr_time, pristr,
	       elrec->elrhdr.rhdr_sid, elrec->elrhdr.rhdr_systype,
	       elrec->elrhdr.rhdr_mperr, elrec->elrhdr.rhdr_mpnum);

	return elrec->elrhdr.rhdr_reclen;
}

static void kn430_dump_cobra_hdr
	(register struct el_cobra_mcheck_event_header *elch)
{
	register long sev;
	register int fru1, fru2;
	register char *sevstr, *fru1str, *fru2str;
	char sevbuf[8], fru1buf[8], fru2buf[8];
	static char *fru_table[] = 
		{"unknown", "I/O", "CPU 1", "CPU 2", "MEM 0",
		 "MEM 1", "MEM 2", "MEM 3"};
	static char *sev_table[] =
		{"invalid", "FATAL", "ALARM", "WARNING", "INFO"};


	sev = elch->elch_sev;
	if (sev >= 0 && sev < (sizeof(sev_table) / sizeof(sev_table[0]))) {
		sevstr = sev_table[sev];
	}
	else {
		sevbuf[0] = '0';
		sevbuf[1] = 'x';
		cobra_cvt_int_to_hexstr(&sevbuf[2], 4, sev);
		sevstr = sevbuf;
	}

	fru1 = elch->elch_fru1;
	if (fru1 >= 0 && fru1 < (sizeof(fru_table) / sizeof(fru_table[0]))) {
		fru1str = fru_table[fru1];
	}
	else {
		fru1buf[0] = '0';
		fru1buf[1] = 'x';
		cobra_cvt_int_to_hexstr(&fru1buf[2], 4, fru1);
		fru1str= fru1buf;
	}

	fru2 = elch->elch_fru2;
	if (fru2 >= 0 && fru2 < (sizeof(fru_table) / sizeof(fru_table[0]))) {
		fru2str = fru_table[fru2];
	}
	else {
		fru2buf[0] = '0';
		fru2buf[1] = 'x';
		cobra_cvt_int_to_hexstr(&fru2buf[2], 4, fru2);
		fru2str= fru2buf;
	}


	printf("\nCobra-Specific Header (size:%08X, revision:%04X):\n"
	       "  VECTOR:%04X  CPUID:%04X  SEVERITY:%s"
	       "  COUNT:%04X  THRESHOLD:%04X\n"
	       "  FRU1: (%s)  FRU2: (%s)\n"
	       "  FAILURE CODE:%04X (%s)\n"
	       "  FLAGS:%l016X %l016X  MISC:%04X %04X %l016X\n",
	       elch->elch_size, elch->elch_rev,
	       elch->elch_vector, elch->elch_cpuid, sevstr,
	       elch->elch_count, elch->elch_thresh,
	       fru1str, fru2str,
	       elch->elch_fcode, "",
	       elch->elch_flags[1], elch->elch_flags[0],
	       elch->elch_res0a, elch->elch_res16, elch->elch_res28);

}

static void kn430_dump_io(register struct el_cobra_frame_io *elci)
{
	register struct el_cobra_data_io *io;

	io = &elci->elci_data;
	printf("\nI/O CSRS (size 0x%lx):\n"
	       "    IOCSR:%l016X    LMBPR:%l016X    FERR2:%l016X\n"
	       "    CERR1:%l016X    FMBPR:%l016X     LINT:%l016X\n"
	       "    CERR2:%l016X  DIAGCSR:%l016X    LERR1:%l016X\n"
	       "    CERR3:%l016X    FERR1:%l016X    LERR2:%l016X\n",
	       elci->elci_hdr.elcf_size, 
	       io->elci_iocsr, io->elci_lmbpr, io->elci_ferr2,
	       io->elci_cerr1, io->elci_fmbpr, io->elci_lint,
	       io->elci_cerr2, io->elci_diagcsr, io->elci_lerr1,
	       io->elci_cerr3, io->elci_ferr1, io->elci_lerr2);
}

static void kn430_dump_mcheck
	(register struct el_cobra_frame_mcheck *elcmc,
	 register int cpuid)
{
	register struct el_cobra_data_mcheck *mc;

	mc = &elcmc->elcmc_data;
	printf("\nProcessor %d CSRs:\n"
	       " EXC_ADDR:%l016X FILL_SYN:%l016X  ICCSR:%l016X\n" 
	       " EXC_SUM :%l016X FILL_ADR:%l016X   HIER:%l016X\n" 
	       " EXC_MASK:%l016X  DC_STAT:%l016X   HIRR:%l016X\n" 
	       " BIU_STAT:%l016X  DC_ADDR:%l016X MM_CSR:%l016X\n" 
	       " BIU_ADDR:%l016X ABOX_CTL:%l016X BC_TAG:%l016X\n" 
	       " BIU_CTL :%l016X PAL_BASE:%l016X     VA:%l016X\n",
	       cpuid,
	       mc->elcmc_exc_addr, mc->elcmc_fill_syndrome, mc->elcmc_iccsr,
	       mc->elcmc_exc_sum, mc->elcmc_fill_addr, mc->elcmc_hier,
	       mc->elcmc_exc_mask, mc->elcmc_dc_stat, mc->elcmc_hirr,
	       mc->elcmc_biu_stat, mc->elcmc_dc_addr, mc->elcmc_mm_csr,
	       mc->elcmc_biu_addr, mc->elcmc_abox_ctl, mc->elcmc_bc_tag,
	       mc->elcmc_biu_ctl, mc->elcmc_pal_base, mc->elcmc_va);

	printf("\nCPU Module %d CSRs:\n"
	       "    BCC  :%l016X     DTER:%l016X     PMBX:%l016X\n"
	       "    BCCE :%l016X    CBCTL:%l016X     IPIR:%l016X\n"
	       "    BCCEA:%l016X    CBE  :%l016X      SIC:%l016X\n"
	       "    BCUE :%l016X    CBEAL:%l016X     ADLK:%l016X\n"
	       "    BCUEA:%l016X    CBEAH:%l016X    MADRL:%l016X\n",
	       cpuid,
	       mc->elcmc_bcc, mc->elcmc_dter, mc->elcmc_pmbx,
	       mc->elcmc_bcce, mc->elcmc_cbctl, mc->elcmc_ipir,
	       mc->elcmc_bccea, mc->elcmc_cbe, mc->elcmc_sic,
	       mc->elcmc_bcue, mc->elcmc_cbeal, mc->elcmc_adlk,
	       mc->elcmc_bcuea, mc->elcmc_cbeah, mc->elcmc_madrl);
	       
	printf("\nPAL Temporary Registers "
	       "(error code = %d, frame revision = %d):\n"
	       " 0:<unavailable>     8:%l016X 16:%l016X 24:%l016X\n"
	       " 1:%l016X  9:%l016X 17:%l016X 25:%l016X\n"
	       " 2:%l016X 10:%l016X 18:%l016X 26:%l016X\n"
	       " 3:%l016X 11:%l016X 19:%l016X 27:%l016X\n"
	       " 4:%l016X 12:%l016X 20:%l016X 28:%l016X\n"
	       " 5:%l016X 13:%l016X 21:%l016X 29:%l016X\n"
	       " 6:%l016X 14:%l016X 22:%l016X 30:%l016X\n"
	       " 7:%l016X 15:%l016X 23:%l016X 31:%l016X\n",
	       mc->elcmc_lhdr.elcl_error_type,
	       mc->elcmc_lhdr.elcl_frame_rev,
	       mc->elcmc_paltemp[8], 
	       mc->elcmc_paltemp[16], mc->elcmc_paltemp[24],
	       mc->elcmc_paltemp[1], mc->elcmc_paltemp[9], 
	       mc->elcmc_paltemp[17], mc->elcmc_paltemp[25],
	       mc->elcmc_paltemp[2], mc->elcmc_paltemp[10], 
	       mc->elcmc_paltemp[18], mc->elcmc_paltemp[26],
	       mc->elcmc_paltemp[3], mc->elcmc_paltemp[11], 
	       mc->elcmc_paltemp[19], mc->elcmc_paltemp[27],
	       mc->elcmc_paltemp[4], mc->elcmc_paltemp[12], 
	       mc->elcmc_paltemp[20], mc->elcmc_paltemp[28],
	       mc->elcmc_paltemp[5], mc->elcmc_paltemp[13], 
	       mc->elcmc_paltemp[21], mc->elcmc_paltemp[29],
	       mc->elcmc_paltemp[6], mc->elcmc_paltemp[14], 
	       mc->elcmc_paltemp[22], mc->elcmc_paltemp[30],
	       mc->elcmc_paltemp[7], mc->elcmc_paltemp[15], 
	       mc->elcmc_paltemp[23], mc->elcmc_paltemp[31]);

	kn430_dump_tail((long *)(mc + 1),
			sizeof(struct el_cobra_data_mcheck),
			((long)elcmc->elcmc_hdr.elcf_size - 
			 sizeof(struct el_cobra_data_mcheck)));
}

static void kn430_dump_proc_bcache_corr
	(register struct el_cobra_frame_proc_bcache_corr *elcpb,
	 register int cpuid)
{
	register struct el_cobra_data_proc_bcache_corr *pb;

	pb = &elcpb->elcpb_data;
	printf("\nProcessor %d B-Cache Correctable Error Frame:\n"
	       "  BIU_CTL:%l016X BIU_STAT:%l016X BIU_ADDR:%l016X\n"
	       " FILL_SYN:%l016X FILL_ADR:%l016X   BC_TAG:%l016X\n",
	       cpuid,
	       pb->elcpb_biu_ctl, pb->elcpb_biu_stat, pb->elcpb_biu_addr,
	       pb->elcpb_fill_syndrome, pb->elcpb_fill_addr, pb->elcpb_bc_tag);

	kn430_dump_tail((long *)(pb + 1), 
			sizeof(struct el_cobra_data_proc_bcache_corr),
			((long)elcpb->elcpb_hdr.elcf_size - 
			 sizeof(struct el_cobra_data_proc_bcache_corr)));
}

static void kn430_dump_ocpu(register struct el_cobra_frame_other_cpu *elco)
{
	register struct el_cobra_data_other_cpu *ocpu;


	ocpu = &elco->elco_data;
	printf("\nCPU Module %d CSRs:\n"
	       "    BCC  :%l016X    DTER: %l016X     PMBX:%l016X\n"
	       "    BCCE :%l016X    CBCTL:%l016X     IPIR:%l016X\n"
	       "    BCCEA:%l016X    CBE  :%l016X      SIC:%l016X\n"
	       "    BCUE :%l016X    CBEAL:%l016X     ADLK:%l016X\n"
	       "    BCUEA:%l016X    CBEAH:%l016X    MADRL:%l016X\n",
	       ocpu->elco_cpuid,
	       ocpu->elco_bcc, ocpu->elco_dter, ocpu->elco_pmbx,
	       ocpu->elco_bcce, ocpu->elco_cbctl, ocpu->elco_ipir,
	       ocpu->elco_bccea, ocpu->elco_cbe, ocpu->elco_sic,
	       ocpu->elco_bcue, ocpu->elco_cbeal, ocpu->elco_adlk,
	       ocpu->elco_bcuea, ocpu->elco_cbeah, ocpu->elco_madrl);
}

static void kn430_dump_memory(register struct el_cobra_frame_memory *elcm)
{
	register struct el_cobra_data_memory *mem;
	mem = &elcm->elcm_data;

	printf("\nMemory Module %d CSRs:\n"
	       "    ERR-0:%l016X CONFIG-3:%l016X EDCCTL-6:%l016X\n"
	       "   CMD1-1:%l016X   EDC1-4:%l016X STRCTL-7:%l016X\n"
	       "   CMD2-2:%l016X   EDC2-5:%l016X REFCTL-8:%l016X\n"
	       " FILTER-9:%l016X\n",
	       mem->elcm_module, 
	       mem->elcm_merr, mem->elcm_mconf, mem->elcm_medcc,
	       mem->elcm_mcmd1, mem->elcm_medc1, mem->elcm_msctl,
	       mem->elcm_mcmd2, mem->elcm_medc2, mem->elcm_mref,
	       mem->elcm_filter);
}

static void kn430_dump_unidentified
	(register struct el_cobra_frame_header *elcf)
{
	printf("\nUnidentified data:\n");
	kn430_dump_tail((long *)(elcf + 1), 0L, (long)elcf->elcf_size);
}

/*
 * kn430_dump_tail
 *
 * Dump unidentified data at the end of a record.  This is needed if the PAL
 * returned more data than expected.  Dumped in column order.
 */
static void kn430_dump_tail
	(register long *start,
	 register long offset,
	 register long rmng)
{
	register long items, rows, bias;
	register long *p1, *p2, *p3, *end;
	static char *row_ctl[3] 
		= {"   <%l04X>:%l016X   <%l04X>:%l016X   <%l04x>:%l016X\n",
		   "   <%l04X>:%l016X\n",
		   "   <%l04X>:%l016X   <%l04X>:%l016X\n"};

	items = rmng / sizeof(long);
	rows = (items - 1) / 3 + 1;
	bias = offset - (long)start;
	p1 = start;
	p2 = p1 + rows;
	p3 = p2 + rows;

	/* Print full rows. */
	end = p2 - 1;
	while (p1 < end) {
		printf(row_ctl[0], 
		       (long)p1 + bias, *p1, 
		       (long)p2 + bias, *p2, 
		       (long)p3 + bias, *p3);
		p1++; p2++; p3++;
	}

	/* Print last row. */
	printf(row_ctl[items % 3], 
	       (long)p1 + bias, *p1, 
	       (long)p2 + bias, *p2, 
	       (long)p3 + bias, *p3);
}


/*
 * Print error packet to the console.
 * This is only done when we are about to panic on the error.
 */
int cobra_consprint()
{
	panic("cobra_consprint");
}

/*****************************************************************************
 * DEC400 LBUS MAILBOX COMMAND ROUTINES					     *
 *****************************************************************************
 * 
 * All lbus mailbox specific processing (i.e. device select) is done here.
 * The lbus has its own write bit and ignores the srm write bit <31>.
 */
void
ee_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_EE;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
uart_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_UART;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
siop_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	/* 
	 * scsi code tells mbox rtn that we're talking to script ram
	 * by setting bit 63, otherwise we're accessing siop (710)
	 */
	if(mbp->rbadr & (1L << 63)) {
		mbp->cmd = LBUS_SCSI_SCRPT;
		mbp->rbadr &= ~(1L << 63);
	}
	else
		mbp->cmd = LBUS_SCSI_CTLR;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
toy_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_TOY_CLK;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
tgec_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_TGEC;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
enet_addr_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_ENET_ADDR_ROM;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
srl_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_SRL_BUS;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

void
flash_mbox_cmd
	(register mbox_t mbp,       /* mailbox pointer */
	 register u_int rwflag,     /* rd/wrt flag */
	 register u_int wmask_as)   /* data type, wrtite mask and addr space */
{
	mbp->cmd = LBUS_FLASH;
	if(rwflag & WRT_CSR)
		mbp->cmd |= LBUS_WRITE|WRT_CSR;
}

/*****************************************************************************
 * DEC 4000 WATCH CHIP ACCESS						     *
 *****************************************************************************/

/*
 * Address of Watch Chip LBUS Mailbox, obtained at initialization.
 */
static struct mbox *kn430_watch_mb;

/*
 * During a panic, the time is written out.  Testing panicstr from
 * cobra_writetodr lets us know if we're about to go down, and provides
 * a convenient hook for flushing error log information.
 */
extern char *panicstr;

/*
 * KN430 Watch Chip Lock.  Needed for SMP.
 * 
 * NOTE: Currently the watch chip is accessed at splhigh, and writes block
 * 	 read access.  This could introduce unacceptable interrupt latencies 
 * 	 for realtime applications which periodically adjust the system time.
 * 	 This lock should be replaced with a multiple readers/single writer
 * 	 lock sometime in the future.  Reads which occur while the clock is
 * 	 being written to should just return the time being written:
 *
 *		Write:  
 *		    retry:
 *			splhigh; lock watch;
 *			if (wc->writing) {
 *				splx; unlock watch; goto retry;
 *			}
 * 			wc->time = new time;
 *			wc->writing = 1;
 * 			unlock watch; splx;
 * 			[set the time]
 *			splhigh; lock watch;
 *			wc->writing = 0;
 * 			unlock watch; splx;
 *
 *		Read:
 *			splhigh; lock watch;
 *			wc_copy = *wc;
 * 			unlock watch; splx;
 *			if (wc_copy.writing) return wc_copy.time;
 *			[read the time]
 *			return the time;
 */
decl_simple_lock_data(extern, kn430_watch_lock)

#define KN430_WATCH_INIT() simple_lock_init(&kn430_watch_lock)
#define KN430_WATCH_LOCK() simple_lock(&kn430_watch_lock)
#define KN430_WATCH_UNLOCK() simple_unlock(&kn430_watch_lock)

/*
 * kn430_watch_conf
 *
 * Configure a bus and mailbox for watch chip use.
 */
static int kn430_watch_conf()
{
	/*
	 * Initialize the watch lock.
	 */
	KN430_WATCH_INIT();

	/*
	 * Create a standalone mailbox structure for exclusive 
	 * use in communicating with the watch chip.
	 */
	kn430_watch_mb = MBOX_ALLOC();
	kn430_watch_mb->bus_timeout = LBUS_TIMEOUT;
	kn430_watch_mb->mbox_reg = (vm_offset_t)&Io_regs->lmbpr;
	kn430_watch_mb->mbox_cmd = toy_mbox_cmd;
	
	return 0;
}

/*
 * kn430_read_watch
 *
 * Read a byte from the Cobra watch chip.  The watch chip is accessed through
 * a mailbox.  The read is synchronous, since the Cobra LMBPR is implemented
 * with only one level of buffering.
 *
 * This routine should be called with the watch_lock held and at the
 * appropriate spl (see cobra_readtodr()).
 *
 * These read and write routines are here to avoid the need for a bus struct
 * for the watch chip.  If a bus struct ends up present anyway, we can go back
 * to using RDCSR and WRTCSR.  We may still want these routines, since they
 * make the calling routines look a little simpler.
 */

static int kn430_read_watch(register u_long remaddr)
{
	register int mbs;			/* Mailbox status. */
	
	/*
	 * Initiate the read by setting up the mailbox and writing its address
	 * to the Cobra LMBPR.
	 */
	mbs = mbox_setup(RD_CSR, BYTE_32, kn430_watch_mb, remaddr, 0);
	if (mbs != MBOX_SUCCESS) 
		mbox_error(kn430_watch_mb, "kn430_read_watch", mbs);
	
	/*
	 * Wait for the read to complete.  
	 * The Cobra LMBPR fifo is only one entry deep, 
	 * so there is no point to trying to be asynchronous.
	 */
	MBOX_WAIT(kn430_watch_mb, "kn430_read_watch");

	/*
	 * If the error bit is set, blow up.
	 */
	if(kn430_watch_mb->mb_status & MBOX_ERR_BIT) {
		mbox_error(kn430_watch_mb, "kn430_read_watch", MBOX_ERR);
		/* not reached */
	}
	
	/*
	 * Return the read data.  Mask off the low byte.  For some reason the
	 * mailbox appears to have garbage in the unused bits.
	 */
	return kn430_watch_mb->rdata & 0xFF;
}

/*
 * kn430_write_watch
 *
 * Routine to write a byte to the Cobra watch chip.  The write is dump and
 * run.  
 *
 * This routine should be called with the watch_lock held and at the
 * appropriate spl (see cobra_readtodr()).
 *
 */

static int kn430_write_watch
	(register u_long remaddr,	/* Remote address to write to. */
	 register int wdata)		/* Data to write. */
{
	register int mbs;			/* Mailbox status. */
	
	/*
	 * Initiate the write by setting up the mailbox and writing its address
	 * to the lmbpr.
	 */
	mbs = mbox_setup(WRT_CSR, BYTE_32, kn430_watch_mb, remaddr, wdata);
	if (mbs != MBOX_SUCCESS) {
		dumpmbox(kn430_watch_mb);
		if (mbs == MBOX_HUNGHOSE) {
			panic("kn430_write_watch: HUNGHOSE");
			/*NOTREACHED*/
		}
		else {  /* May not be QFULL, but need to do something... */
			panic("kn430_write_watch: QFULL");
			/*NOTREACHED*/
		}
	}
	
	return mbs;
}

/*
 * cobra_readtodr
 *
 * Read the time of day from the Cobra watch chip, which resides on the LBUS.
 * The watch chip must be accessed through the LBUS mailbox.
 *
 * The caller of this routine expects the time to be represented as a time of
 * year in hundred nanosecond units (10**7s).  The clock conversion routine
 * and this routine were designed to utilize the watch chip as a true clock,
 * with valid years in it.  Comments in the code indicate modifications which
 * were made to make this routine compatible with its caller.
 *
 * Use the watch chip SET bit to freeze the time buffers while reading the
 * time.  The chip must be in the appropriate mode, or the time will not be
 * read properly.
 */
long cobra_readtodr()
{
	register long todr;
	register int s;
	struct tm tm;
	register int csr_a, csr_b, csr_d;
	register int first_call;
	static long readtodr_calls = 0;
#define KN430_INVALID_TIME 0
	
	/*
	 * Lock out other access to the watch chip.  
	 * Need a simple lock for SMP.
	 * This should be changed in the future, to allow multiple readers, 
	 * single writer, or at least to avoid locking up the 
	 * whole system until the clock has been read...
	 */
	s = splhigh();
	KN430_WATCH_LOCK();

	/*
	 * Keep track of first call to this routine.  The clock gets read just
	 * before we enter single-user mode, so this is a good place to do
	 * last-minute CPU-specific activities which require the full single-
	 * user environment.
	 */
	first_call = (readtodr_calls == 0);
	readtodr_calls++;
	
	/*
	 * Make sure the clock's battery is good.  If not, might as well punt
	 * because we'll lose the time during power outages.
	 */
	csr_d = kn430_read_watch( KN430_WATCH_OFFSET(reg_d));
	if (!(csr_d & LBUS_TOY_VRT)) {
		printf("Watch chip battery malfunction (%x)\n", csr_d);
		kn430_write_watch( KN430_WATCH_OFFSET(reg_d), LBUS_TOY_VRT );
		KN430_WATCH_UNLOCK();
		splx(s);
		todr = KN430_INVALID_TIME;
		goto first_actions;
	}
	
	/*
	 * Wait until an update is not in progress (to avoid leaning on the SET
	 * bit halfway through an update).  Delay between reads to avoid
	 * saturating the bus or chip.
	 *
	 * This might not be needed, but we'll play it safe...
	 */
	csr_a = kn430_read_watch(KN430_WATCH_OFFSET(reg_a));
	while (csr_a & LBUS_TOY_UIP) {
		DELAY(LBUS_TIMEOUT);
		csr_a = kn430_read_watch( KN430_WATCH_OFFSET(reg_a) );
	}; 
	
	/*
	 * Since we had to read register A anyway, check to be sure that the 
	 * watch countdown chain is operating.  If not, try to turn it on.
	 */
	if ((csr_a & 0x70) != 0x20) {
		printf("Watch chip is not ticking: 0x%02x\n", csr_a);
		kn430_write_watch(KN430_WATCH_OFFSET(reg_a),
				  (csr_a & 0x8f) | 0x20);
		csr_a = kn430_read_watch(KN430_WATCH_OFFSET(reg_a));
	        printf("Started watch chip (Reg A now 0x%02x)\n", csr_a);
		csr_a = kn430_read_watch(KN430_WATCH_OFFSET(reg_a));
		while (csr_a & LBUS_TOY_UIP) {
			DELAY(10000);
			csr_a = kn430_read_watch( KN430_WATCH_OFFSET(reg_a) );
		};
	}
	
	/*
	 * Read CSR B, which has the SET bit and also the various mode bits.
	 * If the chip is not in the expected mode, return an erroneous time.
	 * Treat the time as invalid if the SET bit is on -- this allows Cobras
	 * to be shipped with the SET bit on to identify the watch as un-set, 
	 * and it also protects against power failures while the watch is 
	 * being set.
	 *
	 * If the chip is in a known mode, set the SET bit and read the 
	 * time fields.
	 * Leave SET mode when done.
	 *
	 * The counters are double-buffered, so the watch should continues 
	 * to keep time accurately even if an update occurs while SET is
	 * asserted.  If there is no problem with setting the SET bit 
	 * during an update, we can skip the
	 * spin loop on UIP above.
	 *
	 * Note that the SET bit is left on if one of the calls to 
	 * kn430_read_watch causes a panic.  The SET bit gets cleared 
	 * the next time a read or write occurs successfully.  
	 * This means that the time may be frozen after a
	 * panic.  Since we didn't see any mailbox failures during testing, 
	 * it's not worth defending against this...
	 */
	csr_b = kn430_read_watch(KN430_WATCH_OFFSET(reg_b));
	if (!((csr_b & LBUS_TOY_EXPECTED_MODE) != LBUS_TOY_EXPECTED_MODE)
	    || (csr_b & LBUS_TOY_SET)) {
		
		/* Enter SET mode. */
		kn430_write_watch(KN430_WATCH_OFFSET(reg_b), 
				  csr_b | LBUS_TOY_SET);
		
		/* Read the time. */
		tm.tm_sec  = kn430_read_watch(KN430_WATCH_OFFSET(seconds) );
		tm.tm_min  = kn430_read_watch(KN430_WATCH_OFFSET(minutes) );
		tm.tm_hour = kn430_read_watch(KN430_WATCH_OFFSET(hours) ); 
		tm.tm_mday = kn430_read_watch(KN430_WATCH_OFFSET(day_of_mon)); 
		tm.tm_mon  = kn430_read_watch(KN430_WATCH_OFFSET(month) );
		tm.tm_year = kn430_read_watch(KN430_WATCH_OFFSET(year) ); 
		
		/* Leave SET mode. */
		kn430_write_watch(KN430_WATCH_OFFSET(reg_b), 
				  csr_b & ~LBUS_TOY_SET);
	}
	else {
		KN430_WATCH_UNLOCK();
		splx(s);
	        printf("Invalid watch chip mode: 0x%02x\n", csr_b);
		todr = KN430_INVALID_TIME;
		goto first_actions;
	}
	
	/*
	 * Unlock the watch chip.
	 */
	KN430_WATCH_UNLOCK();
	splx(s);
	
	/*
	 * Map years.  The mapping is as follows:
	 *
	 * 	CHIP	DATE		INTERNAL
	 *   00-69	2000-2069	100-169
	 *   70-99	1970-1999	70-99
	 *
	 * This mapping was chosen to allow the current readtodr() to work
	 * correctly.  Right now, readtodr() always sets the watch to a time
	 * within the year 1970, because it expects the watch hardware to have
	 * a limited range.
	 */
	if (tm.tm_year < 70) {
		tm.tm_year += 100;
	}
	
	/*
	 * If no error was encountered, convert the watch time to the 
	 * time in seconds since the epoch.  The read convert routine 
	 * expects the year field to contain the year - 
	 * 1900 (i.e. 1970 is 70, 2001 is 101), and the month
	 * field to contain a base 0 month index.
	 */
	tm.tm_mon -= 1;		/* Month bases are different. */
	todr = cvt_tm_to_sec(&tm);
	
#if !_KN430_WATCH_STORES_DATE
	/*
	 * The caller, inittodr(), expects the time to be represented 
	 * in units of hundreds of nanoseconds, biased by 31 days.
	 */
	todr = TODRZERO + todr * UNITSPERSEC;
#endif /* !_KN430_WATCH_STORES_DATE */
	
	/*
	 * Perform actions which should be done the first time the clock is
	 * read.
	 */
first_actions:
	if (first_call) {
		/*
		 * Copy FRU table into error log.  Do it here because
		 * the binary error logger is initialized at this
		 * point.  If done earlier, the entry gets dropped on
		 * the floor.
		 */
		kn430_copy_fru_table();
	}

	return todr;
#undef KN430_INVALID_TIME
}

/*
 * cobra_writetodr
 *
 * Write the time of day to the Cobra watch chip, which resides on the LBUS.
 * The watch chip must be accessed through the LBUS mailbox.
 *
 * Use the watch chip SET bit to freeze the time buffers while writing the
 * time.  The chip must be in the appropriate mode, or the time will not be
 * written properly.
 *
 * The caller of this routine expects the time to be represented as a time of
 * year in hundred nanosecond units (10**7s).  The clock conversion routine
 * and this routine were designed to utilize the watch chip as a true clock,
 * with valid years in it.  Comments in the code indicate modifications which
 * were made to make this routine compatible with its caller.
 *
 */
int cobra_writetodr(register long todr)
{
	register int s;
	struct tm tm;
	register int csr_a, csr_b, csr_d;
	extern char *panicstr;
	
#if !_KN430_WATCH_STORES_DATE
	/*
	 * The caller provides a time of year in seconds since the epoch.  
	 * This is not symmetric with the call to cobra_readtodr()...
	 */
	/*  todr = (todr - TODRZERO) / UNITSPERSEC; */
#endif !_KN430_WATCH_STORES_DATE
	
	/*
	 * Convert time to watch chip format.  
	 * Number of seconds since the epoch
	 * (01-JAN-1970 00:00:00) is converted to a date.
	 */
	cvt_sec_to_tm(todr, &tm);
	tm.tm_mon += 1;
	
	/*
	 * Map years.  The mapping is as follows:
	 *
	 *	INTERNAL	DATE		CHIP
	 *      70-99        1970-1999		70-99
	 *      100-169      2000-2069          00-69
	 */
	if ((tm.tm_year > 69) && (tm.tm_year < 170)) {
		if (tm.tm_year >= 100) tm.tm_year -= 100;
	}
	else {
		printf("Invalid year (%d), using 1992\n", tm.tm_year + YRREF);
		tm.tm_year = 92;
	}

	/*
	 * Lock out other access to the watch chip.  
	 * Need a simple lock for SMP.
	 */
	s = splhigh();
	KN430_WATCH_LOCK();
	
	/*
	 * Make sure the clock's battery is good.  If not, might as well punt
	 * because we'll lose the time during power outages.
	 */
	csr_d = kn430_read_watch(KN430_WATCH_OFFSET(reg_d));
	if (!(csr_d & LBUS_TOY_VRT)) {
		printf("\nWatch chip battery malfunction (0x%02x)\n", csr_d);
		KN430_WATCH_UNLOCK();
		splx(s);
		return 1;
	}
	
	/*
	 * Wait until an update is not in progress (to avoid leaning on the SET
	 * bit halfway through an update).  Delay between reads to avoid
	 * saturating the bus or chip.
	 *
	 * This might not be needed, but we'll play it safe...
	 */
	csr_a = kn430_read_watch(KN430_WATCH_OFFSET(reg_a));
	while (csr_a & LBUS_TOY_UIP) {
		DELAY(10000);
		csr_a = kn430_read_watch(KN430_WATCH_OFFSET(reg_a));
	}; 
	
	/*
	 * Read CSR B, which has the SET bit and also the various mode bits.
	 * Set the watch mode bits which we care about to be sure that it 
	 * can handle the time representation 
	 * (binary, no daylight savings, 24 hour mode).  
	 * For any bits we don't care about, use the previous settings.
	 */
	csr_b = kn430_read_watch(KN430_WATCH_OFFSET(reg_b));
	csr_b |= LBUS_TOY_EXPECTED_MODE & ~(LBUS_TOY_SET|LBUS_TOY_DSE);
	
	/* Enter SET mode. */
	kn430_write_watch(KN430_WATCH_OFFSET(reg_b),
			  csr_b | LBUS_TOY_SET);

	/* Write the time. */
	kn430_write_watch( KN430_WATCH_OFFSET(seconds),    tm.tm_sec);
	kn430_write_watch( KN430_WATCH_OFFSET(minutes),    tm.tm_min);
	kn430_write_watch( KN430_WATCH_OFFSET(hours),      tm.tm_hour); 
	kn430_write_watch( KN430_WATCH_OFFSET(day_of_mon), tm.tm_mday); 
	kn430_write_watch( KN430_WATCH_OFFSET(month),      tm.tm_mon);
	kn430_write_watch( KN430_WATCH_OFFSET(year),       tm.tm_year); 
	
	/* Leave SET mode. */
	kn430_write_watch(KN430_WATCH_OFFSET(reg_b), 
			  csr_b & ~LBUS_TOY_SET);
	
	/*
	 * Unlock the watch chip.
	 */
	KN430_WATCH_UNLOCK();
	splx(s);

	/*
	 * If we're going down due to a panic, try to shove footprint data into
	 * the error log buffer, so that it gets picked up from the coredump
	 * and placed in the error log.  Otherwise, the footprints can only be
	 * seen using dbx -k on the core image -- they need to be in the error
	 * log in case memory wierdness was part of the reason we went down.
	 */
	if (panicstr) {
		kn430_checkpoint_footprints();
	}

	return 0;
}

/*
 * Ddefinitions needed to get_info routine 
 */
#define	FBUS_4000_INTR_REQ_REG	(0xfffC0800)
#define	COBRA_MOP_SYSID		154

/*
 *
 *   Name: kn430_getinfo(item_list) - Gets system specific information for the requestor
 *
 *   Inputs:	item_list - List of items the caller wants information about.
 *
 *   Outputs:	returned information or NOT_SUPPORTED for each item requested
 *
 *   Return	
 *   Values:	NA.
 */
u_int kn430_getinfo(request)
	struct item_list *request;

{
	do {
		request->rtn_status = INFO_RETURNED;
		switch(request->function) {
			case TEGC_SWITCH:
				request->output_data = (vm_offset_t)&cobra_sw[request->input_data];
				break;
			case FBUS_INTREQ_REG:
   				request->output_data = (long)FBUS_4000_INTR_REQ_REG;
				break;				
			case MOP_SYSID:
   				request->output_data = COBRA_MOP_SYSID;
				break;
			default: 
				request->rtn_status = NOT_SUPPORTED;
		}
		request = request->next_function;		
	} while (request != NULL);
	return(TRUE);
}

/*
 *
 *   Name: kn430_dump_dev() - translate dumpdev into a string the console can understand
 *
 * Translates the SCSI and MSCP device and target data into a device string which can be passed to
 * prom_open. Which a generic IO console callback. See the SRM for details
 *
 *   Inputs:	Dump_req - generic dump info 
 *
 *   Outputs:	dump_req->dev_name - device string for the console
 *
 *   Return	
 *   Values:	NA.
 */
kn430_dump_dev(dump_req)
	struct dump_request *dump_req;

{
	char *device_string;
	char temp_str[8];


	device_string = dump_req->device_name;
	/*
	 * for Cobra the BOOTED_DEV string is:
	 *	SCSI 0 <bus> 0 <target> <target*100> 0 0
	 */
	strcpy(device_string,"SCSI ");
	strcpy(&device_string[strlen(device_string)],"0 ");

	itoa(dump_req->device->ctlr_hd->ctlr_num,temp_str); 
	strcpy(&device_string[strlen(device_string)],temp_str);
	strcpy(&device_string[strlen(device_string)]," 0 ");

	itoa(dump_req->unit,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);
	strcpy(&device_string[strlen(device_string)]," ");

	itoa(dump_req->unit*100,temp_str);
	strcpy(&device_string[strlen(device_string)],temp_str);
	strcpy(&device_string[strlen(device_string)]," 0 0");

}
/* 
 * 	cobra_set_system_string() - This routine sets the system string.  Note that
 *	it assumes cobra series machines will have at most 2 processors, and it uses
 *	the bcache size to distinguish between cobra and fang.
 */
cobra_set_system_string()
{
	extern struct rpb *rpb;
	struct rpb_dsr *dsr;
	extern char *platform_string();
	static char    DEC_4000_610_STRING[] = "DEC4000 Model 610";
	static char    DEC_4000_620_STRING[] = "DEC4000 Model 620";
	extern struct cpusw *cpup;	/* pointer to cpusw entry */

	if (rpb->rpb_vers < 5) /* Backward compatible with the old way */
		/* must be a 610 or 620 because 710/720 doesn't work with early version */
		cpup->system_string = ((rpb->rpb_numprocs == 1) ? 
				       DEC_4000_610_STRING : DEC_4000_620_STRING);

	else
		cpup->system_string = platform_string();
}
