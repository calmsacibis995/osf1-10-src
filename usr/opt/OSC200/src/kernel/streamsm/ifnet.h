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
#define ifnetMinPacket    0
#define ifnetMaxPacket    ETHERMTU	/* 1518-4-14 */
#define ifnetHiWater      ETHERMTU*8
#define ifnetLoWater      ETHERMTU*5

#define IFNET_ID 5501 
#define IFNET_NAME       "ifnet"
#define IFNET_DEV_NAME   "ln"
#define IFNET_VERSION    "Ethernet Interface (STREAMS)"
#define MAX_STREAMS  64
#define MAX_UNITS     2

#define IFNET_ATTACHED 1 
#define IFNET_BOUND    2

#define IFNET_IOCTL_UNIT 1236

