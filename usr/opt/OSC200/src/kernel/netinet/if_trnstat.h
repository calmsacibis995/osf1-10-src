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
 * @(#)$RCSfile: if_trnstat.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1993/07/13 21:12:38 $
 */

#ifndef _IF_TRNSTAT_H_
#define _IF_TRNSTAT_H_
/*
 * Token Ring characteristics.
 */
struct trnchar {
        u_char  mac_addr[6];		/* The mac address 	*/
        u_char	grp_addr[6];		/* Group address	*/
        u_char	func_addr[6];		/* Functional address	*/
        u_short	drop_numb;		/* Physical drop number */
        u_char	upstream_nbr[6];	/* Upstream neighbor	*/
        u_short	upstream_drop_numb;	/* Upstream drop number */
        u_short transmit_access_pri;	/* Trasmit access priority */
	u_short last_major_vector;	/* Last major vector	*/
	u_short ring_status;		/* Ring status		*/
	u_short	interface_state;	/* State of the interface*/
        u_short monitor_contd;		/* monitor contender   */
	u_short	soft_error_timer;	/* Soft error timer value */
	u_short	ring_number;		/* Local ring number	*/
	u_short monitor_error_code;	/* Monitor error code */
	u_short beacon_receive_type;	/* Type of beacon received */
	u_short beacon_transmit_type;	/* Type of beacon transmitted */
	u_char  beacon_una[6];		/* UNA of the beaconing station */
	u_short beacon_stn_drop_numb;	/* Drop number of the beacon station */
        u_short ring_speed;		/* The ring speed	*/
        u_short etr;			/* Early token release  */
	u_short	open_status;		/* Open status		*/
	u_char  token_ring_chip[16];	/* type of chip		*/
};

/*
 * Definitions for the Major Vector commands 
 * See page 2-30 of the TMS380 book
 */
#define MV_RESPONSE		0x00
#define MV_BEACON		0x02
#define MV_CLAIM_TOKEN		0x03
#define MV_RING_PURGE		0x04
#define MV_ACTIVE_MON_PRES	0x05
#define MV_STANDBY_MON_PRES	0x06
#define MV_DUP_ADDR_TEST	0x07
#define MV_LOBE_MEDIA_TEST	0x08
#define MV_TRANSMIT_FORW	0x09
#define MV_RMV_RING_STATION	0x0B
#define MV_CHANGE_PARM		0x0C
#define MV_INIT_RING_STN	0x0D
#define MV_REQ_STN_ADDR		0x0E
#define MV_REQ_STN_STATE	0x0F
#define MV_REQ_STN_ATTACH	0x10
#define MV_REQ_INIT		0x20
#define MV_REPORT_STN_ADDR	0x22
#define MV_REPORT_STN_STATE	0x23
#define MV_REPORT_STN_ATTACH	0x24
#define MV_REPORT_NEW_MONITOR	0x25
#define MV_REPORT_SUA_CHANGE	0x26
#define MV_REPORT_RNG_POLL_FAIL	0x27
#define MV_REPORT_MONTIOR_ERR	0x28
#define MV_REPORT_ERR		0x29
#define MV_REPORT_TRANSMIT_FORW	0x2A

/*
 * Definitions for the beacon types 
 * See page 2-56 of the TMS380 book
 */
#define BT_SET_RECOV_MODE	0x01
#define BT_SET_SIGNAL_LOSS	0x02
#define BT_SET_BIT_STREAMING	0x03
#define BT_SET_CONT_STREAMING	0x04

/*
 * Definitions for the Monitor Error Code.
 * See page 2-47 of the TMS380 book
 */
#define M_NO_ERROR		0x0000
#define M_MON_ERROR		0x0001
#define M_DUPLICATE_MON		0x0002
#define M_DUPLICATE_ADDR	0x0003

/*
 * Token Ring counters
 */
struct trncount {
        u_long	trn_second;          	/* seconds since last zeroed */
        u_long  trn_bytercvd;           /* bytes received */
        u_long  trn_bytesent;           /* bytes sent */
        u_long  trn_pdurcvd;            /* data blocks received */
        u_long  trn_pdusent;            /* data blocks sent */
        u_long  trn_mbytercvd;          /* multicast bytes received */
	u_long  trn_mpdurcvd;           /* multicast blocks received */
        u_long  trn_mbytesent;          /* multicast bytes sent */
        u_long  trn_mpdusent;           /* multicast blocks sent */
	u_long	trn_pduunrecog;		/* frame unrecognized */
	u_long  trn_mpduunrecog;	/* multicast frame unrecognized */
        u_short	trn_nosysbuf;           /* system buffer unavailable */
	u_short	trn_xmit_fail;		/* xmit failures */
	u_short	trn_xmit_underrun;	/* xmit underruns */
        u_short	trn_line_error;		/* Line errors */
	u_short	trn_internal_error;	/* recoverable internal errors */
        u_short	trn_burst_error;	/* Burst error */
        u_short	trn_ari_fci_error;	/* ARI/FCI error */
	u_short	trn_ad_trans;		/* Abort delimiters transmitted */
        u_short	trn_lost_frame_error;	/* Lost frame error */
        u_short	trn_rcv_congestion_error;/* receive overrun (or congestion) */
        u_short	trn_frame_copied_error;	/* Frame copied error */
	u_short	trn_frequency_error;	/* Frequency error */
        u_short	trn_token_error;	/* Token error */
        u_short	trn_hard_error;		/* Hard errors */
        u_short	trn_soft_error;	        /* Soft errors */
	u_short	trn_adapter_reset;	/* Resets performed */
	u_short	trn_signal_loss;	/* Signal loss */
	u_short	trn_xmit_beacon;	/* Beacons transmitted */
	u_short trn_ring_recovery;	/* Ring recoverys received */
	u_short trn_lobe_wire_fault;	/* Lobe faults detected */
	u_short trn_remove_received;	/* Remove received */
	u_short trn_single_station;	/* # of times host was single */
	u_short trn_selftest_fail;	/* # of times selftest failed */
};

/* 
 * Token Ring RFC 1231 definitions.
 */

struct dot5Entry {
    int dot5TrnNumber;
    int dot5IfIndex;
    int dot5Commands;
    int dot5RingStatus;
    int dot5RingState;
    int dot5RingOpenStatus;
    int dot5RingSpeed;
    u_char dot5UpStream[6];
    int dot5ActMonParticipate;
    u_char dot5Functional[6];
};

/* Values for dot5Commands */
#define MIB1231_COMM_NO_OP	1
#define MIB1231_COMM_OPEN	2
#define MIB1231_COMM_RESET	3
#define	MIB1231_COMM_CLOSE	4

/* Values for dot5RingStatus */
#define MIB1231_RSTATUS_NO_PROB		0x0000
#define MIB1231_RSTATUS_RING_RECOVERY	0x0020	/* 32 */
#define MIB1231_RSTATUS_SINGLE_STATION	0x0040	/* 64 */
#define	MIB1231_RSTATUS_REMOVE_RCVD	0x0100	/* 256 */
#define	MIB1231_RSTATUS_RESERVED	0x0200	/* 512 */
#define	MIB1231_RSTATUS_AUTO_REM_ERROR	0x0400	/* 1024 */
#define	MIB1231_RSTATUS_LOBE_WIRE_FAULT	0x0800	/* 2048 */
#define	MIB1231_RSTATUS_TRANSMIT_BEACON	0x1000	/* 4096 */
#define	MIB1231_RSTATUS_SOFT_ERROR	0x2000	/* 8192 */
#define	MIB1231_RSTATUS_HARD_ERROR	0x4000	/* 16384 */
#define	MIB1231_RSTATUS_SIGNAL_LOSS	0x8000	/* 32768 */
#define	MIB1231_RSTATUS_NO_STATUS	0x20000 /* 131072 */

/* Values for dot5RingState */
#define MIB1231_RSTATE_OPENED			1
#define MIB1231_RSTATE_CLOSED			2
#define MIB1231_RSTATE_OPENING			3
#define MIB1231_RSTATE_CLOSING			4
#define MIB1231_RSTATE_OPEN_FAILURE		5
#define MIB1231_RSTATE_RING_FAILURE		6

/* Values for dot5RingStatus */
#define MIB1231_ROSTATUS_NOOPEN		1
#define MIB1231_ROSTATUS_BADPARM	2
#define MIB1231_ROSTATUS_LOBEFAILED	3
#define MIB1231_ROSTATUS_SIG_LOSS	4
#define MIB1231_ROSTATUS_INS_TIMEOUT	5
#define MIB1231_ROSTATUS_RING_FAILED	6
#define MIB1231_ROSTATUS_BEACONING	7
#define MIB1231_ROSTATUS_DUPLICATE_MAC	8
#define MIB1231_ROSTATUS_REQ_FAILED	9
#define MIB1231_ROSTATUS_REM_RECVD	10
#define MIB1231_ROSTATUS_OPEN		11

/* Values for dot5RingSpeed */
#define MIB1231_RSPEED_UNKNOWN		1
#define MIB1231_RSPEED_1_MEG		2
#define MIB1231_RSPEED_4_MEG		3
#define MIB1231_RSPEED_16_MEG		4

/* Values for dot5ActMonParticipate */
#define MIB1231_ACTMON_TRUE		1
#define MIB1231_ACTMON_FALSE		2

/* The statistics table */

struct dot5StatsEntry {
    int dot5StatsIfIndex;
    int dot5StatsLineErrors;
    int dot5StatsBurstErrors;
    int dot5StatsACErrors;
    int dot5StatsAbortTransErrors;
    int dot5StatsInternalErrors;
    int dot5StatsLostFrameErrors;
    int dot5StatsReceiveCongestions;
    int dot5StatsFrameCopiedErrors;
    int dot5StatsTokenErrors;
    int dot5StatsSoftErrors;
    int dot5StatsHardErrors;
    int dot5StatsSignalLoss;
    int dot5StatsTransmitBeacons;
    int dot5StatsRecoverys;
    int dot5StatsLobeWires;
    int dot5StatsRemoves;
    int dot5StatsSingles;
    int dot5StatsFreqErrors;
};

/* The timer table - optional */

struct dot5TimerEntry {
    int dot5TimerIfIndex;
    int dot5TimerReturnRepeat;
    int dot5TimerHolding;
    int dot5TimerQueuePDU;
    int dot5TimerValidTransmit;
    int dot5TimerNoToken;
    int dot5TimerActiveMon;
    int dot5TimerStandbyMon;
    int dot5TimerErrorReport;
    int dot5TimerBeaconTransmit;
    int dot5TimerBeaconReceive;
};

#endif
