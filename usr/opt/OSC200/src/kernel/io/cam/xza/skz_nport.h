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
#if !defined(SKZ_NPORT_INCLUDE)
#define SKZ_NPORT_INCLUDE

/************************************************************************
 *									*
 * File:	skz_nport.h						*
 * Date:	November 5, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *    This file contains nport structure definitions			*
 *									*
 ************************************************************************/

#define	MAX_NP_CHANNELS	16

#define ADAPTER_STOPPER	((vm_offset_t) 0)
#define DRIVER_STOPPER ((vm_offset_t) 1)

/*
**
** Queue Structures
**
*/

/*
** 	Carrier Structure
*/
typedef struct np_carrier {

	vm_offset_t	next_ptr;
	vm_offset_t	qb_ptr;
	long		qb_token;
	long		car_token;

} CARRIER;

/*
**	Queue Structure
*/
typedef struct np_queue {

	CARRIER		*head_ptr;
	CARRIER		*tail_ptr;

} NP_Q;

typedef enum queue_types {
	DRIVER_ADAPTER,
	ADAPTER_DRIVER,
	DRIVER_DRIVER,
	ADAPTER_ADAPTER
} QUEUE_TYPE;

typedef enum queue_names {
	ADRQ, ADFQ, DAFQ, DCCQ0, DCCQ1, DCCQ2, DDFQ, DDEQ, DDEFQ, DDSGQ
} QUEUE_NAMES;


/*
**
** Driver Channel Command Queues
**
*/
struct dcc_queues {
	NP_Q	reserved;
	NP_Q	dccq2;
	NP_Q	dccq1;
	NP_Q	dccq0;

	/*
	** Not used by XZA but defined by N_Port
	*/
	NP_Q	cccq3;
	NP_Q	cccq2;
	NP_Q	cccq1;
	NP_Q	cccq0;
};

/*
** Adapter Parameter Block
*/
typedef struct apb {
	unsigned short		next_len;	/* # of nexus table entries */
	unsigned short		ibuf_len;	/* internal buf size = 4112 */
	unsigned short		rampb;		/* requested ampb size */
	unsigned short		type;		/* adapter type = 2 */

	unsigned long		imp_spec;	/* always zero */
	unsigned long		reserved1;	/* always zero */
	unsigned long		reserved2;	/* always zero */
} APB;

/*
** Channel Parameter Block
*/
typedef struct cpb {
	unsigned short		xp_addr;
	/*
	** fields not used by XZA
	*/
	unsigned char	max_pgrp;
	unsigned char	max_mem;
	unsigned int	sbz1;

	unsigned short	dlink_typ;	/* datalink type, = 2 for SCSI */
	/*
	** fields not used by XZA
	*/
	unsigned char	dlink_param[6];

	unsigned long	imp_spec1;
	unsigned long	imp_spec2; 
} CPB;

/*
** Interrupt data 
*/
typedef struct intr_data {
	unsigned int		slot : 4;	/* XMI node id of lamb */
	unsigned int		vector : 10;	/* interrupt vector */
	unsigned int		level : 2;	/* encoded IPL */
	unsigned short		altvec;		/* must be zero */
} INTR_DATA;

/*
** Command Queue Insertion Block
*/
typedef struct cqib {
	struct {
		unsigned long	ccq2ir;
		unsigned long	ccq1ir;
		unsigned long	sbz;
	} qs[2];
	unsigned long	afqir;
} CQIB;

/*
** Adapter Memory Page
*/
typedef struct amp {
	CQIB		cqib;	/* channel queue insertion block */
} AMP;

typedef struct amp_ptr {
	unsigned long	v	: 1;	/* valid bit */
	unsigned long	sbz	: 12;	/* should be zero */
	unsigned long	pa	: 19;	/* physical address of port page */
	unsigned long	pax	: 16;	/* physical address of port page */
	unsigned long	sbz1	: 16;	/* should be zero */
} AMP_PTR;

typedef struct ampb_base {
	unsigned long	v	: 1;	/* valid bit */
	unsigned long	sbz1	: 4;	/* should be zero */
	unsigned long	pa	: 27;	/* physical address of AMPB */
	unsigned long	pax	: 16;	/* physical address of AMPB */
	unsigned long	sbz2	: 16;	/* should be zero */
} AMPB_BASE;

/*
** Adapter Block
*/

typedef struct adapter_block {

	struct dcc_queues	dccq[XZA_CHANNELS];

	/*
	** Allocate AB space for unused channels, defined by N_Port
	** but not used by the XZA
	*/
	struct dcc_queues	unused_queues[MAX_NP_CHANNELS-XZA_CHANNELS];	

	NP_Q			adrq;	/* adapter-driver response queue */
	NP_Q			dafq;	/* driver-adapter free queue */
	NP_Q			adfq;	/* adapter-driver free queue */

	/*
	** Allocate space in the AB for N_Port defined queues
	** as well as queues used by the XZA
	*/
	NP_Q			damfq_unused;
	NP_Q			admfq_unused;

	NP_Q			aafq;	/* adapter-adapter free queue */

	/*
	** Other unused queues
	*/
	NP_Q			aamfq_unused;

	/*
	** Buffer Descriptor Leaf Table base address, not used by XZA
	*/
	vm_offset_t		bdlt_base;	/* base address */
	unsigned int		bdlt_len;	/* bdlt table length */

	unsigned int		keep_alive;	/* maintenance timer, 100n */

	unsigned int		qe_len;		/* queue buffer length, bytes */
	unsigned int		mqe_length_unused;	

	unsigned long		intr_holdoff;
	
	AMPB_BASE		ampb_base;	/* adapter memory ptr block */
	unsigned int		ampb_len;	/* ampb length */

	unsigned int		reserved1;

	/*
	** Interrupt vector data
	*/
	INTR_DATA		c0_misc_intr_data;	/* chan 1 intr */
	INTR_DATA		c1_misc_intr_data;	/* chan 2 intr */
	INTR_DATA		adrq_intr_data;		/* response intr */

	char			reserved2[20];

	/*
	** Adapter parameter block, Channel parameter blocks
	*/
	APB			apb;
	CPB			cpb[XZA_CHANNELS];

	/*
	** allocate AB space for N_Port defined channels unused by XZA
	*/
	CPB			cpb_unused[MAX_NP_CHANNELS-XZA_CHANNELS];

	

} AB;

/*
** NP Routine Return Values
*/
typedef enum np_status {
	NP_SUCCESS,
	NP_FAILURE
} NP_STATUS;

#endif /* SKZ_NPORT_INCLUDE */
