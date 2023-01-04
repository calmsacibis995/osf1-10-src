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
 * static char *rcsid = "@(#)$RCSfile: if_pdqreg.h,v $ $Revision: 1.1.3.8 $ (DEC) $Date: 1992/10/15 13:09:45 $";
 */

#ifndef _PORT_
#define _PORT_
/*
* Module Name:
*   FUNC_PORT
*
* File Name:
*   port.h
*   
* Abstract:
*   Contains all definitions specified by port specification.  This file is 
*   designed to be shared by driver and adapter programs.  For more information
*   on the definitions in this module, refer to the port specification.
*
*   All arrays are of a numeric (as opposed to symbolic) length because of a 
*   post-processor restriction.
*
* Maintainers:
*   DMW		Douglas M. Washabaugh
*   EM		Evelyn Moy
*   RLK		Richard L. Kirk
*   LVS		Lawrence V. Stefani
*   VK          Venkat Kalkunte
*
* Modification History:
*   Date        Name    Description 
*   07-Jan-91	DMW	Created.
*   18-Feb-91	EM	Added PORT_K_EVENT_HOST_FAULT and 
*                       PORT_K_EVENT_ADAPT_PARITY.  Added error log types.
*   14-Jun-91	DMW	Updated for version 6.0 of port spec.
*			Add LEM codes, changed name of some filters.
*			Added:  PI_STATE_K_NUMBER
*				PI_PCTRL_M_INIT_START
*				PI_PCTRL_M_FW_REV_READ
*				PI_PDATA_A_RESET_M_UPGRADE
*				PI_PDATA_A_RESET_M_SOFT_RESET
*				PI_TYPE_2_PROD_*
*				Removed PI_PCTRL_M_MLA_*
*   01-Jul-91	RLK	Added interrupt bit definitions
*   18-Sep-91	DMW	Removed some stuff to FDDI.H, added stuff for testing
*                       read_status.
*   08-Oct-91	DMW	Added phy constants.
*   16-Oct-91	DMW	Added cntrs_set command.
*			Added PI_FSTATE_K_BLOCK.
*			Added PI_FSTATE_K_PASS.
*			Added PI_ITEM_K_IND_GROUP_PROM_DEF
*			Added PI_ITEM_K_GROUP_PROM_DEF 
*			Added PI_ITEM_K_SMT_PROM_DEF
*			Added PI_ITEM_K_SMT_USER_DEF
*			Added PI_ITEM_K_RESERVED_DEF
*			Added PI_ITEM_K_IMPLEMENTOR_DEF
*			Added PI_ITEM_K_BROADCAST_DEF
*			Added PI_DMA_K_TIMEOUT
*                       Added PI_PCTRL_K_TIMEOUT
*			Added PI_COMPL_M_RCV_INDEX
*			Added PI_COMPL_M_XMT_INDEX
*			Removed PI_LOG_ENTRY_K_INDEX_MAX
*			Removed link counter, not used
*			Changed set_char_max, use item_max
*                       Moved device specific info to new cmd
*                       Added SNMP support
*   07-Jan-92   DMW     Incorporated FDDI.H [kept names same]
*                       Renamed COMPLETION_BLOCK to CONSUMER
*                       Renamed PI_COMPL_M_RCV_INDEX to CONS
*                       Renamed PI_PCTRL_M_COMP_BLOCK to CONS
*                       Renamed PI_TYPE_1_PROD_V_SERVICE to COMP
*                       Renamed PI_TYPE_2_PROD_V_XMT_DATA_SERV
*   08-Jan-92   LVS     Changed the following object names
*                       in PI_CMD_FDDI_MIB_GET_RSP:
*                       	smt_notify  =>  smt_t_notify
*                       	mac_tmax_greatest_lower_bound  =>
*                       	mac_t_max_greatest_lower_bound
*                       	mac_treq  =>  mac_t_req
*                       	mac_tneg  =>  mac_t_neg
*                       Changed the following object name
*                       in PI_CMD_DEC_EXT_MIB_GET_RSP:
*                       	mgmt_sets_allowed_switch  =>
*                       	eif_mgmt_sets_allowed_switch
*                       and removed ifTable group counters from same structure.
*   16-Jan-92   DMW     Cleaned up unsolicited events.
*   16-Jan-92   LVS     Added the following parameters to the Mod Chars 
*                       parameter table:
*                               PI_ITEM_K_TNOTIFY
*                               PI_ITEM_K_MAC_LOOP_TIME
*                               PI_ITEM_K_TBMAX
*                               PI_ITEM_K_INSERT_POLICY
*                       Also added appropriate min, max, default values.  
*                       Updated min/max values of some timer parameters.  
*                       Added the following unsolicited station entity event:  
*                               PI_UNSOL_STAT_K_CONFIG_CHANGE
*   17-Jan-92   DMW     Added padding to FDDI frame header
*   29-Jan-92   LVS     Added the following parameter defaults:
*                               PI_ITEM_K_LEM_THRESHOLD_DEF
*                               PI_ITEM_K_INSERT_POLICY_DEF
*                       Modified PI_UNSOL_REPORT structure, added new event 
*                       argument structures:
*                               PI_UNSOL_ARG_REASON_DESC
*                               PI_UNSOL_ARG_DL_HEADER_DESC
*                               PI_UNSOL_ARG_SOURCE_DESC
*                               PI_UNSOL_ARG_UPSTREAM_NBR_DESC
*                               PI_UNSOL_ARG_DIRECTION_DESC
*   17-Feb-92   LVS     Added the following parameters to the Mod Chars 
*                       parameter table:
*                               PI_ITEM_K_SMT_VERSION_ID
*                               PI_ITEM_K_CONFIG_POLICY
*                               PI_ITEM_K_CONNECTION_POLICY
*                               PI_ITEM_K_STATION_ACTION
*                               PI_ITEM_K_TMAX_GREATEST_LB
*                               PI_ITEM_K_MAC_PATHS_REQUESTED
*                               PI_ITEM_K_FRAME_STATUS
*                               PI_ITEM_K_MAC_ACTION
*                               PI_ITEM_K_CONNECTION_POLICIES
*                               PI_ITEM_K_PORT_PATHS_REQUESTED
*                               PI_ITEM_K_LER_CUTOFF
*                               PI_ITEM_K_LER_ALARM
*                               PI_ITEM_K_PORT_ACTION
*                       Renamed the following item codes:
*                               PI_ITEM_K_TREQ => PI_ITEM_K_T_REQ
*                               PI_ITEM_K_TNOTIFY => PI_ITEM_K_T_NOTIFY
*                               PI_ITEM_K_TBMAX => PI_ITEM_K_TB_MAX
*                       to more closely follow CNS obj names.  Also added 
*                       appropriate min, max, default values.  Updated min/max 
*                       values of some timer parameters and item codes to 
*                       match Port Spec.  Added the unsolicited argument code
*                       tables for RIReason and LCTDirection and updated 
*                       dl_header arg data in PI_UNSOL_ARG_DL_HEADER_DESC
*                       Added the error log code table for Event Header Caller
*                       Id.
*   20-Feb-91   DMW     Changed format of char_set so that an index may be 
*                       provided. Removed all dependencies on COMMON.H so
*                       it no longer needs to be included prior to this module.
*                       Cleaned up format of Error_Get response.
*                       Removed time_since_reset from FDDI_MIB and DEC_EXT_MIB
*                       responses.
*   24-Feb-92   DMW     Changed FDDI_HEADER for longword fc.
*   03-Mar-92   DMW     Beefed up descriptor fields (adds only) fixed RCC mask.
*   04-Mar-92   DMW     Swapped xmt, rcv of:
*                        	PI_TYPE_X_M_RCV_DATA_ENB
*                               PI_TYPE_X_M_HOST_SMT_ENB and unsol
*                               Fix PI_CONS_M_XMT_INDEX.
*                               Fix PI_RCV_DESCR_M_SEG_LEN
*                               Add descr block and consumer block
*   26-Mar-92   DMW     Fixed the FDDI_HEADER to include the packet request
*                       header bytes.
*   31-Mar-92   DMW     Shortened names:
*                        	PI_ITEM_K_MAC_PATHS_REQUESTED =>
*				PI_ITEM_K_MAC_PATHS_REQ
*				PI_ITEM_K_CONNECTION =>
*				PI_ITEM_K_CON
*                               PI_ITEM_K_PORT_PATHS_REQUESTED =>
*				PI_ITEM_K_PORT_PATHS_REQ
*				attachment_optical_bypass =>
*				attachment_ob
*				emac_upstream_nbr_dupl_address_flag =>
*				emac_up_nbr_dup_addr_flag
*				PI_UNSOL_LINK_K_FRAME_STATUS_ERR =>
*				PI_UNSOL_LINK_K_FRAME_STAT_ERR
*				PI_UNSOL_LINK_ARG_K_UPSTREAM_NBR =>
*				PI_UNSOL_LINK_ARG_K_UP_NBR
*				PI_UNSOL_RI_REASON =>
*				PI_UNSOL_RI_RSN
*				PI_HOST_INT_0_M_HOST_BUS_PAR_ERR =>
*				PI_HOST_INT_0_M_BUS_PAR_ERR
*				Changed PI_UNSOL_ARG_SOURCE_DESC and 
*				PI_UNSOL_ARG_UPSTREAM_NBR_DESC to 
*				PI_UNSOL_ARG_NET_ADDR_DESC
*   31-Mar-92   LVS     Added FDDI Data Link Specification Data Types.  Grouped
*                       constants better, updated or added comments.
*                       Shortened names:
*                               PI_UNSOL_RI_RSN_K =>  PI_RI_RSN_K
*                               PI_UNSOL_LCT_DIRECTION_K => PI_LCT_DIRECTION_K
*   08-Apr-92   DMW     Burst size no device specific, use 
*                       PI_PCTRL_M_DEV_SPECIFIC.
*   09-Apr-92   DMW	Added PI_PCTRL_M_SUB_CMD to port control reg definition.
*   10-Apr-92   DMW     Added kongs to counter block.  Added the constants
*                       PI_RSP_K_FLUSH_ENB_BAD and PI_ITEM_K_FLUSH_ENB.
*   13-Apr-92   DMW     Added PI_PDATA_A_RESET_M_SKIP_ST.
*   21-Apr-92   DMW     Fixed name of sub_cmd uninit.
*                       added sub-cmd for setting the burst size.
*   21-Apr-92   LVS     Removed eif_mgmt_sets_allowed_switch field from
*                       PI_CMD_DEC_EXT_MIB_GET_RSP structure per Anil R.'s
*                       recommendation.
*   22-Apr-92   DMW     Changed flush_enb to flush_time for more versatility.
*                       FLUSH_ENB => FLUSH_TIME for constants.
*   28-Apr-92   DMW     Changed default flush time from 2 to 3 so that link
*                       will go unavailable before the flush interrupt is 
*                       generated.
*   30-Apr-92   DMW     Changed name of PI_HALT_ID_K_HOST_BUS_PARITY to 
*                       PI_HALT_ID_K_PARITY_ERROR because the error is in
*                       packet memory (host or adapter cpu could have caused
*                       the error). 
*                       
*                       Changed name of PI_HOST_INT_0_M_LLC_RCV_EMPTY to
*                       PI_HOST_INT_0_M_20MS because the interrupt is delivered
*                       regardless of whether the queue is empty.
*   10-May-92   DMW     Updated item codes.
*   15-May-92   LVS     Changed size of entry_data in PI_UNSOL_REPORT from
*                       75 PI_ULONGs to 20 due to SCO Unix Stream Head not
*                       allowing ioctl calls to send buffers greater than 4K.
*                       [ Old sizeof(PI_UNSOL_REPORT) * PI_UNSOL_ENTRIES > 4K ]
*
*                       Removed the following PI_ITEM constants:
*                               PI_ITEM_K_SMT_VERSION_ID_DEF
*                               PI_ITEM_K_T_MAX_GREATEST_LB_??? (MIN,MAX,DEF)
*                               PI_ITEM_K_FRAME_STATUS_DEF
*                               PI_ITEM_K_INSERT_POLICY_DEF
*                       Updated the following PI_ITEM constants:
*                               PI_ITEM_K_T_NOTIFY_DEF from 10 to 30 secs.
*                               PI_ITEM_K_TB_MAX_DEF from 30 to 50 ms.
*                       Added the following constants:
*                               PI_SNMP_K_TRUE
*                               PI_SNMP_K_FALSE                   
*                               PI_RING_PURGER_K_DEF
*   18-May-92   DMW     Added PI_RSP_K_MAC_PATHS_REQ_BAD, 
*                               PI_ITEM_K_MAC_T_REQ_MIN,
*                               PI_ITEM_K_MAC_T_REQ_MAX,
*                               PI_ITEM_K_MAC_T_REQ_DEF,
*                               PI_ITEM_K_EMAC_RING_PURGER_DEF,
*                               PI_ITEM_K_EMAC_RTOKEN_TIMEOUT_MIN,
*                               PI_ITEM_K_EMAC_RTOKEN_TIMEOUT_MAX,
*                               PI_ITEM_K_EMAC_RTOKEN_TIMEOUT_DEF,
*                               PI_UNSOL_EVENT_K_DATA_SIZE,
*                               PI_LOG_CALLER_ID_K_CONSOLE
*                       Added some FCs and removed the base FC, it wasn't right.
*                       Added PI_HALT_ID_K_IMAGE_CRC_ERROR.
*   19-May-92   LVS     Shortened names of the following item values:
*                               PI_ITEM_K_EMAC_RTOKEN_TIMEOUT_MIN =>
*                               PI_ITEM_K_EMAC_RTOKEN_TIME_MIN
*                               PI_ITEM_K_EMAC_RTOKEN_TIMEOUT_MAX =>
*                               PI_ITEM_K_EMAC_RTOKEN_TIME_MAX
*                               PI_ITEM_K_EMAC_RTOKEN_TIMEOUT_DEF =>
*                               PI_ITEM_K_EMAC_RTOKEN_TIME_DEF
*   20-May-92   DMW     Shortened PI_RSP_K_EMAC_RTOKEN_TIMEOUT_BAD to 
*                               PI_RSP_K_EMAC_RTOKEN_TIME_BAD
*                       Updated caller id's.
*   21-May-92   DMW     Removed PI_CMD_K_STATUS_CHARS_TEST, moved SNMP_SET to
*                       its place.
*                       Changed ring purger default to off.
*   22-May-92   DMW     Changed PI_CMD_SNMP_SET_K_ITEMS_MAX to a lower value.
*   25-May-92   DMW     Added PI_K_FALSE and PI_K_TRUE.
*   26-May-92   DMW     Fixed values of:
*                               PI_CMD_FILTERS_SET_K_ITEMS_MAX
*                               PI_CMD_SNMP_SET_K_ITEMS_MAX
*   26-May-92   LVS     Updated PI_ITEM_K_EMAC_RING_PURGER_DEF constant.
*                       Deleted redundant PI_RING_PURGER_K_DEF constant.
*   27-May-92   DMW     Changed PI_ITEM_K_FLUSH_TIME_DEF to 0 (off).  Added
*                       PI_ITEM_K_FLUSH_TIME_REC for recommended value if flush
*                       is enabled.
*   28-May-92   DMW     Fixed PI_ITEM_K_T_REQ_MAX.
*   03-Jun-92   LVS     Added the following typedefs to PI_UNSOL_REPORT struct:
*                               PI_UNSOL_EVENT_CNTRS
*                               PI_UNSOL_EVENT_REPORT
*   25-Jun-92   VK      Fixed UNSOL and SMT masks (they were reversed).
*   26-Jun-92   LVS     Changed PI_RING_ERR_RSN_K_RING_OP =>
*                               PI_RING_ERR_RSN_K_RING_OP_OSC
*                       Added PI_RING_ERR_RSN_K_RING_OP_DEL.
*   24-Sep-92   VK	Added ANSI SMT 7.2 MIB item codes.
*   24-Sep-92   VK	Added command PI_CMD_K_SMT_MIB_GET to fetch SMT 7.2 
*			objects.  PI_CMD_SMT_MIB_GET_RSP structure is 
*			then filled with response. 
*   25-Sep-92   VK	Added command PI_CMD_K_SMT_MIB_SET to set 7.2 writeable
*		        objects.  See SMT 7.2 spec or port spec for the list of
*			writeable objects.
*   25-Sep-92   VK	Added two SMT 7.2 MAC counters, copied_cnt and 
*			transmit_cnt.  To remain backward compatible, these
*		        counters are returned at bottom of PI_CNTR_BLK.
*   25-Sep-92   VK      Added item code to allow driver to enable/disable 
*			full duplex mode through SNMP_SET command.
*   25-Sep-92   VK      Added alignment constants.
*   30-Sep-92   VK	Added default packet request header bytes.
*   01-Oct-92   LVS     Added remaining SMT 7.2 mandatory item and group
*                       codes.
*   
*/

/* Define types */
typedef u_char FDDI_NET_ADDR[6];
typedef u_long PI_ULONG;
typedef u_int PI_UINT;
typedef u_char PI_UCHAR;

/* Define maximum and minimum legal sizes for frame types */

#define FDDI_LLC_K_LENGTH_MIN		20
#define FDDI_LLC_K_LENGTH_MAX		4495
#define FDDI_SMT_K_LENGTH_MIN		33
#define FDDI_SMT_K_LENGTH_MAX		4494
#define FDDI_MAC_K_LENGTH_MIN		18
#define FDDI_MAC_K_LENGTH_MAX		4494

/* Define FC's */

#define FDDI_FC_K_VOID			0X00	
#define FDDI_FC_K_NON_RESTRICTED_TOKEN	0X80	
#define FDDI_FC_K_RESTRICTED_TOKEN	0XC0	
#define FDDI_FC_K_SMT_MIN		0X41
#define FDDI_FC_K_SMT_MAX               0X4F
#define FDDI_FC_K_MAC_MIN		0XC1
#define FDDI_FC_K_MAC_MAX               0XCF	
#define FDDI_FC_K_ASYNC_LLC_MIN		0X50
#define FDDI_FC_K_ASYNC_LLC_MAX		0X5F
#define FDDI_FC_K_SYNC_LLC_MIN		0XD0
#define FDDI_FC_K_SYNC_LLC_MAX		0XD7
#define FDDI_FC_K_IMPLEMENTOR_MIN	0X60
#define FDDI_FC_K_IMPLEMENTOR_MAX       0X6F
#define FDDI_FC_K_RESERVED_MIN		0X70
#define FDDI_FC_K_RESERVED_MAX		0X7F

#define FDDI_K_SMT_XID_APP		0X80000000
#define FDDI_K_SMT_XID_ENT		0X00000000

#define FDDI_FRAME_K_SMT_TYPE_RSP	0X01
#define FDDI_FRAME_K_SMT_TYPE_NO_RSP	0X00

typedef struct                               /* FDDI frame header */
    {
    unsigned char   prh_0;
    unsigned char   prh_1;
    unsigned char   prh_2;
    unsigned char   fc;
    FDDI_NET_ADDR   dst_addr;
    FDDI_NET_ADDR   src_addr;
    } FDDI_HEADER;

typedef struct			             /* General FDDI frame */
    {
    FDDI_HEADER	    header;
    unsigned char   info[4500];
    } FDDI_FRAME_GEN;

/* Define general structures */
typedef struct                               /* 64-bit counter */
    {
    PI_UINT  ms;
    PI_UINT  ls;
    } PI_CNTR;

typedef struct                               /* LAN address */
    {                                        
    PI_UINT  lwrd_0;
    PI_UINT  lwrd_1;
    } PI_LAN_ADDR;

typedef struct                               /* Station ID address */
    {
    PI_UINT  octet_7_4;
    PI_UINT  octet_3_0;
    } PI_STATION_ID;

/* SMT USER_DATA structure 32 bytes long */

typedef struct
    {
    PI_UINT    lw_1;
    PI_UINT    lw_2;
    PI_UINT    lw_3;
    PI_UINT    lw_4;
    PI_UINT    lw_5;
    PI_UINT    lw_6;
    PI_UINT    lw_7;
    PI_UINT    lw_8;
    } PI_USER_DATA;


/* Define general constants */

#define PI_ALIGN_K_DESC_BLK      8192    /* Descriptor block boundary        */
#define PI_ALIGN_K_CONS_BLK      64      /* Consumer block boundary          */
#define PI_ALIGN_K_CMD_BUFF      128     /* Xmt Command que buffer alignment */
#define PI_ALIGN_K_CMD_BUFF	 128     /* Rcv Command que buffer alignment */
#define PI_ALIGN_K_UNSOL_BUFF    128     /* Unsol que buffer alignment       */
#define PI_ALIGN_K_XMT_DATA_BUFF 0       /* Xmt data que buffer alignment    */
#define PI_ALIGN_K_RCV_DATA_BUFF 128     /* Rcv que buffer alignment         */

/* Define default packet request bytes */

#define PI_PRH_K_BYTE_0_DEF      0x20              /* Refer to Port Spec */
#define PI_PRH_K_BYTE_1_DEF      0x28              /* Refer to Port Spec */
#define PI_PRH_K_BYTE_2_DEF      0x00              /* Refer to Port Spec */

#define PI_DMA_K_TIMEOUT		    2000    /* Milliseconds */

#define PI_PHY_K_S			    0	    /* Index to S phy */
#define PI_PHY_K_A			    0	    /* Index to A phy */
#define PI_PHY_K_B			    1	    /* Index to B phy */
#define PI_PHY_K_MAX			    2	    /* Max number of phys */

/* Define FDDI Data Link Functional Specification Data Types */

#define PI_STATION_TYPE_K_SAS		    0       /* Station Type */
#define PI_STATION_TYPE_K_DAC		    1
#define PI_STATION_TYPE_K_SAC		    2
#define PI_STATION_TYPE_K_NAC		    3
#define PI_STATION_TYPE_K_DAS		    4

#define PI_STATION_STATE_K_OFF		    0       /* Station State */
#define PI_STATION_STATE_K_ON		    1
#define PI_STATION_STATE_K_LOOPBACK	    2

#define PI_LINK_STATE_K_OFF_READY           1       /* Link State */
#define PI_LINK_STATE_K_OFF_FAULT           2
#define PI_LINK_STATE_K_ON_RING_INIT        3
#define PI_LINK_STATE_K_ON_RING_RUN         4
#define PI_LINK_STATE_K_BROKEN              5

#define PI_DA_TEST_STATE_K_UNKNOWN          0       /* Dupl Addr Test State */
#define PI_DA_TEST_STATE_K_SUCCESS          1
#define PI_DA_TEST_STATE_K_DUPLICATE        2

#define PI_RP_STATE_K_OFF                   0       /* Ring Purger State */
#define PI_RP_STATE_K_CANDIDATE             1
#define PI_RP_STATE_K_NON_PURGER            2
#define PI_RP_STATE_K_PURGER                3

#define PI_FS_MODE_K_SA_MATCH               0       /* Frame Strip Mode */
#define PI_FS_MODE_K_FCI_STRIP              1

#define PI_RING_ERR_RSN_K_NO_ERROR          0       /* Ring Error Reason */
#define PI_RING_ERR_RSN_K_INIT_INIT	    5
#define PI_RING_ERR_RSN_K_INIT_RCVD	    6
#define PI_RING_ERR_RSN_K_BEACON_INIT	    7
#define PI_RING_ERR_RSN_K_DUP_ADDR	    8
#define PI_RING_ERR_RSN_K_DUP_TOKEN	    9
#define PI_RING_ERR_RSN_K_PURGE_ERROR	    10
#define PI_RING_ERR_RSN_K_FCI_ERROR	    11
#define PI_RING_ERR_RSN_K_RING_OP_OSC	    12
#define PI_RING_ERR_RSN_K_DIR_BEACON	    13
#define PI_RING_ERR_RSN_K_TRACE_INIT	    14
#define PI_RING_ERR_RSN_K_TRACE_RCVD	    15
#define PI_RING_ERR_RSN_K_RING_OP_DEL       29

#define PI_STATION_MODE_K_NORMAL            0       /* Station Mode */
#define PI_STATION_MODE_K_LOOPBACK          1

#define PI_PHY_TYPE_K_A                     0       /* PHY Type */
#define PI_PHY_TYPE_K_B                     1
#define PI_PHY_TYPE_K_S                     2
#define PI_PHY_TYPE_K_M                     3
#define PI_PHY_TYPE_K_UNKNOWN               4

#define PI_PMD_TYPE_K_ANSI_MULTI            0       /* PMD Type */
#define PI_PMD_TYPE_K_ANSI_SINGLE_1         1
#define PI_PMD_TYPE_K_ANSI_SINGLE_2         2
#define PI_PMD_TYPE_K_ANSI_SONET            3
#define PI_PMD_TYPE_K_LOW_POWER             100
#define PI_PMD_TYPE_K_THIN_WIRE             101
#define PI_PMD_TYPE_K_SHIELD_TWISTED        102
#define PI_PMD_TYPE_K_UNSHIELD_TWISTED      103

#define PI_PHY_STATE_K_INT_LOOPBACK         0       /* PHY State */
#define PI_PHY_STATE_K_BROKEN               1
#define PI_PHY_STATE_K_OFF_READY            2
#define PI_PHY_STATE_K_WAITING              3
#define PI_PHY_STATE_K_STARTING             4
#define PI_PHY_STATE_K_FAILED               5
#define PI_PHY_STATE_K_WATCH                6
#define PI_PHY_STATE_K_IN_USE               7

#define PI_REJECT_RSN_K_NONE                0       /* Reject Reason */
#define PI_REJECT_RSN_K_LOCAL_LCT           1
#define PI_REJECT_RSN_K_REMOTE_LCT          2
#define PI_REJECT_RSN_K_LCT_BOTH            3
#define PI_REJECT_RSN_K_LEM_REJECT          4
#define PI_REJECT_RSN_K_TOPOLOGY_ERROR      5
#define PI_REJECT_RSN_K_NOISE_REJECT        6
#define PI_REJECT_RSN_K_REMOTE_REJECT       7
#define PI_REJECT_RSN_K_TRACE_IN            8
#define PI_REJECT_RSN_K_TRACE_RCVD_DIS      9
#define PI_REJECT_RSN_K_STANDBY             10
#define PI_REJECT_RSN_K_PROTO_ERROR         11

#define PI_RI_RSN_K_TVX_EXPIRED             0       /* RI Reason */
#define PI_RI_RSN_K_TRT_EXPIRED             1
#define PI_RI_RSN_K_RING_PURGER             2
#define PI_RI_RSN_K_PURGE_ERROR             3
#define PI_RI_RSN_K_TOKEN_TIMEOUT           4

#define PI_LCT_DIRECTION_K_LOCAL            1       /* LCT Direction */
#define PI_LCT_DIRECTION_K_REMOTE           2
#define PI_LCT_DIRECTION_K_BOTH             3

/* Define FMC descriptor fields */

#define PI_FMC_DESCR_V_SOP		    31
#define PI_FMC_DESCR_V_EOP		    30
#define PI_FMC_DESCR_V_FSC		    27
#define PI_FMC_DESCR_V_FSB_ERROR	    26
#define PI_FMC_DESCR_V_FSB_ADDR_RECOG	    25
#define PI_FMC_DESCR_V_FSB_ADDR_COPIED	    24
#define PI_FMC_DESCR_V_FSB		    22
#define PI_FMC_DESCR_V_RCC_FLUSH	    21
#define PI_FMC_DESCR_V_RCC_CRC		    20
#define PI_FMC_DESCR_V_RCC_RRR		    17
#define PI_FMC_DESCR_V_RCC_DD               15
#define PI_FMC_DESCR_V_RCC_SS               13
#define PI_FMC_DESCR_V_RCC		    13
#define PI_FMC_DESCR_V_LEN		    0

#define PI_FMC_DESCR_M_SOP		    0X80000000
#define PI_FMC_DESCR_M_EOP		    0X40000000
#define PI_FMC_DESCR_M_FSC		    0X38000000
#define PI_FMC_DESCR_M_FSB_ERROR	    0X04000000
#define PI_FMC_DESCR_M_FSB_ADDR_RECOG	    0X02000000
#define PI_FMC_DESCR_M_FSB_ADDR_COPIED	    0X01000000
#define PI_FMC_DESCR_M_FSB		    0X07C00000
#define PI_FMC_DESCR_M_RCC_FLUSH	    0X00200000
#define PI_FMC_DESCR_M_RCC_CRC		    0X00100000
#define PI_FMC_DESCR_M_RCC_RRR		    0X000E0000
#define PI_FMC_DESCR_M_RCC_DD               0X00018000
#define PI_FMC_DESCR_M_RCC_SS               0X00006000
#define PI_FMC_DESCR_M_RCC		    0X003FE000
#define PI_FMC_DESCR_M_LEN		    0X00001FFF

#define PI_FMC_DESCR_K_RCC_FMC_INT_ERR	    0X01AA

#define PI_FMC_DESCR_K_RRR_SUCCESS	    0X00
#define PI_FMC_DESCR_K_RRR_SA_MATCH	    0X01
#define PI_FMC_DESCR_K_RRR_DA_MATCH	    0X02
#define PI_FMC_DESCR_K_RRR_FMC_ABORT	    0X03
#define PI_FMC_DESCR_K_RRR_LENGTH_BAD	    0X04
#define PI_FMC_DESCR_K_RRR_FRAGMENT	    0X05
#define PI_FMC_DESCR_K_RRR_FORMAT_ERR	    0X06
#define PI_FMC_DESCR_K_RRR_MAC_RESET	    0X07

#define PI_FMC_DESCR_K_DD_NO_MATCH          0X0
#define PI_FMC_DESCR_K_DD_PROMISCUOUS       0X1
#define PI_FMC_DESCR_K_DD_CAM_MATCH         0X2
#define PI_FMC_DESCR_K_DD_LOCAL_MATCH       0X3

#define PI_FMC_DESCR_K_SS_NO_MATCH          0X0
#define PI_FMC_DESCR_K_SS_BRIDGE_MATCH      0X1
#define PI_FMC_DESCR_K_SS_NOT_POSSIBLE      0X2
#define PI_FMC_DESCR_K_SS_LOCAL_MATCH       0X3

/* Define some max buffer sizes */

#define PI_CMD_REQ_K_SIZE_MAX		    512
#define PI_CMD_RSP_K_SIZE_MAX		    512
#define PI_UNSOL_K_SIZE_MAX		    512
#define PI_SMT_HOST_K_SIZE_MAX		    4608		/* 4 1/2 K */
#define PI_RCV_DATA_K_SIZE_MAX		    4608		/* 4 1/2 K */
#define PI_XMT_DATA_K_SIZE_MAX		    4608		/* 4 1/2 K */

/* Define general frame formats */

typedef struct
    {
    FDDI_FRAME_GEN  frame;			    /* Regular FDDI frame */
    } PI_RCV_FRAME;

/* Define adapter states */

#define PI_STATE_K_RESET	    0
#define PI_STATE_K_UPGRADE          1
#define PI_STATE_K_DMA_UNAVAIL	    2
#define PI_STATE_K_DMA_AVAIL	    3
#define PI_STATE_K_LINK_AVAIL	    4
#define PI_STATE_K_LINK_UNAVAIL     5
#define PI_STATE_K_HALTED           6
#define PI_STATE_K_RING_MEMBER      7
#define PI_STATE_K_NUMBER	    8

/* Define codes for command type */

#define PI_CMD_K_START			    0x00
#define PI_CMD_K_FILTERS_SET		    0x01
#define PI_CMD_K_FILTERS_GET	            0x02
#define PI_CMD_K_CHARS_SET	            0x03
#define PI_CMD_K_STATUS_CHARS_GET	    0x04
#define PI_CMD_K_CNTRS_GET		    0x05
#define PI_CMD_K_CNTRS_SET		    0x06
#define PI_CMD_K_ADDR_FILTER_SET	    0x07
#define PI_CMD_K_ADDR_FILTER_GET	    0x08
#define PI_CMD_K_ERROR_LOG_CLEAR	    0x09
#define PI_CMD_K_ERROR_LOG_GET		    0x0A
#define PI_CMD_K_FDDI_MIB_GET               0x0B
#define PI_CMD_K_DEC_EXT_MIB_GET            0x0C
#define PI_CMD_K_DEVICE_SPECIFIC_GET        0x0D
#define PI_CMD_K_SNMP_SET		    0x0E
#define PI_CMD_K_UNSOL_TEST                 0x0F
#define PI_CMD_K_SMT_MIB_GET		    0x10
#define PI_CMD_K_SMT_MIB_SET		    0x11
#define PI_CMD_K_MAX			    0x11	/* Must match last */

/* Define item codes for Chars_Set and Filters_Set commands */

#define PI_ITEM_K_EOL                       0x00 /* End-of-Item list */
#define PI_ITEM_K_T_REQ                     0x01 /* DECnet T_REQ */
#define PI_ITEM_K_TVX                       0x02 /* DECnet TVX */
#define PI_ITEM_K_RESTRICTED_TOKEN          0x03 /* DECnet Restricted Token */     
#define PI_ITEM_K_LEM_THRESHOLD             0x04 /* DECnet LEM Threshold */
#define PI_ITEM_K_RING_PURGER               0x05 /* DECnet Ring Purger Enable */    
#define PI_ITEM_K_CNTR_INTERVAL             0x06 /* Chars_Set */
#define PI_ITEM_K_IND_GROUP_PROM            0x07 /* Filters_Set */
#define PI_ITEM_K_GROUP_PROM                0x08 /* Filters_Set */
#define PI_ITEM_K_BROADCAST                 0x09 /* Filters_Set */
#define PI_ITEM_K_SMT_PROM                  0x0A /* Filters_Set */
#define PI_ITEM_K_SMT_USER                  0x0B /* Filters_Set */
#define PI_ITEM_K_RESERVED                  0x0C /* Filters_Set */
#define PI_ITEM_K_IMPLEMENTOR               0x0D /* Filters_Set */
#define PI_ITEM_K_LOOPBACK_MODE             0x0E /* Chars_Set */
#define PI_ITEM_K_CONFIG_POLICY             0x10 /* SMTConfigPolicy */
#define PI_ITEM_K_CON_POLICY                0x11 /* SMTConnectionPolicy */
#define PI_ITEM_K_T_NOTIFY                  0x12 /* SMTTNotify */
#define PI_ITEM_K_STATION_ACTION            0x13 /* SMTStationAction */
#define PI_ITEM_K_MAC_PATHS_REQ       	    0x15 /* MACPathsRequested */
#define PI_ITEM_K_MAC_ACTION                0x17 /* MACAction */
#define PI_ITEM_K_CON_POLICIES              0x18 /* PORTConnectionPolicies */
#define PI_ITEM_K_PORT_PATHS_REQ            0x19 /* PORTPathsRequested */
#define PI_ITEM_K_MAC_LOOP_TIME             0x1A /* PORTMACLoopTime */
#define PI_ITEM_K_TB_MAX                    0x1B /* PORTTBMax */
#define PI_ITEM_K_LER_CUTOFF                0x1C /* PORTLerCutoff */
#define PI_ITEM_K_LER_ALARM                 0x1D /* PORTLerAlarm */
#define PI_ITEM_K_PORT_ACTION               0x1E /* PORTAction */
#define PI_ITEM_K_FLUSH_TIME                0x20 /* Chars_Set */
#define PI_ITEM_K_MAC_T_REQ		    0x29 /* MACTReq */
#define PI_ITEM_K_EMAC_RING_PURGER          0x2A /* eMACRingPurgerEnable */
#define PI_ITEM_K_EMAC_RTOKEN_TIMEOUT	    0x2B /* eMACRestrictedTokenTimeout*/
#define PI_ITEM_K_MAX			    0x2B /* Must equal high item */

/* Values for some of the items */

#define PI_K_OFF			    0       /* Generic OFF|ON consts */
#define PI_K_ON				    1

#define PI_K_FALSE                          0       /* Generic false */
#define PI_K_TRUE                           1       /* Generic true */

#define PI_SNMP_K_TRUE                      1       /* SNMP true/false values */
#define PI_SNMP_K_FALSE                     2

#define PI_FSTATE_K_BLOCK		    0       /* Filter State */
#define PI_FSTATE_K_PASS		    1

/* Item value limits and defaults */

#define PI_TIME_UNIT			    80
#define PI_ITEM_K_T_REQ_MIN		    (4000000/PI_TIME_UNIT)
#define PI_ITEM_K_T_REQ_MAX		    (173015040/PI_TIME_UNIT)
#define PI_ITEM_K_T_REQ_DEF		    (8000000/PI_TIME_UNIT)
#define PI_ITEM_K_TVX_MIN		    (2500000/PI_TIME_UNIT)
#define PI_ITEM_K_TVX_MAX		    (5222400/PI_TIME_UNIT)
#define PI_ITEM_K_TVX_DEF		    (2621440/PI_TIME_UNIT)
#define PI_ITEM_K_RESTRICTED_TOKEN_MIN	    0
#define PI_ITEM_K_RESTRICTED_TOKEN_MAX	    ((1000000000/PI_TIME_UNIT)*10)
#define PI_ITEM_K_RESTRICTED_TOKEN_DEF	    (1000000000/PI_TIME_UNIT)
#define PI_ITEM_K_LEM_THRESHOLD_MIN	    5
#define PI_ITEM_K_LEM_THRESHOLD_MAX	    8
#define PI_ITEM_K_LEM_THRESHOLD_DEF	    8
#define PI_ITEM_K_RING_PURGER_DEF           PI_K_OFF 
#define PI_ITEM_K_IND_GROUP_PROM_DEF	    PI_FSTATE_K_BLOCK
#define PI_ITEM_K_GROUP_PROM_DEF	    PI_FSTATE_K_BLOCK
#define PI_ITEM_K_BROADCAST_DEF		    PI_FSTATE_K_BLOCK
#define PI_ITEM_K_SMT_PROM_DEF		    PI_FSTATE_K_BLOCK
#define PI_ITEM_K_SMT_USER_DEF	            PI_FSTATE_K_BLOCK
#define PI_ITEM_K_RESERVED_DEF		    PI_FSTATE_K_BLOCK
#define PI_ITEM_K_IMPLEMENTOR_DEF	    PI_FSTATE_K_BLOCK
#define PI_ITEM_K_LOOPBACK_NONE             0        /* No loopback */
#define PI_ITEM_K_LOOPBACK_INT              1        /* Internal loopback */
#define PI_ITEM_K_LOOPBACK_EXT              2        /* External loopback */
#define PI_ITEM_K_CONFIG_POLICY_DEF         0        /* none supported (def) */
#define PI_ITEM_K_CON_POLICY_MIN            0        /* Reject none */
#define PI_ITEM_K_CON_POLICY_MAX            65535    /* Reject A-A A-B A-S... */
#define PI_ITEM_K_CON_POLICY_DEF            0x8021   /* Reject A-A B-B M-M */
#define PI_ITEM_K_T_NOTIFY_MIN              2        /* 2 second minimum */
#define PI_ITEM_K_T_NOTIFY_MAX              30       /* 30 second maximum */
#define PI_ITEM_K_T_NOTIFY_DEF              30       /* 30 second default */
#define PI_ITEM_K_MAC_PATHS_REQ_DEF   	    1        /* Primary path default */
#define PI_ITEM_K_CON_POLICIES_MIN          0        /* none */
#define PI_ITEM_K_CON_POLICIES_MAX          7        /* LCT, Loop & Placement */
#define PI_ITEM_K_CON_POLICIES_DEF          4        /* Pc-MAC-Placement def */
#define PI_ITEM_K_PORT_PATHS_REQ_DEF        1        /* Primary path default */
#define PI_ITEM_K_MAC_LOOP_TIME_MIN         (200000000/PI_TIME_UNIT)
#define PI_ITEM_K_MAC_LOOP_TIME_MAX         ((1000000000/PI_TIME_UNIT)*10)
#define PI_ITEM_K_MAC_LOOP_TIME_DEF         (200000000/PI_TIME_UNIT)
#define PI_ITEM_K_TB_MAX_MIN                (30000000/PI_TIME_UNIT)
#define PI_ITEM_K_TB_MAX_MAX                ((1000000000/PI_TIME_UNIT)*10)
#define PI_ITEM_K_TB_MAX_DEF                (50000000/PI_TIME_UNIT)
#define PI_ITEM_K_LER_CUTOFF_MIN            4        /* min value - SMT spec */
#define PI_ITEM_K_LER_CUTOFF_MAX            15       /* max value - SMT spec */
#define PI_ITEM_K_LER_CUTOFF_DEF            7        /* def value - SMT spec */
#define PI_ITEM_K_LER_ALARM_MIN             4        /* min value - SMT spec */
#define PI_ITEM_K_LER_ALARM_MAX             15       /* max value - SMT spec */
#define PI_ITEM_K_LER_ALARM_DEF             8        /* def value - SMT spec */
#define PI_ITEM_K_FLUSH_TIME_MIN            0	     /* Disables flush */
#define PI_ITEM_K_FLUSH_TIME_MAX            255      /* Number of seconds */
#define PI_ITEM_K_FLUSH_TIME_DEF            0        /* Default */
#define PI_ITEM_K_FLUSH_TIME_REC            3        /* Arch recommended */
#define PI_ITEM_K_MAC_T_REQ_MIN		    PI_ITEM_K_T_REQ_MIN
#define PI_ITEM_K_MAC_T_REQ_MAX		    PI_ITEM_K_T_REQ_MAX
#define PI_ITEM_K_MAC_T_REQ_DEF             PI_ITEM_K_T_REQ_DEF
#define PI_ITEM_K_EMAC_RING_PURGER_DEF      PI_SNMP_K_FALSE
#define PI_ITEM_K_EMAC_RTOKEN_TIME_MIN      PI_ITEM_K_RESTRICTED_TOKEN_MIN
#define PI_ITEM_K_EMAC_RTOKEN_TIME_MAX      PI_ITEM_K_RESTRICTED_TOKEN_MAX
#define PI_ITEM_K_EMAC_RTOKEN_TIME_DEF      PI_ITEM_K_RESTRICTED_TOKEN_DEF


/* Default, min, max definitions for writeable SMT 7.2 mandatory objects */

#define PI_ITEM_K_SMT_CONFIG_POL_DEF   0
#define PI_ITEM_K_SMT_CONN_POL_DEF     0x8021
#define PI_ITEM_K_SMT_CONN_POL_MIN     0x8000
#define PI_ITEM_K_SMT_CONN_POL_MAX     0xFFFF
#define PI_ITEM_K_SMT_T_NOTIFY_DEF     30
#define PI_ITEM_K_SMT_T_NOTIFY_MIN     2
#define PI_ITEM_K_SMT_T_NOTIFY_MAX     30
#define PI_ITEM_K_SMT_STAT_POL_DEF     PI_K_TRUE
#define PI_ITEM_K_SMT_TR_MAX_EXP_DEF   ((700000000/PI_TIME_UNIT)*10)
#define PI_ITEM_K_SMT_TR_MAX_EXP_MIN   ((600177300/PI_TIME_UNIT)*10)
#define PI_ITEM_K_SMT_TR_MAX_EXP_MAX   ((1000000000/PI_TIME_UNIT)*100)
#define PI_ITEM_K_MAC_REQ_PATHS_DEF    1
#define PI_ITEM_K_MAC_FRM_ERR_THR_DEF  0
#define PI_ITEM_K_MAC_FRM_ERR_THR_MIN  0
#define PI_ITEM_K_MAC_FRM_ERR_THR_MAX  65535
#define PI_ITEM_K_MAC_MA_UNIT_ENAB_DEF PI_K_TRUE
#define PI_ITEM_K_PATH_TVX_LB_DEF      (2500000/PI_TIME_UNIT)
#define PI_ITEM_K_PATH_T_MAX_LB_DEF    (165000000/PI_TIME_UNIT)
#define PI_ITEM_K_PATH_MAX_T_REQ_DEF   (165000000/PI_TIME_UNIT)
#define PI_ITEM_K_PORT_CONN_POLS_DEF   0
#define PI_ITEM_K_PORT_REQ_PATHS_DEF   1
#define PI_ITEM_K_PORT_LER_CUTOFF_DEF  7
#define PI_ITEM_K_PORT_LER_CUTOFF_MIN  4
#define PI_ITEM_K_PORT_LER_CUTOFF_MAX  15
#define PI_ITEM_K_PORT_LER_ALARM_DEF   8
#define PI_ITEM_K_PORT_LER_ALARM_MIN   4
#define PI_ITEM_K_PORT_LER_ALARM_MAX   15

/*
 *  Item code and default constant for FDX (Full-Duplex) enable object.  This
 *  object is settable through SNMP_SET and is only valid for firmware images
 *  containing SMT 7.2 CNS code.
 */

#define PI_ITEM_K_FDX_ENB_DIS          0x2F11             /* eFDXEnable */
#define PI_ITEM_K_FDX_ENB_DEF          PI_SNMP_K_FALSE
 
/* Define command return codes */

#define PI_RSP_K_SUCCESS		    0x00
#define PI_RSP_K_FAILURE		    0x01
#define PI_RSP_K_WARNING		    0x02
#define PI_RSP_K_LOOP_MODE_BAD		    0x03
#define PI_RSP_K_ITEM_CODE_BAD		    0x04
#define PI_RSP_K_TVX_BAD		    0x05
#define PI_RSP_K_TREQ_BAD		    0x06
#define PI_RSP_K_TOKEN_BAD		    0x07
#define PI_RSP_K_NO_EOL			    0x0C
#define PI_RSP_K_FILTER_STATE_BAD	    0x0D
#define PI_RSP_K_CMD_TYPE_BAD		    0x0E
#define PI_RSP_K_ADAPTER_STATE_BAD	    0x0F
#define PI_RSP_K_RING_PURGER_BAD	    0x10
#define PI_RSP_K_LEM_THRESHOLD_BAD	    0x11
#define PI_RSP_K_LOOP_NOT_SUPPORTED	    0x12
#define PI_RSP_K_FLUSH_TIME_BAD		    0x13
#define PI_RSP_K_NOT_IMPLEMENTED	    0x14
#define PI_RSP_K_CONFIG_POLICY_BAD          0x15
#define PI_RSP_K_STATION_ACTION_BAD         0x16
#define PI_RSP_K_MAC_ACTION_BAD             0x17
#define PI_RSP_K_CON_POLICIES_BAD           0x18
#define PI_RSP_K_MAC_LOOP_TIME_BAD          0x19
#define PI_RSP_K_TB_MAX_BAD                 0x1A
#define PI_RSP_K_LER_CUTOFF_BAD             0x1B
#define PI_RSP_K_LER_ALARM_BAD              0x1C
#define PI_RSP_K_MAC_PATHS_REQ_BAD          0x1D
#define PI_RSP_K_MAC_T_REQ_BAD              0x1E
#define PI_RSP_K_EMAC_RING_PURGER_BAD       0x1F
#define PI_RSP_K_EMAC_RTOKEN_TIME_BAD       0x20
#define PI_RSP_K_NO_SUCH_ENTRY              0x21
#define PI_RSP_K_T_NOTIFY_BAD		    0x22
#define PI_RSP_K_TR_MAX_EXP_BAD		    0x23
#define PI_RSP_K_MAC_FRM_ERR_THR_BAD	    0x24
#define PI_RSP_K_MAX_T_REQ_BAD		    0x25
#define PI_RSP_K_FDX_ENB_DIS_BAD	    0x26

/* SNMP Values */

#define PI_STATION_ACT_K_OTHER              0x01	/* SMTStationAction */
#define PI_STATION_ACT_K_CON		    0x02	
#define PI_STATION_ACT_K_DISCON             0x03
#define PI_STATION_ACT_K_PATH_TEST          0x04
#define PI_STATION_ACT_K_SELF_TEST          0x05

#define PI_MAC_ACT_K_OTHER                  0x01	/* MACAction */
#define PI_MAC_ACT_K_ENB_LLC                0x02
#define PI_MAC_ACT_K_DIS_LLC                0x03
#define PI_MAC_ACT_K_CON                    0x04
#define PI_MAC_ACT_K_DISCON                 0x05


/* Commonly used structures */

typedef struct                               /* Item list */
    {
    PI_UINT  item_code;
    PI_UINT  value;
    } PI_ITEM_LIST;

typedef struct
    {
    PI_UINT  resrv;
    PI_UINT  cmd_type;
    PI_UINT  status;
    } PI_RSP_HEADER;

/*
 * Command structure for the following commands:
 * start, filters_get, status_char_get, cntrs_get, addr_filter and
 * error_log_clear.
 */
typedef struct {
    PI_UINT cmd_type;
} NODATA_CMD;

/* Start Command */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_START_REQ;

/* Start Response */

typedef struct 
    {
    PI_RSP_HEADER	header;    
    } PI_CMD_START_RSP;

/* Filters_Set Request */

#define PI_CMD_FILTERS_SET_K_ITEMS_MAX  63 /* Fits in a 512 byte buffer */

typedef struct 
    {
    PI_UINT	    cmd_type;
    PI_ITEM_LIST    item[PI_CMD_FILTERS_SET_K_ITEMS_MAX];
    } PI_CMD_FILTERS_SET_REQ;

/* Filters_Set Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_FILTERS_SET_RSP;

/* Filters_Get Request */

typedef struct
    {
    PI_UINT  cmd_type;   
    } PI_CMD_FILTERS_GET_REQ;

/* Filters_Get Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    PI_UINT	    ind_group_prom;
    PI_UINT	    group_prom;
    PI_UINT	    broadcast_all;
    PI_UINT	    smt_all;
    PI_UINT	    smt_user;
    PI_UINT	    reserved_all;
    PI_UINT	    implementor_all;
    } PI_CMD_FILTERS_GET_RSP;

/* Chars_Set Request */

#define PI_CMD_CHARS_SET_K_ITEMS_MAX 42 /* Fits in a 512 byte buffer */

typedef struct
    {
    PI_UINT	  cmd_type;
    struct                               /* Item list */
        {
        PI_UINT  item_code;
        PI_UINT  value;
        PI_UINT  item_index;
        } item[PI_CMD_CHARS_SET_K_ITEMS_MAX];    
    } PI_CMD_CHARS_SET_REQ;

/* Chars_Set Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_CHARS_SET_RSP;

/* SNMP_Set Request */

#define PI_CMD_SNMP_SET_K_ITEMS_MAX 42       	/* Fits in a 512 byte buffer */

typedef struct
    {
    PI_UINT	    cmd_type;
    struct                               /* Item list */
        {
        PI_UINT    item_code;
        PI_UINT    value;
        PI_UINT    item_index;
        } item[PI_CMD_SNMP_SET_K_ITEMS_MAX];    
    } PI_CMD_SNMP_SET_REQ;

/* SNMP_Set Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_SNMP_SET_RSP;

/* SMT_MIB_SET Request */

#define PI_CMD_SMT_MIB_SET_K_ITEMS_MAX 42   /* Max number of items */ 

typedef struct
    {
    PI_UINT	    cmd_type;
    struct
        {
	PI_UINT    item_code;
	PI_UINT    value;
	PI_UINT    item_index;
	} item[PI_CMD_SMT_MIB_SET_K_ITEMS_MAX];
    } PI_CMD_SMT_MIB_SET_REQ;

/* SMT_MIB_Set Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_SMT_MIB_SET_RSP;

/* SMT_MIB_Get Request */

typedef struct
    {
    PI_ULONG  cmd_type;
    } PI_CMD_SMT_MIB_GET_REQ;

/* SMT_MIB_GET Response */

typedef struct                          /* Refer to ANSI FDDI SMT Rev. 7.2 */
    {
    PI_RSP_HEADER  header;
    
    /* SMT GROUP */                                                          


    PI_STATION_ID  smt_station_id;
    PI_UINT       smt_op_version_id;
    PI_UINT       smt_hi_version_id;
    PI_UINT       smt_lo_version_id;
    PI_USER_DATA   smt_user_data;
    PI_UINT       smt_mib_version_id;
    PI_UINT       smt_mac_ct;
    PI_UINT       smt_non_master_ct;
    PI_UINT       smt_master_ct;
    PI_UINT       smt_available_paths;
    PI_UINT       smt_config_capabilities;
    PI_UINT       smt_config_policy;
    PI_UINT       smt_connection_policy;
    PI_UINT       smt_t_notify;
    PI_UINT       smt_stat_rpt_policy;
    PI_UINT       smt_trace_max_expiration;
    PI_UINT       smt_bypass_present;
    PI_UINT       smt_ecm_state;
    PI_UINT       smt_cf_state;
    PI_UINT       smt_remote_disconnect_flag;
    PI_UINT	  smt_station_status;
    PI_UINT       smt_peer_wrap_flag;
    PI_CNTR	  smt_msg_time_stamp;
    PI_CNTR	  smt_transition_time_stamp;

    /* MAC GROUP */

    PI_UINT       mac_frame_status_functions;
    PI_UINT       mac_t_max_capability;
    PI_UINT       mac_tvx_capability;
    PI_UINT       mac_available_paths;
    PI_UINT       mac_current_path;
    PI_LAN_ADDR    mac_upstream_nbr;
    PI_LAN_ADDR    mac_downstream_nbr;
    PI_LAN_ADDR    mac_old_upstream_nbr;
    PI_LAN_ADDR    mac_old_downstream_nbr;
    PI_UINT       mac_dup_address_test;
    PI_UINT       mac_requested_paths;
    PI_UINT       mac_downstream_port_type;
    PI_LAN_ADDR    mac_smt_address;
    PI_UINT       mac_t_req;
    PI_UINT       mac_t_neg;
    PI_UINT       mac_t_max;
    PI_UINT       mac_tvx_value;
    PI_UINT       mac_frame_error_threshold;
    PI_UINT       mac_frame_error_ratio;
    PI_UINT       mac_rmt_state;
    PI_UINT       mac_da_flag;
    PI_UINT       mac_unda_flag;
    PI_UINT       mac_frame_error_flag;
    PI_UINT	  mac_ma_unitdata_available;
    PI_UINT	  mac_hw_present;
    PI_UINT       mac_ma_unitdata_enable;

    /* PATH GROUP */

    PI_USER_DATA  path_configuration;
    PI_UINT       path_tvx_lower_bound;
    PI_UINT       path_t_max_lower_bound;
    PI_UINT       path_max_t_req;

    /* PORT GROUP */

    PI_UINT       port_my_type[PI_PHY_K_MAX];
    PI_UINT       port_neighbor_type[PI_PHY_K_MAX];
    PI_UINT       port_connection_policies[PI_PHY_K_MAX];
    PI_UINT       port_mac_indicated[PI_PHY_K_MAX];
    PI_UINT       port_current_path[PI_PHY_K_MAX];
    PI_UINT       port_requested_paths[PI_PHY_K_MAX];
    PI_UINT       port_mac_placement[PI_PHY_K_MAX];
    PI_UINT       port_available_paths[PI_PHY_K_MAX];
    PI_UINT       port_pmd_class[PI_PHY_K_MAX];
    PI_UINT       port_connection_capabilities[PI_PHY_K_MAX];
    PI_UINT       port_bs_flag[PI_PHY_K_MAX];
    PI_UINT       port_ler_estimate[PI_PHY_K_MAX];
    PI_UINT       port_ler_cutoff[PI_PHY_K_MAX];    
    PI_UINT       port_ler_alarm[PI_PHY_K_MAX];
    PI_UINT       port_connect_state[PI_PHY_K_MAX];
    PI_UINT       port_pcm_state[PI_PHY_K_MAX];
    PI_UINT       port_pc_withhold[PI_PHY_K_MAX];
    PI_UINT       port_ler_flag[PI_PHY_K_MAX];
    PI_UINT       port_hardware_present[PI_PHY_K_MAX];

    } PI_CMD_SMT_MIB_GET_RSP; 


/*
 *  Item and group code definitions for SMT 7.2 mandatory objects.  These
 *  definitions are to be used as appropriate in SMT_MIB_SET commands and
 *  certain host-sent SMT frames such as PMF Get and Set requests.  The
 *  codes have been taken from the MIB summary section of ANSI SMT 7.2.
 */
    
#define PI_GRP_K_SMT_STATION_ID        0x100A
#define PI_ITEM_K_SMT_STATION_ID       0x100B
#define PI_ITEM_K_SMT_OP_VERS_ID       0x100D
#define PI_ITEM_K_SMT_HI_VERS_ID       0x100E
#define PI_ITEM_K_SMT_LO_VERS_ID       0x100F
#define PI_ITEM_K_SMT_USER_DATA        0x1011
#define PI_ITEM_K_SMT_MIB_VERS_ID      0x1012

#define PI_GRP_K_SMT_STATION_CONFIG    0x1014
#define PI_ITEM_K_SMT_MAC_CT           0x1015
#define PI_ITEM_K_SMT_NON_MASTER_CT    0x1016
#define PI_ITEM_K_SMT_MASTER_CT        0x1017
#define PI_ITEM_K_SMT_AVAIL_PATHS      0x1018
#define PI_ITEM_K_SMT_CONFIG_CAPS      0x1019
#define PI_ITEM_K_SMT_CONFIG_POL       0x101A
#define PI_ITEM_K_SMT_CONN_POL         0x101B
#define PI_ITEM_K_SMT_T_NOTIFY         0x101D
#define PI_ITEM_K_SMT_STAT_POL         0x101E
#define PI_ITEM_K_SMT_TR_MAX_EXP       0x101F
#define PI_ITEM_K_SMT_PORT_INDEXES     0x1020
#define PI_ITEM_K_SMT_MAC_INDEXES      0x1021
#define PI_ITEM_K_SMT_BYPASS_PRESENT   0x1022

#define PI_GRP_K_SMT_STATUS	       0x1028
#define PI_ITEM_K_SMT_ECM_STATE        0x1029
#define PI_ITEM_K_SMT_CF_STATE         0x102A
#define PI_ITEM_K_SMT_REM_DISC_FLAG    0x102C
#define PI_ITEM_K_SMT_STATION_STATUS   0x102D
#define PI_ITEM_K_SMT_PEER_WRAP_FLAG   0x102E

#define PI_GRP_K_SMT_MIB_OPERATION     0x1032
#define PI_ITEM_K_SMT_MSG_TIME_STAMP   0x1033
#define PI_ITEM_K_SMT_TRN_TIME_STAMP   0x1034

#define PI_ITEM_K_SMT_STATION_ACT      0x103C

#define PI_GRP_K_MAC_CAPABILITIES      0x200A
#define PI_ITEM_K_MAC_FRM_STAT_FUNC    0x200B
#define PI_ITEM_K_MAC_T_MAX_CAP        0x200D
#define PI_ITEM_K_MAC_TVX_CAP          0x200E

#define PI_GRP_K_MAC_CONFIG	       0x2014
#define PI_ITEM_K_MAC_AVAIL_PATHS      0x2016
#define PI_ITEM_K_MAC_CURRENT_PATH     0x2017
#define PI_ITEM_K_MAC_UP_NBR           0x2018
#define PI_ITEM_K_MAC_DOWN_NBR         0x2019
#define PI_ITEM_K_MAC_OLD_UP_NBR       0x201A
#define PI_ITEM_K_MAC_OLD_DOWN_NBR     0x201B
#define PI_ITEM_K_MAC_DUP_ADDR_TEST    0x201D
#define PI_ITEM_K_MAC_REQ_PATHS        0x2020
#define PI_ITEM_K_MAC_DOWN_PORT_TYPE   0x2021
#define PI_ITEM_K_MAC_INDEX            0x2022

#define PI_GRP_K_MAC_ADDRESS           0x2028
#define PI_ITEM_K_MAC_SMT_ADDRESS      0x2029

#define PI_GRP_K_MAC_OPERATION         0x2032
#define PI_ITEM_K_MAC_TREQ             0x2033 /* resolve conflict with 6.2 */
#define PI_ITEM_K_MAC_T_NEG            0x2034
#define PI_ITEM_K_MAC_T_MAX            0x2035
#define PI_ITEM_K_MAC_TVX_VALUE        0x2036

#define PI_GRP_K_MAC_COUNTERS          0x2046
#define PI_ITEM_K_MAC_FRAME_CT         0x2047
#define PI_ITEM_K_MAC_COPIED_CT        0x2048
#define PI_ITEM_K_MAC_TRANSMIT_CT      0x2049
#define PI_ITEM_K_MAC_ERROR_CT         0x2051
#define PI_ITEM_K_MAC_LOST_CT          0x2052

#define PI_GRP_K_MAC_FRM_ERR_COND      0x205A

#define PI_ITEM_K_MAC_FRM_ERR_THR      0x205F
#define PI_ITEM_K_MAC_FRM_ERR_RAT      0x2060

#define PI_GRP_K_MAC_STATUS            0x206E
#define PI_ITEM_K_MAC_RMT_STATE        0x206F
#define PI_ITEM_K_MAC_DA_FLAG          0x2070
#define PI_ITEM_K_MAC_UNDA_FLAG        0x2071
#define PI_ITEM_K_MAC_FRM_ERR_FLAG     0x2072
#define PI_ITEM_K_MAC_MA_UNIT_AVAIL    0x2074
#define PI_ITEM_K_MAC_HW_PRESENT       0x2075
#define PI_ITEM_K_MAC_MA_UNIT_ENAB     0x2076

#define PI_GRP_K_PATH_CONFIG           0x320A
#define PI_ITEM_K_PATH_INDEX           0x320B

#define PI_ITEM_K_PATH_CONFIGURATION   0x3212

#define PI_ITEM_K_PATH_TVX_LB          0x3215
#define PI_ITEM_K_PATH_T_MAX_LB        0x3216
#define PI_ITEM_K_PATH_MAX_T_REQ       0x3217

#define PI_GRP_K_PORT_CONFIG           0x400A

#define Pi_ITEM_K_PORT_MY_TYPE         0x400C
#define PI_ITEM_K_PORT_NBR_TYPE        0x400D
#define PI_ITEM_K_PORT_CONN_POLS       0x400E
#define PI_ITEM_K_PORT_MAC_INDICATED   0x400F
#define PI_ITEM_K_PORT_CURRENT_PATH    0x4010
#define PI_ITEM_K_PORT_REQ_PATHS       0x4011
#define PI_ITEM_K_PORT_MAC_PLACEMENT   0x4012
#define PI_ITEM_K_PORT_AVAIL_PATHS     0x4013
#define PI_ITEM_K_PORT_PMD_CLASS       0x4016
#define PI_ITEM_K_PORT_CONN_CAPS       0x4017
#define PI_ITEM_K_PORT_INDEX           0x401D
#define PI_GRP_K_PORT_OPERATION        0x401E

#define PI_ITEM_K_PORT_BS_FLAG         0x4021

#define PI_GRP_K_PORT_ERR_CNTRS        0x4028
#define PI_ITEM_K_PORT_LCT_FAIL_CT     0x402A

#define PI_GRP_K_PORT_LER              0x4032
#define PI_ITEM_K_PORT_LER_ESTIMATE    0x4033
#define PI_ITEM_K_PORT_LEM_REJ_CT      0x4034
#define PI_ITEM_K_PORT_LEM_CT          0x4035

#define PI_ITEM_K_PORT_LER_CUTOFF      0x403A
#define PI_ITEM_K_PORT_LER_ALARM       0x403B
#define PI_GRP_K_PORT_STATUS           0x403C

#define PI_ITEM_K_PORT_CONNECT_STATE   0x403D
#define PI_ITEM_K_PORT_PCM_STATE       0x403E
#define PI_ITEM_K_PORT_PC_WITHHOLD     0x403F
#define PI_ITEM_K_PORT_LER_FLAG	       0x4040
#define PI_ITEM_K_PORT_HW_PRESENT      0x4041

#define PI_ITEM_K_PORT_ACT             0x4046


/* Addr_Filter_Set Request */

#define PI_CMD_ADDR_FILTER_K_FIRST   2
#define PI_CMD_ADDR_FILTER_K_SIZE   62

typedef struct
    {
    PI_UINT	cmd_type;
    PI_LAN_ADDR	entry[PI_CMD_ADDR_FILTER_K_SIZE];
    } PI_CMD_ADDR_FILTER_SET_REQ;

/* Addr_Filter_Set Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_ADDR_FILTER_SET_RSP;

/* Addr_Filter_Get Request */

typedef struct
    {
    PI_UINT	cmd_type;
    } PI_CMD_ADDR_FILTER_GET_REQ;

/* Addr_Filter_Get Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    PI_LAN_ADDR	    entry[PI_CMD_ADDR_FILTER_K_SIZE];
    } PI_CMD_ADDR_FILTER_GET_RSP;

/* Dev_Specific_Get Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_DEVICE_SPECIFIC_GET_REQ;

/* Dev_Specific_Get Response => defined in device specific module */

/* Status_Chars_Get Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_STATUS_CHARS_GET_REQ;

/* Status_Chars_Get Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    PI_STATION_ID   station_id;				/* Station */
    PI_UINT	    station_type;
    PI_UINT	    smt_ver_id;
    PI_UINT	    smt_ver_id_max;
    PI_UINT	    smt_ver_id_min;
    PI_UINT	    station_state;
    PI_LAN_ADDR	    link_addr;				/* Link */
    PI_UINT	    t_req;
    PI_UINT	    tvx;
    PI_UINT	    token_timeout;
    PI_UINT	    purger_enb;
    PI_UINT	    link_state;
    PI_UINT	    tneg;
    PI_UINT	    dup_addr_flag;
    PI_LAN_ADDR	    una;
    PI_LAN_ADDR	    una_old;
    PI_UINT	    un_dup_addr_flag;
    PI_LAN_ADDR	    dna;
    PI_LAN_ADDR	    dna_old;
    PI_UINT	    purger_state;
    PI_UINT	    fci_mode;
    PI_UINT	    error_reason;
    PI_UINT	    loopback;
    PI_UINT	    ring_latency;
    PI_LAN_ADDR	    last_dir_beacon_sa;
    PI_LAN_ADDR	    last_dir_beacon_una;
    PI_UINT	    phy_type[PI_PHY_K_MAX];		/* Phy */
    PI_UINT	    pmd_type[PI_PHY_K_MAX];
    PI_UINT	    lem_threshold[PI_PHY_K_MAX];
    PI_UINT	    phy_state[PI_PHY_K_MAX];
    PI_UINT	    nbor_phy_type[PI_PHY_K_MAX];
    PI_UINT	    link_error_est[PI_PHY_K_MAX];
    PI_UINT	    broken_reason[PI_PHY_K_MAX];
    PI_UINT	    reject_reason[PI_PHY_K_MAX];
    PI_UINT	    cntr_interval;			/* Miscellaneous */
    PI_UCHAR	    module_rev[4];
    PI_UCHAR	    firmware_rev[4];
    PI_UINT	    mop_device_type;
    PI_UINT	    phy_led[PI_PHY_K_MAX];
    PI_UINT	    flush_time;
    } PI_CMD_STATUS_CHARS_GET_RSP;

/* STATUS_CHARS_TEST Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_STATUS_CHARS_TEST_REQ;

/* STATUS_CHARS_TEST Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_STATUS_CHARS_TEST_RSP;
	
/* FDDI_MIB_Get Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_FDDI_MIB_GET_REQ;

/* FDDI_MIB_Get Response */

typedef struct
    {
    PI_RSP_HEADER   header;		

    /* SMT GROUP */

    PI_STATION_ID   smt_station_id;			
    PI_UINT	    smt_op_version_id;
    PI_UINT	    smt_hi_version_id;
    PI_UINT	    smt_lo_version_id;
    PI_UINT	    smt_mac_ct;				
    PI_UINT	    smt_non_master_ct;			
    PI_UINT	    smt_master_ct;				
    PI_UINT	    smt_paths_available;			
    PI_UINT	    smt_config_capabilities;		
    PI_UINT	    smt_config_policy;		
    PI_UINT	    smt_connection_policy;		
    PI_UINT	    smt_t_notify;	
    PI_UINT	    smt_status_reporting;
    PI_UINT	    smt_ecm_state;	
    PI_UINT	    smt_cf_state;	
    PI_UINT	    smt_hold_state;		
    PI_UINT	    smt_remote_disconnect_flag;
    PI_UINT	    smt_station_action;			

    /* MAC GROUP */

    PI_UINT	    mac_frame_status_capabilities;	
    PI_UINT	    mac_t_max_greatest_lower_bound;
    PI_UINT	    mac_tvx_greatest_lower_bound;
    PI_UINT	    mac_paths_available;
    PI_UINT	    mac_current_path;
    PI_LAN_ADDR	    mac_upstream_nbr;			
    PI_LAN_ADDR	    mac_old_upstream_nbr;		
    PI_UINT	    mac_dup_addr_test;			
    PI_UINT	    mac_paths_requested;
    PI_UINT	    mac_downstream_port_type;
    PI_LAN_ADDR     mac_smt_address;			
    PI_UINT	    mac_t_req;				
    PI_UINT	    mac_t_neg;
    PI_UINT        mac_t_max;				
    PI_UINT	    mac_tvx_value;			
    PI_UINT	    mac_t_min;				
    PI_UINT	    mac_current_frame_status;
    /*              mac_frame_cts 			*/
    /* 		    mac_error_cts 			*/
    /* 		    mac_lost_cts 			*/
    PI_UINT	    mac_frame_error_threshold;		
    PI_UINT	    mac_frame_error_ratio;		
    PI_UINT	    mac_rmt_state;
    PI_UINT	    mac_da_flag;
    PI_UINT	    mac_una_da_flag;			
    PI_UINT	    mac_frame_condition;
    PI_UINT	    mac_chip_set;			
    PI_UINT	    mac_action;				

    /* PATH GROUP => Does not need to be implemented */

    /* PORT GROUP */

    PI_UINT	    port_pc_type[PI_PHY_K_MAX];			
    PI_UINT	    port_pc_neighbor[PI_PHY_K_MAX];			
    PI_UINT	    port_connection_policies[PI_PHY_K_MAX];
    PI_UINT	    port_remote_mac_indicated[PI_PHY_K_MAX];
    PI_UINT	    port_ce_state[PI_PHY_K_MAX];
    PI_UINT	    port_paths_requested[PI_PHY_K_MAX];
    PI_UINT	    port_mac_placement[PI_PHY_K_MAX];
    PI_UINT	    port_available_paths[PI_PHY_K_MAX];
    PI_UINT	    port_mac_loop_time[PI_PHY_K_MAX];
    PI_UINT	    port_tb_max[PI_PHY_K_MAX];
    PI_UINT	    port_bs_flag[PI_PHY_K_MAX];
    /*		    port_lct_fail_cts[PI_PHY_K_MAX];	*/
    PI_UINT	    port_ler_estimate[PI_PHY_K_MAX];			    
    /*		    port_lem_reject_cts[PI_PHY_K_MAX];	*/
    /*	    	    port_lem_cts[PI_PHY_K_MAX];		*/
    PI_UINT	    port_ler_cutoff[PI_PHY_K_MAX];			    
    PI_UINT	    port_ler_alarm[PI_PHY_K_MAX];			    
    PI_UINT	    port_connect_state[PI_PHY_K_MAX];
    PI_UINT	    port_pcm_state[PI_PHY_K_MAX];
    PI_UINT	    port_pc_withhold[PI_PHY_K_MAX];
    PI_UINT	    port_ler_condition[PI_PHY_K_MAX];			    
    PI_UINT	    port_chip_set[PI_PHY_K_MAX];			    
    PI_UINT	    port_action[PI_PHY_K_MAX];			    

    /* ATTACHMENT GROUP */

    PI_UINT	    attachment_class;
    PI_UINT	    attachment_ob_present;
    PI_UINT	    attachment_imax_expiration;
    PI_UINT	    attachment_inserted_status;
    PI_UINT	    attachment_insert_policy;

    /* CHIP SET GROUP => Does not need to be implemented */

    } PI_CMD_FDDI_MIB_GET_RSP;

/* FDDI_MIB_TEST Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_FDDI_MIB_TEST_REQ;

/* FDDI_MIB_TEST Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_FDDI_MIB_TEST_RSP;

/* DEC_Ext_MIB_Get Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_DEC_EXT_MIB_GET_REQ;

/* DEC_Ext_MIB_Get (efddi and efdx groups only) Response */

typedef struct
    {
    PI_RSP_HEADER   header;		

    /* SMT GROUP */

    PI_UINT	    esmt_station_type;

    /* MAC GROUP */

    PI_UINT	    emac_link_state;			
    PI_UINT	    emac_ring_purger_state;
    PI_UINT        emac_ring_purger_enable;
    PI_UINT	    emac_frame_strip_mode;
    PI_UINT	    emac_ring_error_reason;
    PI_UINT	    emac_up_nbr_dup_addr_flag;
    PI_UINT	    emac_restricted_token_timeout;

    /* PORT GROUP */

    PI_UINT	    eport_pmd_type[PI_PHY_K_MAX];
    PI_UINT	    eport_phy_state[PI_PHY_K_MAX];
    PI_UINT	    eport_reject_reason[PI_PHY_K_MAX];

    /* FDX (Full-Duplex) GROUP */

    PI_UINT	    efdx_enable;	/* Valid only in SMT 7.2 */
    PI_UINT	    efdx_op;		/* Valid only in SMT 7.2 */
    PI_UINT	    efdx_state;		/* Valid only in SMT 7.2 */

    } PI_CMD_DEC_EXT_MIB_GET_RSP;

/* DEC_MIB_TEST Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_DEC_EXT_MIB_TEST_REQ;

/* DEC_MIB_TEST Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_DEC_EXT_MIB_TEST_RSP;

typedef struct
    {
    PI_CNTR	    traces_rcvd;			/* Station */
    PI_CNTR	    frame_cnt;				/* Link */
    PI_CNTR	    error_cnt;
    PI_CNTR	    lost_cnt;
    PI_CNTR	    octets_rcvd;
    PI_CNTR	    octets_sent;
    PI_CNTR	    pdus_rcvd;
    PI_CNTR	    pdus_sent;
    PI_CNTR	    mcast_octets_rcvd;
    PI_CNTR	    mcast_octets_sent;
    PI_CNTR	    mcast_pdus_rcvd;
    PI_CNTR	    mcast_pdus_sent;
    PI_CNTR	    xmt_underruns;
    PI_CNTR	    xmt_failures;
    PI_CNTR         block_check_errors;
    PI_CNTR	    frame_status_errors;
    PI_CNTR         pdu_length_errors;
    PI_CNTR	    rcv_overruns;
    PI_CNTR	    user_buff_unavailable;
    PI_CNTR	    inits_initiated;
    PI_CNTR	    inits_rcvd;
    PI_CNTR	    beacons_initiated;
    PI_CNTR	    dup_addrs;
    PI_CNTR	    dup_tokens;
    PI_CNTR	    purge_errors;
    PI_CNTR	    fci_strip_errors;
    PI_CNTR	    traces_initiated;
    PI_CNTR	    directed_beacons_rcvd;
    PI_CNTR         emac_frame_alignment_errors;
    PI_CNTR	    ebuff_errors[PI_PHY_K_MAX];		/* Phy */
    PI_CNTR	    lct_rejects[PI_PHY_K_MAX];
    PI_CNTR	    lem_rejects[PI_PHY_K_MAX];
    PI_CNTR	    link_errors[PI_PHY_K_MAX];
    PI_CNTR	    connections[PI_PHY_K_MAX];
    PI_CNTR         copied_cnt;		/* Vailid only if using SMT 7.2 */
    PI_CNTR	    transmit_cnt;	/* Vailid only if using SMT 7.2 */
    } PI_CNTR_BLK;

/* Counters_Get Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_CNTRS_GET_REQ;

/* Counters_Get Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    PI_CNTR	    time_since_reset;			
    PI_CNTR_BLK	    cntrs;			    
    } PI_CMD_CNTRS_GET_RSP;

/* Counters_Set Request */

typedef struct
    {
    PI_UINT	cmd_type;
    PI_CNTR_BLK	cntrs;			    
    } PI_CMD_CNTRS_SET_REQ;

/* Counters_Set Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_CNTRS_SET_RSP;

/* CNTRS_TEST Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_CNTRS_TEST_REQ;

/* CNTRS_TEST Response */

typedef struct 
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_CNTRS_TEST_RSP;

/* Error_Log_Clear Request */

typedef struct
    {
    PI_UINT  cmd_type;
    } PI_CMD_ERROR_LOG_CLEAR_REQ;

/* Error_Log_Clear Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_ERROR_LOG_CLEAR_RSP;

/* Error_Log_Get Request */

#define PI_LOG_ENTRY_K_INDEX_MIN    0	    /* Minimum index for entry */

typedef struct
    {
    PI_UINT  cmd_type;
    PI_UINT  entry_index;
    } PI_CMD_ERROR_LOG_GET_REQ;

/* Error_Log_Get Response */

#define PI_K_LOG_FW_SIZE	111	    /* Max number of fw longwords */
#define PI_K_LOG_DIAG_SIZE      6	    /* Max number of diag longwords */

typedef struct
    {
    struct 
        {
        PI_UINT    fru_imp_mask;
        PI_UINT    test_id;
        PI_UINT    reserved[PI_K_LOG_DIAG_SIZE];
        } diag;
    PI_UINT	    fw[PI_K_LOG_FW_SIZE]; /* Refer to port_dev.h for details */
    } PI_LOG_ENTRY;

typedef struct
    {
    PI_RSP_HEADER   header;    
    PI_UINT	    event_status;
    PI_UINT	    caller_id;
    PI_UINT	    timestamp_l;
    PI_UINT	    timestamp_h;
    PI_UINT	    write_count;
    PI_LOG_ENTRY    entry_info;
    } PI_CMD_ERROR_LOG_GET_RSP;

/* Define error log related constants and types.                    */
/*   Not all of the caller id's can occur.  The only ones currently */
/*   implemented are: none, selftest, mfg, fw, console              */

#define PI_LOG_EVENT_STATUS_K_VALID	0	/* Valid Event Status */
#define PI_LOG_EVENT_STATUS_K_INVALID	1	/* Invalid Event Status */
#define PI_LOG_CALLER_ID_K_NONE         0	/* No caller */
#define PI_LOG_CALLER_ID_K_SELFTEST     1	/* Normal power-up selftest */
#define PI_LOG_CALLER_ID_K_MFG          2	/* Mfg power-up selftest */
#define PI_LOG_CALLER_ID_K_ONLINE	3	/* On-line diagnostics */
#define PI_LOG_CALLER_ID_K_HW		4	/* Hardware */
#define PI_LOG_CALLER_ID_K_FW		5	/* Firmware */
#define PI_LOG_CALLER_ID_K_CNS_HW	6	/* CNS firmware */
#define PI_LOG_CALLER_ID_K_CNS_FW	7	/* CNS hardware */
#define PI_LOG_CALLER_ID_K_CONSOLE      8       /* Console Caller Id */

/* TEST_UNSOL Request */

typedef struct
    {
    PI_UINT	cmd_type;
    } PI_CMD_UNSOL_TEST_REQ;

/* TEST_UNSOL Response */

typedef struct
    {
    PI_RSP_HEADER   header;    
    } PI_CMD_TEST_UNSOL_RSP;

/* Define the types of unsolicited events */

#define PI_UNSOL_TYPE_K_EVENT_REPORT	    0
#define PI_UNSOL_TYPE_K_EVENT_CNTRS	    1

/* Define entity codes for unsolicited event reports */

#define PI_UNSOL_ENTITY_K_STATION	    0
#define PI_UNSOL_ENTITY_K_LINK		    1
#define PI_UNSOL_ENTITY_K_PHY		    2

/* Define station unsolicited event and argument codes */

#define PI_UNSOL_STAT_K_SELFTEST_FAILED     0                  
#define PI_UNSOL_STAT_K_PC_TRACE_RCVD       1
#define PI_UNSOL_STAT_K_CONFIG_CHANGE       2

#define PI_UNSOL_STAT_ARG_K_REASON	    0

/* Define link unsolicited event and argument codes */

#define PI_UNSOL_LINK_K_XMT_UNDERRUN        0
#define PI_UNSOL_LINK_K_XMT_FAILURE         1
#define PI_UNSOL_LINK_K_BLOCK_CHECK_ERR	    2
#define PI_UNSOL_LINK_K_FRAME_STAT_ERR      3
#define PI_UNSOL_LINK_K_LENGTH_ERR	    4
#define PI_UNSOL_LINK_K_BAD_IND_DST	    5
#define PI_UNSOL_LINK_K_BAD_MCAST_DST	    6
#define PI_UNSOL_LINK_K_RCV_OVERRUN         7
#define PI_UNSOL_LINK_K_NO_LINK_BUFFER      8
#define PI_UNSOL_LINK_K_NO_USER_BUFFER      9
#define PI_UNSOL_LINK_K_INIT_INITD	    10
#define PI_UNSOL_LINK_K_RING_INIT_RCVD      11
#define PI_UNSOL_LINK_K_BEACON_INITD	    12
#define PI_UNSOL_LINK_K_DUP_ADDR_FOUND	    13
#define PI_UNSOL_LINK_K_DUP_TOKEN_FOUND	    14
#define PI_UNSOL_LINK_K_RING_PURGE_ERR	    15
#define PI_UNSOL_LINK_K_FCI_STRIP_ERR	    16
#define PI_UNSOL_LINK_K_PC_TRACE_INITD	    17
#define PI_UNSOL_LINK_K_BEACON_RCVD         18

#define PI_UNSOL_LINK_ARG_K_REASON	    0
#define PI_UNSOL_LINK_ARG_K_DL_HEADER       1
#define PI_UNSOL_LINK_ARG_K_SOURCE          2
#define PI_UNSOL_LINK_ARG_K_UP_NBR          3

/* Define PHY event and argument codes */

#define PI_UNSOL_PHY_K_LEM_REJECT	    0
#define PI_UNSOL_PHY_K_EBUFF_ERR	    1
#define PI_UNSOL_PHY_K_LCT_REJECT           2

#define PI_UNSOL_PHY_ARG_K_DIRECTION        0

/* Define End-of-Argument list code */

#define PI_UNSOL_ARG_K_EOL                  0xFF

typedef struct 				/* Event Header */
    {
    PI_UINT	entity;
    PI_UINT	entity_index;
    PI_UINT	event_code;
    } PI_EVENT_HEADER;

typedef struct				/* Reason Argument Description */
    {
    PI_UINT    arg_code;
    PI_UINT	reason_code;
    } PI_UNSOL_ARG_REASON_DESC;

typedef struct				/* Data Link Header Argument Desc */
    {
    PI_UINT    arg_code;
    struct 
	{
        PI_UINT    fc;
        PI_LAN_ADDR dst_addr;
        PI_LAN_ADDR src_addr;
        } mac;
    struct 
	{
        PI_UINT    dsap;
        PI_UINT    ssap;
        PI_UINT    control;
        PI_UINT    pid_1;
        PI_UINT    pid_2;
        } llc;
    } PI_UNSOL_ARG_DL_HEADER_DESC;

typedef struct				/* Net Address Argument Description */
    {
    PI_UINT    arg_code;
    PI_LAN_ADDR	net_addr;
    } PI_UNSOL_ARG_NET_ADDR_DESC;

typedef struct				/* Direction Argument Description */
    {
    PI_UINT    arg_code;
    PI_UINT	direction;
    } PI_UNSOL_ARG_DIRECTION_DESC;

typedef struct
    {
    PI_CNTR         time_since_reset;			
    PI_CNTR_BLK     cntrs;
    } PI_UNSOL_EVENT_CNTRS;

#define PI_UNSOL_EVENT_K_DATA_SIZE  20  /* Size of event data (in longwords) */

typedef struct
    {
    PI_EVENT_HEADER event_header;
    PI_UINT        event_data[PI_UNSOL_EVENT_K_DATA_SIZE];
    } PI_UNSOL_EVENT_REPORT;

typedef struct 				/* Unsol Report: Cntr or Report */
    {
    PI_UINT	resrv;
    PI_UINT event_type;
    union
	{
        PI_UNSOL_EVENT_CNTRS  cntrs;
        PI_UNSOL_EVENT_REPORT report;
	} info;
    } PI_UNSOL_REPORT;

/* Define format of Consumer Block (resident in host memory) */

typedef struct
    {
    PI_UINT	xmt_rcv_data;
    PI_UINT	reserved_1;
    PI_UINT	smt_host;
    PI_UINT	reserved_2;
    PI_UINT	unsol;
    PI_UINT	reserved_3;
    PI_UINT	cmd_rsp;
    PI_UINT	reserved_4;
    PI_UINT	cmd_req;
    } PI_CONSUMER_BLOCK;

#define PI_CONS_M_RCV_INDEX	0X000000FF
#define PI_CONS_M_XMT_INDEX     0X00FF0000
#define PI_CONS_V_RCV_INDEX     0
#define PI_CONS_V_XMT_INDEX     16

/* Offsets into consumer block */

#define PI_CONS_BLK_K_XMT_RCV	0X00
#define PI_CONS_BLK_K_SMT_HOST	0X08
#define PI_CONS_BLK_K_UNSOL	0X10
#define PI_CONS_BLK_K_CMD_RSP	0X18
#define PI_CONS_BLK_K_CMD_REQ	0X20

/* Offsets into descriptor block */

#define PI_DESCR_BLK_K_RCV_DATA	0X0000
#define PI_DESCR_BLK_K_XMT_DATA	0X0800
#define PI_DESCR_BLK_K_SMT_HOST 0X1000
#define PI_DESCR_BLK_K_UNSOL	0X1200
#define PI_DESCR_BLK_K_CMD_RSP	0X1280
#define PI_DESCR_BLK_K_CMD_REQ	0X1300	

/* Define format of a rcv descr (Rcv Data, Cmd Rsp, Unsolicited, SMT Host) */

typedef struct
    {
    PI_UINT	long_1;
    PI_UINT	buff_lo;
    } PI_RCV_DESCR;

#define	PI_RCV_DESCR_M_SOP	0X80000000
#define PI_RCV_DESCR_M_MBZ	0X60000000 
#define PI_RCV_DESCR_M_SEG_LEN	0X1F800000
#define PI_RCV_DESCR_M_SEG_CNT	0X000F0000
#define PI_RCV_DESCR_M_BUFF_HI	0X0000FFFF

#define	PI_RCV_DESCR_V_SOP	31
#define PI_RCV_DESCR_V_MBZ	29
#define PI_RCV_DESCR_V_SEG_LEN	23
#define PI_RCV_DESCR_V_SEG_CNT	16
#define PI_RCV_DESCR_V_BUFF_HI	0

/* Define the format of a transmit descriptor (Xmt Data, Cmd Req) */

typedef struct
    {
    PI_UINT	long_1;
    PI_UINT	buff_lo;
    } PI_XMT_DESCR;

#define	PI_XMT_DESCR_M_SOP	0X80000000
#define PI_XMT_DESCR_M_EOP	0X40000000
#define PI_XMT_DESCR_M_MBZ	0X20000000 
#define PI_XMT_DESCR_M_SEG_LEN	0X1FFF0000
#define PI_XMT_DESCR_M_BUFF_HI	0X0000FFFF

#define	PI_XMT_DESCR_V_SOP	31
#define	PI_XMT_DESCR_V_EOP	30
#define PI_XMT_DESCR_V_MBZ	29
#define PI_XMT_DESCR_V_SEG_LEN	16
#define PI_XMT_DESCR_V_BUFF_HI	0

/* Define format of the Descriptor Block (resident in host memory) */

#define PI_RCV_DATA_ENTRIES	256
#define PI_XMT_DATA_ENTRIES	256
#define PI_SMT_HOST_ENTRIES	64
#define PI_UNSOL_ENTRIES	16
#define PI_CMD_RSP_ENTRIES	16
#define PI_CMD_REQ_ENTRIES	16

typedef struct
    {
    PI_RCV_DESCR  rcv_data[PI_RCV_DATA_ENTRIES];
    PI_XMT_DESCR  xmt_data[PI_XMT_DATA_ENTRIES];
    PI_RCV_DESCR  smt_host[PI_SMT_HOST_ENTRIES];
    PI_RCV_DESCR  unsol[PI_UNSOL_ENTRIES];
    PI_RCV_DESCR  cmd_rsp[PI_CMD_RSP_ENTRIES];
    PI_XMT_DESCR  cmd_req[PI_CMD_REQ_ENTRIES];
    } PI_DESCR_BLOCK;

/* Port Control Register - Command codes for primary commands */

#define PI_PCTRL_M_CMD_ERROR		0x8000
#define PI_PCTRL_M_BLAST_FLASH		0x4000
#define PI_PCTRL_M_HALT			0x2000
#define PI_PCTRL_M_COPY_DATA		0x1000
#define PI_PCTRL_M_ERROR_LOG_START	0x0800
#define PI_PCTRL_M_ERROR_LOG_READ	0x0400
#define PI_PCTRL_M_XMT_DATA_FLUSH_DONE	0x0200
#define PI_PCTRL_M_INIT			0x0100
#define PI_PCTRL_M_INIT_START		0X0080
#define PI_PCTRL_M_CONS_BLOCK		0x0040
#define PI_PCTRL_M_UNINIT		0x0020
#define PI_PCTRL_M_RING_MEMBER		0x0010
#define PI_PCTRL_M_MLA			0x0008		
#define PI_PCTRL_M_FW_REV_READ		0x0004
#define PI_PCTRL_M_DEV_SPECIFIC		0x0002
#define PI_PCTRL_M_SUB_CMD		0x0001

/* Define sub-commands accessed via the PI_PCTRL_M_SUB_CMD command */

#define PI_SUB_CMD_K_LINK_UNINIT	0X0001
#define PI_SUB_CMD_K_BURST_SIZE_SET	0X0002

/* Define some Port Data B values */

#define PI_PDATA_B_DMA_BURST_SIZE_4     0       /* Valid values for command */
#define PI_PDATA_B_DMA_BURST_SIZE_8     1
#define PI_PDATA_B_DMA_BURST_SIZE_16    2
#define PI_PDATA_B_DMA_BURST_SIZE_32    3

/* Define timeout period for all port control commands but blast_flash */

#define PI_PCTRL_K_TIMEOUT		2000		    /* millisecs */

/* Port Data A Reset state */

#define PI_PDATA_A_RESET_M_UPGRADE	0X00000001
#define PI_PDATA_A_RESET_M_SOFT_RESET	0X00000002
#define PI_PDATA_A_RESET_M_SKIP_ST	0X00000004

#define PI_PDATA_A_MLA_K_LO		0
#define PI_PDATA_A_MLA_K_HI		1

/* Port Reset Register */

#define PI_RESET_M_ASSERT_RESET		1

/* Port Status register */

#define PI_PSTATUS_V_RCV_DATA_PENDING	31
#define PI_PSTATUS_V_XMT_DATA_PENDING	30
#define PI_PSTATUS_V_SMT_HOST_PENDING	29
#define PI_PSTATUS_V_UNSOL_PENDING	28
#define PI_PSTATUS_V_CMD_RSP_PENDING	27
#define PI_PSTATUS_V_CMD_REQ_PENDING	26
#define PI_PSTATUS_V_TYPE_0_PENDING	25
#define PI_PSTATUS_V_RESERVED_1		16
#define PI_PSTATUS_V_RESERVED_2		11
#define PI_PSTATUS_V_STATE		8
#define PI_PSTATUS_V_HALT_ID		0

#define PI_PSTATUS_M_RCV_DATA_PENDING	0X80000000
#define PI_PSTATUS_M_XMT_DATA_PENDING	0X40000000
#define PI_PSTATUS_M_SMT_HOST_PENDING	0X20000000
#define PI_PSTATUS_M_UNSOL_PENDING	0X10000000
#define PI_PSTATUS_M_CMD_RSP_PENDING	0X08000000
#define PI_PSTATUS_M_CMD_REQ_PENDING	0X04000000
#define PI_PSTATUS_M_TYPE_0_PENDING	0X02000000
#define PI_PSTATUS_M_RESERVED_1		0X01FF0000
#define PI_PSTATUS_M_RESERVED_2		0X0000F800
#define PI_PSTATUS_M_STATE		0X00000700
#define PI_PSTATUS_M_HALT_ID		0X000000FF

/* Define Halt Id's */

#define PI_HALT_ID_K_SELFTEST_TIMEOUT	0
#define PI_HALT_ID_K_PARITY_ERROR	1
#define PI_HALT_ID_K_HOST_DIR_HALT	2
#define PI_HALT_ID_K_SW_FAULT		3
#define PI_HALT_ID_K_HW_FAULT		4
#define PI_HALT_ID_K_PC_TRACE		5
#define PI_HALT_ID_K_DMA_ERROR		6	/* Host Data has error reg */
#define PI_HALT_ID_K_IMAGE_CRC_ERROR    7       /* Image is bad, update it */

/* Host_Int_Enb_X [lower bits defined in Host Int Type 0 register] */

#define PI_TYPE_X_M_XMT_DATA_ENB	    0x80000000  /* Type 2 Enables */
#define PI_TYPE_X_M_RCV_DATA_ENB	    0x40000000  

#define PI_TYPE_X_M_HOST_SMT_ENB	    0x10000000	/* Type 1 Enables */
#define PI_TYPE_X_M_UNSOL_ENB	    	    0x20000000  
#define PI_TYPE_X_M_CMD_RSP_ENB		    0x08000000
#define PI_TYPE_X_M_CMD_REQ_ENB		    0x04000000
#define	PI_TYPE_X_M_RESERVED_ENB	    0x00FF0000

#define PI_TYPE_ALL_INT_DISABLE		    0x00000000
#define PI_TYPE_ALL_INT_ENB		    0xFFFFFFFF

/* Host Interrupt Type 0 */

#define	PI_HOST_INT_0_M_RESERVED	    0x0000FFC0  
#define PI_HOST_INT_0_M_20MS		    0X00000040
#define PI_HOST_INT_0_M_CSR_CMD_DONE	    0x00000020
#define PI_HOST_INT_0_M_STATE_CHANGE	    0x00000010
#define PI_HOST_INT_0_M_XMT_DATA_FLUSH	    0x00000008
#define PI_HOST_INT_0_M_NXM		    0x00000004
#define PI_HOST_INT_0_M_PM_PAR_ERR	    0x00000002
#define PI_HOST_INT_0_M_BUS_PAR_ERR         0x00000001

/* Type 1 Producer Register */

#define PI_TYPE_1_PROD_V_REARM		    31
#define PI_TYPE_1_PROD_V_MBZ_1		    14
#define PI_TYPE_1_PROD_V_COMP	            8
#define PI_TYPE_1_PROD_V_MBZ_2	 	    6
#define PI_TYPE_1_PROD_V_PROD		    0

#define PI_TYPE_1_PROD_M_REARM		    0X80000000
#define PI_TYPE_1_PROD_M_MBZ_1		    0X7FFFC000
#define PI_TYPE_1_PROD_M_COMP    	    0X00003F00
#define PI_TYPE_1_PROD_M_MBZ_2		    0X000000C0
#define PI_TYPE_1_PROD_M_PROD		    0X0000003F

/* Type 2 Producer Register */

#define PI_TYPE_2_PROD_V_XMT_DATA_COMP	    24
#define PI_TYPE_2_PROD_V_RCV_DATA_COMP	    16
#define PI_TYPE_2_PROD_V_XMT_DATA_PROD	    8
#define PI_TYPE_2_PROD_V_RCV_DATA_PROD	    0

#define PI_TYPE_2_PROD_M_XMT_DATA_COMP	    0XFF000000
#define PI_TYPE_2_PROD_M_RCV_DATA_COMP	    0X00FF0000
#define PI_TYPE_2_PROD_M_XMT_DATA_PROD	    0X0000FF00
#define PI_TYPE_2_PROD_M_RCV_DATA_PROD	    0X000000FF

/*
 * All the registers on the DEFTA adapter.
 */

struct pi_regs {
	volatile unsigned int	*port_ctrl;
	volatile unsigned int	*port_data_A;
	volatile unsigned int	*port_data_B;
	volatile unsigned int	*int_enb_X;
	volatile unsigned int	*port_reset;
	volatile unsigned int	*port_status;
	volatile unsigned int	*type2_prod;
	volatile unsigned int	*unsol_prod;
	volatile unsigned int	*cmdreq_prod;
	volatile unsigned int	*cmdrsp_prod;
	volatile unsigned int	*hostsmt_prod;
	volatile unsigned int	*host_data;
	volatile unsigned int	*intr_type0;
};

#define PORT_REGS struct pi_regs

/* Define mode base for CSR access.  The driver selects the mode    */
/*      automatically via the address bits <31:28>.                 */

#define FBUS_MODE1_BASE 0X00000800
#define FBUS_MODE2_BASE 0X00000000


/* Define Port Registers */
#ifdef mips
#define TURBO_MODE_BASE 0X00100000
#define BI_KR_PORT_RESET            (0X0000)
#define BI_KR_HOST_DATA             (0X0004)
#define BI_KR_PORT_CTRL             (0X0008)
#define BI_KR_PORT_DATA_A           (0X000C)
#define BI_KR_PORT_DATA_B           (0X0010)
#define BI_KR_PORT_STATUS           (0X0014)
#define BI_KR_HOST_INT_TYPE_0       (0X0018)
#define BI_KR_HOST_INT_ENB_X        (0X001C)
#define BI_KR_TYPE_2_PROD_REARM     (0X0020) /* for sbus only */
#define BI_KR_TYPE_2_PROD           (0X0024)
#define BI_KR_CMD_RSP_PROD          (0X0028)
#define BI_KR_CMD_REQ_PROD          (0X002C)
#define BI_KR_SMT_HOST_PROD         (0X0030)
#define BI_KR_UNSOL_PROD            (0X0034)
#endif /* mips */
/*
 * For alpha we use sparse space addressing, so the distances between each
 * csr is 8 bytes.
 */
#ifdef __alpha
#define TURBO_MODE_BASE 0X00200000
#define BI_KR_PORT_RESET            (0X0000)
#define BI_KR_HOST_DATA             (0X0008)
#define BI_KR_PORT_CTRL             (0X0010)
#define BI_KR_PORT_DATA_A           (0X0018)
#define BI_KR_PORT_DATA_B           (0X0020)
#define BI_KR_PORT_STATUS           (0X0028)
#define BI_KR_HOST_INT_TYPE_0       (0X0030)
#define BI_KR_HOST_INT_ENB_X        (0X0038)
#define BI_KR_TYPE_2_PROD_REARM     (0X0040) /* for sbus only */
#define BI_KR_TYPE_2_PROD           (0X0048)
#define BI_KR_CMD_RSP_PROD          (0X0050)
#define BI_KR_CMD_REQ_PROD          (0X0058)
#define BI_KR_SMT_HOST_PROD         (0X0060)
#define BI_KR_UNSOL_PROD            (0X0068)
#endif /* __alpha */
#endif
