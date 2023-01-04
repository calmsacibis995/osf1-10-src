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
 * @(#)$RCSfile: latcp.h,v $ $Revision: 1.1.11.2 $ (DEC) $Date: 1993/07/16 17:58:24 $
 */

#define    	CPSTART            0x1
#define    	CPSTOP             0x2
#define    	CPSHOW_CHAR        0x3
#define    	CPSHOW_SRVR        0x4
#define   	CPSHOW_CB_CTR      0x5
#define 	CPZERO_CTR         0x6
#define    	CPSET_MULTI        0x7
#define    	CPSET_SVC          0x8
#define    	CPDEL_SVC          0x16
#define    	CPSET_GRPS         0x9
#define    	CPDEL_GRPS         0xa
#define    	CPSET_RATING       0xb
#define   	CPDEL_PRT          0xe
#define  	CPSET_PRT          0xf
#define    	CPSHOW_PRT         0x10
#define    	CPSET_NODE         0x11
#define    	CPSHOW_NODE        0x12
#define    	MASK               0x7
#define    	OPTSTR 		   "shNSCzg:G:m:n:Aa:oi:x:DPp:LIH:R:V:QE:e:dr"

/* data structures and defines used by latcp and latioctl */
#define 	MAX_LINES 	256    /* max possible entries recv'd from driver */
#define 	APPL_DISP 	-1      /* application port display only */
#define 	INTER_DISP 	-2      /* interactive port display only */
#define 	ALL_DISP 	-3      /* display all ports */
#define 	MAX_SLOTS       2040    /* max # of total slots available to use */
#define 	MAX_SVCS	8       /* max # of services */
#define 	MAX_POSS_TTY	620     /* size of tty name range tty00-tty9Z */
#define 	DYNAMIC_RATING 	-1	/* init value for static value */    
#define 	MAX_STAT_RATING 255	/* max static rating 0 - 255, -1 dynamic  */
#define 	MIN_MULTI_TIMER 10	/* in seconds */
#define 	MAX_MULTI_TIMER 180 	/* in seconds */
#define 	EBADSLT 55      	/* ????? not define in errno.h  */
#define 	LAT_SW_VERSION  5	/* current lat protocol version */
#define 	LAT_ECO		1 	/* level of engineering */
#define 	LAT_MAXGROUPS   32      /* max char allowed for groups */
#define 	MAXGROUPSNUMB   255     
#define 	LAT_DEFAULTGROUP   1   /* for group 0 */
#define 	SHOW_BOTH_PRT	0
#define 	SHOW_APPL_PRT 	1
#define 	SHOW_INTR_PRT 	2
#define 	TERMINAL_FOUND  0 

#define 	MAX_SERVICES 	8 	/* max number of service in one node */

#define 	LAT_MAJOR       5	


/*defines from kar */
#define	   OSNAME             "DEC OSF/1 Version "
#define	   SVCID              " LAT SERVICE"
#define	   NODEID             " LAT NODE"
#define    LATCPDEV           "/dev/lat"
#define    KINFODEV           "/dev/streams/kinfo"
#define    LATCPDEV_LEN       24
#define    LAT_NODE_LEN       6
#define    CPSET_ADAPT        0x13
#define    CPDEL_ADAPT        0x14
#define    CPRESET            0x15


#define    LAT_MULTI_TIMER   60
#define    MAX_HOSTS          255
