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
 * @(#)$RCSfile: cam_config.h,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 13:33:56 $
 */
#ifndef	CAM_CONFIG_INCLUDE
#define	CAM_CONFIG_INCLUDE	1

/************************************************************************
 *
 * File:	cam_config.h
 * Date:	March 8, 1991
 * Author:	Robin T. Miller
 *
 * Description:
 *	CAM peripheral driver configuration definitions.
 *
 * Modification History:
 *
 *    91/06/17  RPS        Added TYPEDEF
 */

#define	INIT_CAM_PDRV_LIMIT	16		/* # of p. driver tbl. ent. */

/*
 * CAM Peripheral Driver Configuration Structure.
 */

typedef struct cam_peripheral_driver
    {
    char        *cpd_name;
    I32         (*cpd_slave)();
    I32         (*cpd_attach)();
    I32         (*cpd_unload)();
    } CAM_PERIPHERAL_DRIVER;

#endif /* CAM_CONFIG_INCLUDE */
