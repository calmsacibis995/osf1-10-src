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
 *      @(#)$RCSfile: binlog.h,v $ $Revision: 1.1.2.6 $ (DEC) $Date: 1992/06/25 22:30:39 $
 */

/* 
 * logs from origional file location (kernel/sys/binlog.h)
 *
 * 	Decided, from code review, to remove open and seqnum fields from
 * 	struct binlog_getstatus for security reasons.
 * 	[92/02/19  13:25:24  Scott_Cranston]
 * 
 * 	Added an 'L' to the value for BUFMIN and BUFMAX as these are now used
 * 	with a long data type.
 * 	[92/02/07  12:45:07  Scott_Cranston]
 * 
 * 	  -  deleted class code ELCT_EXPTFLT... was VAX specific.
 * 
 * 	  -  in struct binlog_bufhdr changed int's to long's so that the
 * 	     data structure would be sized for machine architecture boundaries.
 * 	[92/02/07  09:54:36  Scott_Cranston]
 * 
 * 	Make some items unsigned.
 * 	[92/01/28  18:54:23  Scott_Cranston]
 * 
 * 	  -  changed some long's to int's for 64-bit cleanliness.
 * 
 * 	  -  moved rhdr_sid and rhdr_elver so they would be a constant
 * 	     offset from start of header regardless of 32/64 bit arch.
 * 	     This is to help the report generator easly find this info
 * 	     so it can determine the machine arch.
 * 	[92/01/28  17:46:55  Scott_Cranston]
 * 
 * 	bcreate'ed file worng and had an 'rcsid'.
 * 	[92/01/15  08:57:40  Scott_Cranston]
 * 
 * 	Initial file creation.
 * 	[92/01/14  15:59:52  Scott_Cranston]
 * 
 */

#ifndef __BINLOG__
#define __BINLOG__ 


/*
 *  This file contains the common components of a binary event log record.  
 *  The data structures and ID codes to be used are defined here.
 *
 *  This file also contains the ID codes used by/with the kernel binary
 *  event logging routines.
 *
 *  The following is a template and description for what a binary event 
 *  record looks like....
 *
 *          struct el_rec {
 *                  struct el_rhdr elrhdr;            ** mandatory component **
 *                  struct el_sub_id elsubid;         ** mandatory component **
 *                  struct EVENT-SPECIFIC-DATA
 *                  char eltrailer[EL_TRAILERSIZE];   ** mandatory component **
 *          };
 *
 *  The mandatory components must be in every event record and must be named
 *  as shown above.  The sequence of the components of an event record must
 *  be as shown.
 *
 ***************
 *
 *  The component 'elrhdr' is the primary header of the event record.  It
 *  contains vendor independent data suppled by the kernel.  The data is
 *  generic to the event being logged and does not describe the event itself.
 *  The data in elrhdr is ONLY manipulated by the kernel binary event
 *  logging routines.  An entity logging an event does not touch any element
 *  of elrhdr, doing so could produce unpredictable results.  The elrhdr 
 *  data structure is composed of the elements:
 *
 *        rhdr_reclen  -  The length in bytes of the total event record
 *                        being logged.  This is inclusive of 'elrhdr',
 *                        'elsubid', 'eltrailer' and of course the event
 *                        specific data.
 *
 *        rhdr_seqnum  -  The sequence number of the event.  When the system
 *                        boots the sequence number is initailized to start
 *                        at zero (0).  The sequence number is incremented
 *                        for each event that is attempted to be logged. 
 *
 *        rhdr_time    -  This is the time at which the event record was
 *                        created, effectively the time at which the event
 *                        occured.
 *
 *        rhdr_sid     -  The system (CPU type) identifer for the processor
 *                        which is logging the event.  This is defined as:
 *
 *                          rhdr_sid
 *                            bits                definition
 *                          --------   --------------------------------------
 *                            31:16    machine_slot[].cpu_subtype, bits <15:00>
 *                            15:00    machine_slot[].cpu_type, bits <15:00>
 *
 *                         The index into machine_slot[] is obtained by
 *                         using cpu_number().  See mach/machine.h for further
 *                         details.
 *
 *        rhdr_valid   -  A flag that signals that the event record is valid.
 *                        This is either EL_INVALID or EL_VALID.
 *
 *        rhdr_pri     -  The priority level (severity) of the event being 
 *                        logged.  Allowed values are:
 *
 *                            EL_PRISEVERE - An unrecoverable/fatal error
 *                                           condition.  System or device
 *                                           operation can not continue.
 *                                           Immediate attention is required.
 *
 *                            EL_PRIHIGH   - A successfully recovered error
 *                                           condition.  An error/problem was
 *                                           detected with the system or device
 *                                           which has been corrected or is not
 *                                           fatal.  Attention to the situation
 *                                           will be needed as soon as possible.
 *
 *                            EL_PRILOW    - Non critical events, can be of an
 *                                           informational nature only.  
 *                                           Attention to the stituation can
 *                                           be given on an as needed basis.
 *
 *        rhdr_elver   -  The verision of the event record format. This is
 *                        set to ELVER.
 *
 *        rhdr_hname   -  The hostname of the system logging the event.
 *
 *        rhdr_systype -  Not used, always zero (0).
 *
 *        rhdr_mpnum   -  The number of active processors on the system.
 *                        This is set equal to machine_info.avail_cpus (see
 *                        mach/machine.h).
 *
 *        rhdr_mperr   -  The slot number of the CPU logging the event.
 *                        The slot number is obtained by using cpu_number();
 *
 *************
 *
 *  The component 'elsubid' is a sub header and contains data specific to 
 *  the event being logged.  The entity logging the event supplies the
 *  data for elsubid.  The LSUBID macro is the recomended programming interface
 *  to use for doing this.  The elsubid data structure is composed of
 *  the elements:
 *
 *        subid_class  -  A code identifing the top level catagory  that
 *                        the event being logged fits into.  These 'class'
 *                        are common across all vendors and architectures.
 *                        For example there are class codes for disk, tape,
 *                        ascii messages, etc.  The class codes are defined 
 *                        in this file.  The subid_class code is mandatory.
 *                        
 *        subid_type   -  An identifier code that is unique to the event
 *                        being logged.  This code provides a finner
 *                        ganularity description of the event than the
 *                        subid_class code does.  For example if subid_class
 *                        specifies disk, than subid_type could specify
 *                        SCSI.  These codes are to be #define'ed in a vendor
 *                        supplied unique header file.  The subid_type is 
 *                        optional and should then be set to EL_UNDEF.
 *
 *         subid_ctldevtype  -
 *                        A identifer code that specifies what the specific
 *                        kind of entity it is that is logging the event.
 *                        For a hardware device this would typically
 *                        identify the model or name.  These codes are 
 *                        to be #defined'ed in a vendor supplied unique header
 *                        file. The subid_ctldevtype is optional and should
 *                        then be set to EL_UNDEF.
 *
 *   Note:   The subid_class, subid_type, and subid_ctldevtype codes, when
 *           used together, uniquely identify what a given event record
 *           is logging.  Specificly, it uniquely identifies what the data
 *           structure is that contains the event specific data.  This
 *           is required so that the binary event log report generator
 *           (bit-to-text) tool can determine how to interpret the event
 *           specific data.... this is vendor dependent data.  
 *
 *           Mininally the subid_class is required to be used.  Depending
 *           upon the event specific data being logged either subid_type
 *           or subid_ctldevtype (or both) may not be required.  For example
 *           if subid_class is ELMSGT_SU (a startup event) than both
 *           subid_type and subid_ctldevtype can be EL_UNDEF.  In this
 *           example the class code ELMSGT_SU is sufficent for uniquely
 *           describing what is being logged.
 *           
 *
 *         subid_num   -  The number identifing the CPU, bus, controller or
 *                        adapter on which the event being logged occured. 
 *                        This item is not applicable in all cases or may
 *                        be unknown and is then set to EL_UNDEF.
 *
 *         subid_unitnum  -
 *                        The physical unit number of the device on which the
 *                        event being logged occured.  This item is not 
 *                        applicable or may be unknown and is then set 
 *                        to EL_UNDEF.
 *                        
 *                        
 *   Note:   The usage of subid_num and subid_unitnum is not clear in all
 *           cases and can be confusing.  Perhaps an example will clarify:
 *
 *               A disk subsystem consists of a contoller (connected to the
 *               system bus) and the disk drive itself.  Take for example
 *               a system with two SCSI disk contollers (0 & 1) with
 *               one disk on each contoller, physical unit numbers 4 & 6
 *               respectively.  Suppose a nonexistent memory error occured 
 *               during a DMA write to disk unit number 6.  This is a
 *               contoller problem, not a disk problem.  In this case 
 *               subid_num would be set to '1' and subid_unitnum would
 *               be EL_UNDEF.  Or, suppose a CRC error occured on a read
 *               from disk unit 4.  This is a disk error and subid_num would
 *               be set to '0' and subid_unitnum would be set to 4.
 *                        
 *           In some cases the event specifc data protion of the event
 *           record may describe the identifing numbers.  In this case
 *           subid_num and subid_unitnum may optionally be set to EL_UNDEF.
 *
 *
 *         subid_errcode  -
 *                        A code that summarizes what the eventbeing logged
 *                        is.  This is optional and may be set to EL_UNDEF.
 *
 ************
 *
 *  The 'struct EVENT-SPECIFIC-DATA' is data structure describing the event
 *  specific data being logged.  These data structures and any associated
 *  #define's should go into a vendor unique binary event logging header 
 *  file.  Generally you will #include binlog.h into this file.
 *  This file should define all the event specific data structures, 
 *  subid_type codes and subid_ctldevtype codes unique to a specific vendors
 *  needs.  At the end of this vendor unique file you may wish to do
 *  something like:
 *
 *          struct el_rec {
 *                  struct el_rhdr elrhdr;
 *                  struct el_sub_id elsubid;
 *                  union {
 *                           PUT-ALL-YOUR-EVENT-SPECIFIC-DATA-STRUCTURES-HERE
 *                           struct el_msg elmsg;      ** an example **
 *                   } el_body;
 *                  char eltrailer[EL_TRAILERSIZE];    ** mandatory component **
 *          };
 *   
 *
 *  Using the union 'el_body' may make things more convenient to use when
 *  coding up the actual binary event logging.  The struct el_msg is shown
 *  here as an example.  The el_msg is used by the kernel binary event 
 *  logging facility to capture formated kernel ASCII messages (printf) into 
 *  the binary event log.  It can also be used anywhere it is desired to put 
 *  a formatted ASCII into the binary event log.
 *
 ************
 *
 *  The eltrailer[] uniquely marks the end of an event record.  The purpose
 *  of this is to allow an application to easily search backward through a
 *  file of binary event log records (for example the binary event log file).
 *  Doing a binary search through such file is even aided by having the 
 *  trailer.  Once a trailer is identified the begining of a new event record
 *  would follow.
 *
*/



/***********  START OF class codes  (el_sub_id.subid_class)  ***************/

/*       Note:  Please give careful consideration to adding a new
 *              class code.  Be very certain that existing ones can not
 *              serve the purpose when used with appropriate 
 *              el_sub_id.subid_type and el_sub_id.subid_ctldevtyp codes.
 *
 *       Note:  For class codes labeled as an "ascii message" the data
 *              structure el_msg is always to be used to log the ascii
 *              message with.
*/

/* Hardware related events 100-199 */
#define ELCT_MCK	100		/* CPU machinechecks & exceptions */
#define ELCT_EXPTFLT	100		/* Generalized exception/ */
					/* fault. Distinguished by */
					/* subid type and subid */
					/* ctldevtyp */
#define ELCT_MEM	101		/* memory errors soft/hard */
#define ELCT_DISK	102		/* disk errors */
#define ELCT_TAPE	103		/* tape errors */
#define ELCT_DCNTL	104		/* device controller errs */
#define ELCT_ADPTR	105		/* adapter errs */
#define ELCT_BUS	106		/* bus errs */
#define ELCT_SINT	107		/* stray intr. */
#define ELCT_CTE	108		/* console event */
#define ELCT_STKDMP	109		/* stack dump */
#define ELCT_STATE	110		/* generalized machine state. */
					/* Distinguished by subid type */
					/* and subid ctldevtyp */
#define ELCT_CAM_ERRPKT 199             /* SCSI CAM Event */


/* Software detected events 200-249 */
#define ELSW_CIPPD      201                     /* ci ppd events */
#define	ELSW_SCS	202			/* scs events */


/* Informational ascii messages 250-299 */
#define ELMSGT_INFO	250			/* generic info ascii message */


/* Operational Class 300-400 */


/* reload/restart 300-350 */
#define ELMSGT_SU	300			/* start up ascii message */
#define ELMSGT_SD	301			/* shutdown ascii message */
#define ELMSGT_PANIC    302                     /* ascii panic message */

/* time stamp 310 */
#define ELMSGT_TIM	310			/* time stamp */


/* usage 311-315 */

/* statistics 316-319 */

/* maintenance events 350-399 */
#define ELMSGT_DIAG	350			/* diag ascii message */
#define ELMSGT_REPAIR   351			/* repair ascii message */


/************************   END OF subid_class CODES  ************************/






#define BINLOG_MAGIC    0x063061L      /* kernel buffer valid indicator */

/* minimum & maximum configurable sizes of the kernel binary event log buffer */
#define BINLOG_BUFMIN    8192L   /* min size of kernel event log buffer, 8Kb */
#define BINLOG_BUFMAX   49152L   /* max size of kernel event log buffer, 48Kb */


/* sc_state bit masks */
#define BINLOG_ON         0x1     /* binary event logging enabled */
#define BINLOG_ASCIION    0x2     /* ASCII msg binary event logging enabled */
#define BINLOG_BUFALLOCED 0x4     /* binary log buffer has been allocated */
#define BINLOG_RDWAIT     0x10    /* readers waiting */


/* binlog_alloc() return values */
#define BINLOG_NOBUFAVAIL  (char *)0    /* no buf space available */


/* sizes of mandatory parts of an event record */
#define EL_RHDRSIZE	sizeof(struct el_rhdr)
#define EL_SUBIDSIZE	sizeof(struct el_sub_id)
#define EL_MISCSIZE  	(EL_RHDRSIZE + EL_SUBIDSIZE + EL_TRAILERSIZE)
#define EL_TRAILERSIZE  4L
#define trailer "%~<^"


/* misc event record codes */
#define EL_INVALID 	0L      /* event record is not valid */
#define EL_VALID 	1L      /* event record is valid */
#define EL_UNDEF	-1L     /* unused element of a data structure */


/* event record prioroty (severity) levels (el_rhdr.rhdr_pri) */
#define EL_PRISEVERE	1
#define EL_PRIHIGH	3
#define EL_PRILOW	5


/* misc sizes */
#define EL_STKDUMP	512L
#define EL_SIZE512	512L
#define EL_SIZE256	256L
#define EL_SIZE128	128L
#define EL_SIZE64	64L
#define EL_SIZE16	16L
#define EL_SIZE12	12L
#define EL_SIZE2048	2048L
#define EL_SIZE4096	4096L
#define EL_SIZEAPPND	1400L


#define ELVER 5          /* event log record format version */


/*
 * Keep this data structure sized for machine architecture boundaries.
 * This makes the start of the data area (which is right after this data 
 * structure and is pointed to by binlog_bufhdr.in) aligned appropriately.
 */
struct binlog_bufhdr {              /* Kernel binary event log buffer header */
          unsigned long  size;      /* size of buffer - for savecore */
	  unsigned long  magic;     /* validity flag */
	  unsigned long  pending;   /* event records in process count */
	  char           *in;       /* pointer to start of current input spot */
          char           *out;      /* pointer to start of output data */
	  char           *le;       /* pointer to logical end of buffer */
};

#define BUFHDRSIZE     sizeof(struct binlog_bufhdr)  /* size of buffer header */


/* BINLOG_GETSTATUS ioctl state/status info for binary event logger */
struct binlog_getstatus {  
       unsigned int   sc_state;        /* state flags */
       unsigned int   sc_size;         /* size of event log buffer */
       unsigned int   sc_nobufavail;   /* no buffer available counter */
       unsigned int   sc_badsize;      /* bad record size counter */
       unsigned int   sc_readbusy;     /* log busy on read counter */
};



/* programming interface for filling struct el_sub_id */
#define LSUBID(eptr, class, type, ctldev, num, unitnum, errcode) \
	eptr->elsubid.subid_class  = class; \
	eptr->elsubid.subid_type = type; \
	eptr->elsubid.subid_ctldevtyp = ctldev; \
	eptr->elsubid.subid_num = num; \
	eptr->elsubid.subid_unitnum = unitnum; \
	eptr->elsubid.subid_errcode = errcode;


struct el_rhdr {			/* errlog primary header */
	unsigned short rhdr_reclen;	/* errlog record length */
	unsigned short rhdr_seqnum; 	/* seq. number */
	unsigned int rhdr_sid;		/* cpu subtype & type */
	unsigned char rhdr_valid;	/* valid error record */
	unsigned char rhdr_pri;		/* priority hi - low */
	unsigned short rhdr_elver;	/* errlog version,filled in by elcs */
	int rhdr_time;		        /* time in sec event logged */
	char rhdr_hname[EL_SIZE12];	/* host name, filled in by daemon */
	unsigned int rhdr_systype;	/* system type register */
	unsigned int rhdr_mpnum;       	/* number of processors in system */
	unsigned int rhdr_mperr;       	/* which cpu serviced the error */
};

struct el_sub_id {			/* errlog secondary header */
	unsigned short subid_class;	/* class type, ELCT_DISK */
	unsigned char subid_type;	/* error type, ELDEV_MSCP */
	unsigned char subid_ctldevtyp;	/* controller/device type, ELDT_RA60 */
	unsigned short subid_num;	/* cpu number - mchk errors */
					/* bus number - bus/adptr/cntr errors*/
					/* cntl number - device errors */
	unsigned short subid_unitnum;	/* adpt number - adapter errors */
					/* cntl number - controller errors */
					/* unit number - device errors */
	unsigned int subid_errcode;	/* ci,mck summ.,cte,emm,exptflt */
	unsigned int subid_notused;     /* padding to 64bit alignment */
};



/******  COMMON (VENDOR INDEPENDENT) EVENT SPECIFIC DATA DEFINITIONS *********/
/* alpha note:
 * The code referencing msg_len uses ints.  We make it a long here to
 * guarantee 64 bit alignment of msg_asc;
 */

struct el_msg {				/* ascii text message */
	unsigned int msg_type;
	unsigned int msg_len;		/* actual length of string */
	char msg_asc[1024];		/* the ascii string */
};


#endif /* __BINLOG__ */
