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
 *	@(#)$RCSfile: scc_attns.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:42:54 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        	Copyright 1986 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED.     Licensed Material  -  Property of Encore Computer
 * Corporation.  This software is made available solely pursuant to the terms
 * of a  software  license  agreement  which governs  its use.   Unauthorized
 * duplication,  distribution or sale  are  strictly prohibited.
 *
 * Include file description:
 * 	Defines all of the SCC-specific attention messages.
 *
 * Original Author: Thomas B. Westbom	Created on: June 24, 1986
 *
 */
/*  There are four types of environmental conditions.  They are:
 *
 *	1.  Temperatures or voltages marginally too low (warning).
 *	2.  Temperatures or voltages marginally too high (warning).
 *	3.  Temperatures or voltages dangerously low (shutdown).
 *	4.  Temperatures or voltages dangerously high (shutdown).
 *
 *  There are 10 different temperatures or voltages which may be causing
 *  the warning or shutdown conditions.  They are:
 *
 *	0.  SCC +5 voltage.
 *	1.  SCC +12 voltage.
 *	2.  SCC -12 voltage.
 *	3.  Battery back-up circuit voltage.
 *	4.  Top of SCC temperature.
 *	5.  Bottom of SCC temperature.
 *	6.  Front Panel (or Ambient) temperature.
 *	7.  Peripheral controller power supply +5 voltage.
 *	8.  The difference between the Top of SCC and Ambient temperatures.
 *	9.  The difference between the Bottom of SCC and Top of SCC temps.
 *
 *  The CRQ attention status indicates the following:
 *
 *	ENV_STATUS_WRNING   -	Environmental warning condition only.
 *	ENV_STATUS_VALID    -	Same as ENV_STATUS_WRNING
 *	ENV_STAT_SHTDWN_WRNING-	Indicates that SCC detected a shutdown
 *				condition.  To be confirmed by...
 *	ENV_STATUS_SHTDWN   -	...Shutdown confirmation.  Power going down
 *				soon!  Get ready for a power failure.
 *	ENV_STATUS_CANCEL   -	Can be used after SHTDWN_WRNING, but only
 *				before SHTDWN.
 *	ENV_STATUS_TIMEOUT  -	A/D converter failed to complete conversions
 *				on all channels.
 *	ENV_STATUS_FAULT    -	This indicates a spurious interrupt by the
 *				A/D converter.  Probable hardware failure.
 *
 *  Mask descriptions:
 *
 *  A bit-mask is provided for each of the 4 conditions indicating which of
 *  the 10 voltages or temperatures (listed above) are cause for concern.
 *
 *  Currently the only shutdown conditions that you will ever see are
 *  high temperatures.
 *
 *  For example, a shutdown condition caused by high ambient temperature
 *  will cause bit 6 of the env_hi_shtdwn_mask to be set in the attention
 *  message.
 *
 *  A warning condition caused by low SCC +5 voltage will cause bit 0
 *  of the env_lo_wrning_mask to be set.
 *
 *  Upon receipt of an environmental attention, it is the responsibility
 *  of the host software to:
 *
 *	1.  Use the CRQOP_SCC_GET_AD command to request the latest voltages
 *	    and temperatures from the SCC.  (See scc_msgs.h)
 *	2.  Log all voltages and temperatures into some file for field service.
 *	3.  Output a console message indicating which voltage/temperature(s)
 *	    was causing the attention to be sent.
 *	4.  Prepare for a power shutdown (or at least a system reset) in the
 *	    event a shutdown condition is confirmed.
 *
 *	Include files you will need are:
 *	    scc_attns.h		For the attention message and status.
 *	    scc_msgs.h		CRQ_GET_AD_MSG.
 *	    sccdefs.h		For the definition of NUM_AD.
 *	    crq_opcodes.h	CRQATTN_ENV and CRQOP_SCC_GET_AD.
 */
#define ENV_STATUS_VALID	1
#define ENV_STATUS_WRNING	ENV_STATUS_VALID
#define ENV_STATUS_SHTDWN	2
#define ENV_STATUS_CANCEL	3
#define ENV_STATUS_TIMEOUT	4
#define ENV_STATUS_FAULT	5
#define ENV_STAT_SHTDWN_WRNING	6

/* The SCC sends an ENV_STAT_SHTDWN_WRNING message to the host software
 * indicating that a power shutdown due to environmental conditions may
 * take place.  If within X_TIME, the environment returns to within working
 * tolerances, the SCC will cancel the shutdown (ENV_STATUS_CANCEL).
 * Otherwise, the SCC will confirm the shutdown (ENV_STAT_SHTDWN) and
 * power the system down after Y_TIME has elapsed.  (Y_TIME starts counting
 * only after X_TIME has elapsed for a total of 3 minutes.)
 *
 * Constant warnings (not shutdown warnings, but simple warning conditions)
 * will cause a warning attention to be sent every 30 minutes.  If the
 * condition is fluctuating then everytime a condition changes from good
 * to warning an attention will be sent.
 */
#define X_TIME	(1 * 60 * 1000000)	/* Time is in microseconds.	    */
#define Y_TIME	(2 * 60 * 1000000)
#define C_TIME	(3 * 30 * 1000000)	/* System NMI time for crash dump.  */

typedef struct crq_env {		/* Environmental Sensors.	    */
    crq_msg_t	env_hdr;
    long	env_attn_status;
    short	env_lo_wrning_mask;
    short	env_hi_wrning_mask;
    short	env_lo_shtdwn_mask;
    short	env_hi_shtdwn_mask;
} crq_env_t;

typedef struct crq_mem_csr {		/* Memory CSR scan abnormalities.   */
    crq_msg_t	    mem_csr_hdr;
    long	    mem_csr_slot;
    status_mem_csr_t  mem_csr_csr;
} crq_mem_csr_t;

typedef struct crq_ps_fail {		/* Power Supply failure.	    */
    crq_msg_t	    ps_fail_hdr;
    long	    ps_fail_unit;	/* Power Supply unit number (1,2,3).*/
} crq_ps_fail_t;

typedef struct crq_softerr {		/* Hardware protocol failure.	    */
    crq_msg_t	    softerr_hdr;
    long	    softerr_status;	/* Unused.			    */
} crq_softerr_t;

typedef struct crq_alive {		/* "Are you alive?" attention.	    */
    crq_msg_t	    alive_hdr;
    long	    alive_status;	/* Unused.			    */
} crq_alive_t;
