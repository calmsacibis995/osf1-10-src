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
 * @(#)$RCSfile: sim_target.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/07 20:40:23 $
 */
#ifndef _SIM_TARGET_
#define _SIM_TARGET_

/* ---------------------------------------------------------------------- */

/* sim_target.h		Version 1.01			Nov 13, 1991 */

/*  This file contains the definitions needed for target mode operation
    of the SIM.

Modification History

	1.01	11/13/91	janet
	Added struct VERS defines.

	1.00	06/12/90	janet
	Created this file.
*/

/* ---------------------------------------------------------------------- */

#define SIM_TARG_MODE_CMD_LN 	16
/*
 * The following structure is the target working set used during target 
 * mode operation.
 */
typedef struct sim_targ_ws {
#define SIM_TARG_WS_VERS 1
    U32 flags;			/* See below */
    u_char it_conn;		/* Bit code of initiator and bit code	*
				 * of the target will be set.  Read off	*
				 * the SCSI bus during the selection.	*/
    U32 disconnect;		/* Set to zero if disconnects are disabled */
    u_char command[SIM_TARG_MODE_CMD_LN];
    				/* CDB command recieved from initiator. */
    u_char command_len;		/* Size of CDB received from initiator. */
} SIM_TARG_WS;

/*
 * Target working set flags bit definitions.
 */
#define TARG_GOT_CMD	 0x00000001	/* Set once a CDB has been read  */
					/* in from the fifo */
#define TARG_ATNSET	 0x00000002	/* Set if ATN is set */
#define TARG_CMDCPLT	 0x00000004	/* Set once command is to be completed*/
					/* next phase should be status phase */
/*
 * Information for the default data used for the inquiry command.
 */
#define TARG_VENDOR_ID		"DEC     "
#define TARG_PRODUCT_ID		"CAM TARGET MODE "
#define TARG_REVISION		"1.00"

/*
 * Index into ccb cam_iorsvd0 field where the initiator address is kept.
 */
#define INITIATOR_BUS		0
#define INITIATOR_TARGET	1
#define INITIATOR_LUN		2

				/* Maximum target mode command length */
#endif /* _SIM_TARGET_ */
