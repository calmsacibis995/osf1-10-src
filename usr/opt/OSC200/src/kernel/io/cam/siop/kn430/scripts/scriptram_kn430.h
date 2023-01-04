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

#ifndef _SCRIPTRAM_KN430_H_ 
#define _SCRIPTRAM_KN430_H_ 


#define SCRIPT_RAM_SIZE		(128*1024)
#define MAX_SCRIPT_SIZE		(SCRIPT_RAM_SIZE/SIOP_NCTLR)
#define SCRIPTRAMBASE		(0x80000000)

/* Mask to mask only the valid (from the host's perspective) SCRIPTS RAM
 * address bits.
 */
#define SCRIPT_RAM_MASK		(SCRIPT_RAM_SIZE-1)

#define BOOTBASE	0x1ffe0L	/* location of boot SCRIPT */

/* Location of SCRIPTS stuff that must be known to both the host and
 * SCRIPTS.
 */
#define SIOP_SR_JOBTABLE	0x6400
#define SIOP_SR_TDTABLE		0x6500
#define SIOP_SR_SCRIPTHOST	0x6558

/* The address of the start of the tag cache table.  This is a table of
 * the host physical address of the last 32 tagged requests sent to
 * a target/lun.  This table is accesses as a direct mapped cache
 * using the the tag, the target ID
 * as the table index, and the LUN as the
 * cache tag.  The algorithm used to convert a target/lun/tag
 * combination to a table address is:
 *
 *	table_base + ((tid<<10) + (tag&0xfc) + ((tag&3)<<8))
 *
 * Things are done this way to make the SCRIPTS algorith for calculation
 * the address simpler with the minimal set of SCRIPTS instructions.
 *
 * When a tagged request is started the host will store the address of
 * the tag cache entry in the sj_tag field of the SIOPJOB structure.
 * When the SCRIPTS starts the request it will place the host physical
 * address of the SIOPJOB structure at this address if the cache line
 * in empty (to prevent the possibility of no cache-hits when there
 * are a series of cache line collisions).  Since the SIOPJOB
 * structure will always be quadword aligned (on 32 bit machine it MUST
 * be quadword aligned), the lower three bits of this address is set to
 * the logical unit number.
 *
 * When a reconnect comes in, the SCRIPTS will calculate to cache address
 * based on the targer/lun/tag of the reconnecting request.  If there is
 * a non-zero value stored atthis address this it will compare the LUN
 * the reconnecting request to the lower three bits of the
 * longword in the cache.  If they match then this is a hit and the
 * SIOPJOB address from the cache is used.  If there is a miss then the
 * SCRIPTS will perform the LOOK_TAG RPC to find the request. 
 *
 * This should save the SCRIPTS from performaing the LOOK_TAG RPC for
 * most requests.  This will work best when there is only one logical
 * unit in use on a target since there will be no cache line collisions
 * and the SCRIPTS will be able to keep track of all 256 tagged requests
 * without cache misses.  For multiple LUN targets, hopefully the tags
 * will be sufficiently random (with respect to each other) to prevent
 * and large number of misses.
 *
 */
#define SIOP_SR_TAG_CACHE	0x4000
#define MAX_TAG			255		/* biggest tag allowed */

#ifdef _INSCRIPTS_
#define SCRIPT_RAM_TAG_CACHE		start+SIOP_SR_TAG_CACHE
#else
#define SIOP_TAG_CACHE_LINE(T,A)	\
		((((T)&7)<<10) + ((A)&0xfc) + (((A)&3)<<8) + \
		SIOP_SR_TAG_CACHE)
#endif

/* A place to copy the siopjob structure for the currently active
 * request.  This must be 512 bytes long and aligned on 256 bytes.
 * The datai array MUST land on start+0x6100.
 */
#ifdef _INSCRIPTS_
#define	siopjob			start+0x6100-SJ_COPYOUT_SIZE
#define siopjob_datai		start+0x6100
#define siopjob_cmdptr		siopjob+SJ_OFFSET_CMDPTR
#define siopjob_msgoptr		siopjob+SJ_OFFSET_MSGOPTR
#define siopjob_smsgoptr	siopjob+SJ_OFFSET_SMSGOPTR
#define siopjob_soffset		siopjob+SJ_OFFSET_SOFFSET
#define siopjob_loffset		siopjob+SJ_OFFSET_LOFFSET
#define siopjob_doffset		siopjob+SJ_OFFSET_DOFFSET
#define siopjob_sdatai		siopjob+SJ_OFFSET_SDATAI
#define siopjob_ldatai		siopjob+SJ_OFFSET_LDATAI
#define siopjob_sdatap		siopjob+SJ_OFFSET_SDATAP
#define siopjob_ldatap		siopjob+SJ_OFFSET_LDATAP
#define siopjob_err		siopjob+SJ_OFFSET_ERR
#define siopjob_status		siopjob+SJ_OFFSET_STATUS
#define siopjob_tid		siopjob+SJ_OFFSET_TID
#define siopjob_lun		siopjob+SJ_OFFSET_LUN
#define siopjob_tagged		siopjob+SJ_OFFSET_TAGGED
#define siopjob_tag		siopjob+SJ_OFFSET_TAG
#define siopjob_term		siopjob+SJ_OFFSET_TERM
#define siopjob_abort		siopjob+SJ_OFFSET_ABORT
#define siopjob_me		siopjob+SJ_OFFSET_ME
#define siopjob_version		siopjob+SJ_OFFSET_VERSION

/* Active job pointer table for untagged requests:  256 bytes */
#define SCRIPT_RAM_JOBTABLE	start+SIOP_SR_JOBTABLE

/* Table of Sync offset values: 8 bytes.
 * Store the value for the SXFER register.
 */
#define SCRIPT_RAM_TDTABLE	start+SIOP_SR_TDTABLE

/* Location to which the host writes the address of the scripthost
 * structure.  THIS MUST ALWAYS BE THE LAST LOCATION IN AN SIOP'S
 * MEMORY SPACE.
 */
#define SCRIPT_RAM_SH		start+SIOP_SR_SCRIPTHOST
#endif

#endif 
