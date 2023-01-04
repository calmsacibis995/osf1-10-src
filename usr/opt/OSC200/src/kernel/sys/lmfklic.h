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
 * @(#)$RCSfile: lmfklic.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/07/15 18:51:39 $
 */
/*            @(#)lmfklic.h	4.1  (ULTRIX)        7/2/90        */

/* Definitions and interfaces to the LMF kernel code
 *
 *  3-Dec-89	Giles Atkinson
 *	Better support for dynamic SMM change in multi-processors.
 *
 * 19-Sep-89	Lisa Allgood
 *	Moved LMF_TOKEN and LMF_HARDWARE to lmf.h
 *
 * 29-Jun-89	Giles Atkinson
 *	Add support for P_FAMILY changes.
 *
 *  1-Jun-89	Giles Atkinson
 *	Moved definition of LMFF_MORE to lmf.h
 *
 * Lisa Allgood and Giles Atkinson - 4th May 1989
 *	Original version
 */

#ifndef _SYS_LMFKLIC_H_
#define _SYS_LMFKLIC_H_

#include <sys/types.h>
#ifdef _KERNEL
#include <sys/exit_actn.h>
#endif
#include <sys/lmf.h>

/* Values for flag argument to getsysinfo */

#define LMF_GETSERV 1
#define LMF_GETSMM  2
#define LMF_GETLIC  3

/* Values for flag argument to setsysinfo */

#define LMF_SETSERV 1
#define LMF_SETLIC  2
#define LMF_ADJLIC  3
#define LMF_GETAUTH 4
#define LMF_RELAUTH 5
#define LMF_SETSMM  6

/* Maximum size of array of unsigned shorts that is returned for LMF_GETSMM.
 * This contains the current SMM, a count and an array of all possible
 * SMM values on a multi-processor.   The count contains is the maximum
 * number of CPUs and the size of the rest of the array.
 * Currently allow for up to 10 CPUs.
 */

#define MAX_SMMD    12

/* Flag bits used on some setsysinfo calls - must be in high two bytes.
 * Note: an additional flag LMFF_MORE is defined in lmf.h.
 */

#define LMF_CMASK   0xffff			/* To mask off command */
#define LMFF_COPY   (1<<17)			/* Used with LMF_ADJLIC */
#define LMFF_CLR_RESTRICTED (1<<18)		/* Used with LMF_ADJLIC */

/* Hash table size for lmf_klicense structure */

#define	HASHBITS 3
#define HASHSIZE (2<<HASHBITS)
#define HASHMASK (HASHSIZE-1)

/* String sizes for lmf_klicense structure */

#define LMF_PRODUCT	25
#define LMF_PRODUCER	25

/* Codes used in kl_flags.fl_status */

#define USED		1
#define NEW_ENTRY	2
#define NO_CHANGE	3
#define NOT_USED	0

typedef struct lmf_klicense {
	struct lmf_klicense  *kl_next;		/* Pointer to next record   */
	time_t kl_release_date;			/* Product Release Date	    */
	time_t kl_termination;			/* Termination date -
						   minimum of PAK termination 
						   and customer termination */
	ver_t   kl_version;			/* Product Version	    */
	int 	kl_act_charge;			/* Product Activity Charge  */
	int 	kl_locked_units;		/* Locked units             */
	int 	kl_usable_units;		/* Usable activity units    */
	int 	kl_total_units;			/* Total units              */
	unsigned short kl_max_cpus;		/* Maximum allowed CPUs     */
	unsigned short kl_pad_s1;		/* Padding */

	struct {
		unsigned fl_no_cache : 1;	/* Indicates that a signal
						   should be delivered when
						   units are released.	    */
		unsigned fl_local : 1;		/* Indicates some fields 
						   generated from local ldb.*/
		unsigned fl_restricted : 1;	/* Indicates units allocated 
						   locally during network
						   or server failure        */
		unsigned fl_family : 1;		/* An ancestor which holds
						   license units implicitly
						   licenses its descendants */
		unsigned fl_used_smm : 1;	/* The SMM was significant
						   in setting charge        */
		unsigned fl_blocked : 1;	/* Blocks use of product    */
		unsigned : 18;			/* Pad to 24 bits           */

/* The Following flags are not true flags and are not stored in the kernel  */
		unsigned fl_status : 8;		/* Used in the reset command
						   to indicate whether a 
						   klic has been matched 
						   with an ldb record (USED),
						   not matched (NOT_USED), 
						   matched but can't be 
						   updated (NO_CHANGE), or
						   its a new klic 
						   (NEW_ENTRY). These values
						   are defined in reset.h   */
	} kl_flags;				/* Status Flags             */
	char 	kl_product_name[LMF_PRODUCT];	/* Product Name		    */
	char 	kl_producer[LMF_PRODUCER];	/* Producer Name	    */
	char	kl_product_token[LMF_TOKEN];	/* Product Token	    */
	char	kl_hardware_id[LMF_HARDWARE];	/* Hardware id		    */
} klic_t;

#ifdef KERNEL

/* Extended klic_t which is used when the P_FAMILY option is set */

struct xklic {
	struct lmf_klicense xkl_klic;		/* Usual klic_t */
	struct exit_klic *xkl_usage[HASHSIZE];	/* Pointers to usage data.
						   Only if fl_family set    */
};

/*
 *	The exit_actn addition for lmf
 */
struct exit_klic{
	struct exit_actn  xk_action;		/* Linkage and function
						   pointers */
	klic_t		 *xk_klic;		/* Pointer to product data */
	int		  xk_uses;		/* Allocated units ... */
	int		  xk_charge;		/*  is product of these */
						/* The remaining fields are
						 * for the P_FAMILY option.
						 */
	pid_t		  xk_pid;		/* Owning process  */
	struct exit_klic *xk_indirect;		/* Pointer to master */
	int		  xk_refs;		/* Reference count if master */
	struct exit_klic *xk_forw, *xk_back;	/* Links to other exit_klic
						 * structures for this product.
						 */
};

#define xk_next	xk_action.xa_next
#define xk_func	xk_action.xa_func

#endif

#endif
