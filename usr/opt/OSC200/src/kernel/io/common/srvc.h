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
 * @(#)$RCSfile: srvc.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/18 19:18:08 $
 */

/************************************************************************
 *
 *  srvc.h		Version 1.00 		April 5, 1993	
 *
 *  This file contains the definitions and data structures for the
 *  Services Device Driver (srvc) that controls access to services
 *  provided by the peripheral drivers. 
 *
 *  MODIFICATION HISTORY:
 *
 *  VERSION  DATE	WHO	REASON
 *
 *  1.00     04/05/93	deleo	Created from Service Spec.
 *
 ************************************************************************/

#ifndef _COMMON_SRVC_H_
#define _COMMON_SRVC_H_

#define SRVC_NAME_SIZE	16	/* Size of all names		*/

#ifdef KSERVICE
typedef struct srvc_head 
{
    SRVC_HEAD	*srvc_fl_head;		/* Forward link to list of */
                                        /* services */
    SRVC_HEAD	*srvc_bl_head;		/* Backward link to list of */
                                        /* services */
    SRVC_HEAD	*srvc_fl_member;	/* Forward link to list of IO */
                                        /* subsystems */
    SRVC_HEAD	*srvc_bl_member;	/* Backward link to list of */ 
                                        /* IO subsystems */
    SRVC_HEAD	*srvc_fl_driver;	/* Forward link to list of IO */
                                        /* peripheral drivers */
    SRVC_HEAD	*srvc_bl_driver;	/* Backward link to list of */
    SRVC_HEAD	*srvc_parent;		/* Direct link ot parent */
    U64		flags;			/* Generic flags */
    U64		srvc_dispatches;	/* Number of dispatches */ 
                                        /* through this service/IO */
                                        /* subsystem/driver */ 
    void	(*srvc_dispatch)();	/* Driver dispatch entry */ 
                                        /* point */
    char	srvc_name[SRVC_NAME_SIZE];	/* ASCII Service name */
    char	srvc_io[SRVC_NAME_SIZE];	/* ASCII IO subsystem name */
    char	srvc_drvr[SRVC_NAME_SIZE];	/* ASCII peripheral driver */
                                        /* name */
    lck_t	srvc_lck;		/* SMP lock structure */
} SRVC_HEAD;



/*
 * flags field defines.
 */
#define SRVC_SERVICE	0x01		/* Indicates that the */
                                        /* structure contains info */
                                        /* for a Service. */
#define SRVC_IO		0x02		/* Indicates that the */
                                        /* structure contains info */
                                        /* for an IO subsystem. */
#define SRVC_DRVR	0x04		/* Indicates that the */
                                        /* structure contains info */
                                        /* for an IO device driver. */


#endif

typedef struct srvc_req
{
   U64		srvc_op;		/* Service request opcode */
   char		srvc_name[16];		/* Service name.  15 chars */
                                        /* plus null termination */
   char		srvc_io[16];		/* IO subsystem name.  15 */
                                        /* chars plus null */ 
                                        /* termination */
   char		srvc_drvr[16];		/* Drvier name.  15 chars */
                                        /* plus null termination */
   U64		srvc_flags;		/* Service flags */
   dev_t	srvc_dev;		/* device type of device */
   char		*srvc_buffer;		/* User's buffer address */
                                        /* that contains Service */
                                        /* specific info that is */
                                        /* intepreted by the device */
                                        /* driver. */
   U64		srvc_len;		/* length of srvc_buffer */
                                        /* in bytes */
   int		srvc_errno;		/* Error number.  Valid if */ 
                                        /* error bit set in flags */ 
                                        /* field */
   char		srvc_str[128];		/* String area.  Valid if */
                                        /* string valid bit set in */
                                        /* flags field.   Mechanism */
                                        /* to allow more descriptive */
                                        /* error notification. */
} SRVC_REQ;

/*
 * Service flag field bit definitions.
*/
#define	STR_MSG_VALID	0x0000000000000001	/* String area valid. */
#define	SRVC_ERROR   	0x8000000000000000      /* Error and errno is valid. */

/*
 * Service Request Opcode definitions.
*/
#define	SRVC_DISPATCH	0x01		/* Service Dispatch function */
#define GET_NUM_SRVCS	0x02		/* Get the number of services */
                                        /* function */
#define GET_SRVC_LIST	0x03		/* Get the list of service */ 
                                        /* names function */
#define GET_SPECIFIC_NUM_SRVC	0x04	/* Get the number of a */
                                        /* specific service function */
#define GET_IO_LIST	0x05		/* Get the list of IO service */
                                        /* names function */
#define GET_NUM_DRVRS	0x06		/* Get the number of drivers */
                                        /* for service/IO subsystem */
                                        /* function */
#define GET_DRVR_LIST	0x07		/* Get the list of driver */ 
                                        /* names for service/IO  */ 
                                        /* subsystem  function */ 

/* 
 * Service string contents
 */
#define NSTD_RAID_SERVICE "NSTD_RAID"
#define SCSI_IO_SUB "SCSI"
#define DISK_DRIVER "DISK"
/*
 * Service Request packet's service buffer contents definitions
 * Each Service Request opcode has unique service buffer contents.
 */

/* GET_NUM_SRVCS Service Request Opcode */

typedef struct num_srvcs
{
   U64		get_srvc_num;		/* The number of services */
                                        /* registered in the */ 
                                        /* system */
} NUM_SRVC;


/* GET_SRVC_LIST Service Request Opcode */

typedef struct srvc_list
{
   char		srvc_name[16];
} SRVC_LIST;


/* GET_SPECIFIC_NUM_SRVC Service Request Opcode */

typedef struct num_specific
{
   U64		get_spec_num;
} NUM_SPEC;


/* GET_IO_LIST Service Request Opcode */

typedef struct list_io
{
   char		io_name[16];
} LIST_IO;


/* GET_NUM_DRVRS Service Request Opcode */

typedef struct io_drvrs
{
   U64		num_drivers;
} IO_DRVRS;


/* GET_DRVR_LIST Service Request Opcode */

typedef struct list_drvrs
{
   char		drvr_name[16];
} LIST_DRVRS;

#endif
