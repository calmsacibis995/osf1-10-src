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
 * @(#)$RCSfile: sim_config.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 13:46:38 $
 */
#ifndef	SIM_CONFIG_INCLUDE
#define	SIM_CONFIG_INCLUDE	1

/************************************************************************
 *
 * File:	sim_config.h
 * Date:	June 17, 1991
 * Author:	Robert P. Scott
 *
 * Description:
 *	CAM SIM subsystem configuration definitions.
 *
 * Modification History:
 *
 *      08/17/91    rps    Added chip reset pointer to hba struct.
 */

#define	INIT_CAM_HBA_LIMIT	16		/* # of sim drv. tbl. ent. */
#define	INIT_CAM_DME_LIMIT	16		/* # of sim drv. tbl. ent. */

typedef struct cam_hba_list_entry 
    {
    char        *cs_name;
    int         (*cs_probe)();
    int         (*cs_attach)();
    int         (*cs_reset_attach)();
    int         (*cs_unload)();
    U32	        (*hba_init)();	/* initialize the HBA			*/
    U32         (*hba_go)();	/* function to start a command off	*
    U32         (*hba_sm)();	/* HBA specific state machine		*/
    U32         (*hba_bus_reset)();
    U32         (*hba_send_msg)();
    U32         (*hba_xfer_info)();
    U32         (*hba_sel_msgout)();
    U32         (*hba_msgout_pend)();
    U32         (*hba_msgout_clear)();
    U32         (*hba_msg_accept)();
    U32         (*hba_setup_sync)();
    U32         (*hba_discard_data)();
    } CAM_HBA_LIST_ENTRY;

typedef struct cam_dme_list_entry 
    {
    char        *dme_name;
    int         (*dme_init)();
    int         (*dme_unload)();
    void 	*base_address;
    } CAM_DME_LIST_ENTRY;

#endif /* SIM_CONFIG_INCLUDE */












