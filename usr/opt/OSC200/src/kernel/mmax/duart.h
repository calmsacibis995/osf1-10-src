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
 *	@(#)$RCSfile: duart.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:31 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * Copyright (c) 1989 Encore Computer Corporation
 */

#ifndef _DUART_H
#define	_DUART_H


#ifndef	LOCORE
typedef struct duart_ctl_regs {
        unsigned int
                mode_a,         /* mode register A      MRA     */
                status_a,       /* status/clock select  SRA     */
                cmd_a,          /* command register A   CRA     */
                data_a,         /* holding data A       HRA     */
                auxctl,         /* Auxiliary control    ACR     */
                intr_stat,      /* interrupt stat/mask  ISMR    */
                reserved,       /* unused       offset: 0x18    */
                reserved2,      /* unused       offset: 0x1C    */
                mode_b,         /* mode register B      MRB     */
                status_b,       /* status/clock select  SRB     */
                reserved3,      /* unused       offset: 0x28    */
                reserved4,      /* unused       offset: 0x2C    */
                cmd_b,          /* command register B   CRB     */
                data_b;         /* holding data B       HRB     */
} duart_ctl_t;
#endif	LOCORE

/*
 * bits in MRA/MRB
 */
#define DUART_MR1_RTS           0x80    /* RTS enable           */
#define DUART_MR1_RXFULL        0x40    /* RX INT select        */
#define DUART_MR1_BLOCK         0x20    /* error mode           */
#define DUART_MR1_FPAR          0x08    /* force parity         */
#define DUART_MR1_NOPAR         0x10    /* no parity            */
#define DUART_MR1_MDROP         0x18    /* multi-drop mode      */
#define DUART_MR1_ODDP          0x04    /* odd parity if set    */
#define DUART_MR1_5BITS         0x00    /* 5 bits per char      */
#define DUART_MR1_6BITS         0x01    /* 6 bits per char      */
#define DUART_MR1_7BITS         0x02    /* 7 bits per char      */
#define DUART_MR1_8BITS         0x03    /* 8 bits per char      */
#define DUART_MR2_ECHO          0x40    /* channel auto-echo    */
#define DUART_MR2_LLOOP         0x80    /* channel local loop   */
#define DUART_MR2_RLOOP         0xc0    /* channel remote loop  */
#define DUART_MR2_RTS           0x20    /* Tx RTS enable        */
#define DUART_MR2_CTS           0x10    /* Tx CTS enable        */
#define DUART_MR2_STOPB         0x0f    /* stop bit length mask */
/*
 * bits in CRA/CRB
 */
#define DUART_TX_DISABLE        0x08    /* Tx disable           */
#define DUART_TX_ENABLE         0x04    /* Tx enable            */
#define DUART_RX_DISABLE        0x02    /* Rx disable           */
#define DUART_RX_ENABLE         0x01    /* Rx enable            */
                                        /* A/B Commands:        */
#define DUART_NO_CMD            0x00    /* Not a command        */
#define DUART_RESET_MR          0x10    /* reset MR pointer     */
#define DUART_RESET_RX          0x20    /* reset receiver       */
#define DUART_RESET_TX          0x30    /* reset transmitter    */
#define DUART_RESET_ERR         0x40    /* reset error          */
#define DUART_RESET_BRK         0x50    /* reset break intr.    */
#define DUART_BRK_ON            0x60    /* start break          */
#define DUART_BRK_OFF           0x70    /* stop break           */
/*
 * bits in SRA/SRB
 */
#define DUART_BRK               0x80    /* received break       */
#define DUART_FRERR             0x40    /* framing error        */
#define DUART_PARERR            0x20    /* parity error         */
#define DUART_OVERRUN           0x10    /* overrun error        */
#define DUART_TXEMT             0x08    /* Transmitter EMPTY    */
#define DUART_TXRDY             0x04    /* Transmitter READY    */
#define DUART_TXRDY_BIT         2
#define DUART_FFULL             0x02    /* FFULL                */
#define DUART_RXRDY             0x01    /* Receiver READY       */
#define DUART_RXRDY_BIT         0
/*
 * bits in ACR
 */
#define DUART_BRG_SET2          0x80    /* select baud set #2   */

#endif	_DUART_H
