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
 * @(#)$RCSfile: fbus_loadable.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/07/22 18:07:17 $
 */

struct fbus_intr_info {
        caddr_t configuration_st;       /* Pointer to bus/controller struct */
        int (*intr)();                  /* Interrupt handler */
        caddr_t param;                  /* param for interrupt handler */
        unsigned int config_type;       /* Driver type, FBUS_ADPT or FBUS_CTLR */
	unsigned int vector_offset;	/* Fbus interrupt vector for device to use */
	long reserved[6];
};

struct fbus_handler_info {
	vm_offset_t vec_addr;
	int vec_offset;
        int (*intr)();                  /* Interrupt handler */
        caddr_t param;                  /* param for interrupt handler */
	struct fbus_handler_info *self_reference;
};
