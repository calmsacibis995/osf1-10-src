#ifndef _dwc_db_work_structures_h_
#define _dwc_db_work_structures_h_
/* $Id$ */
/* dwc_db_work_structures.h */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; database access routines
**
**  AUTHOR:
**
**	Per Hamnqvist, November 1987
**
**  ABSTRACT:
**	This module defines the datastructures (in memory) used by
**	the database access routines.
**
**--
*/

#include <stdio.h>	/* FILE */
#include <stddef.h>	/* offsetof */
#if !defined(offsetof)
#define offsetof(s_name,m_name) (size_t)&(((s_name*)0))->m_name
#endif

/*
** Old style day marks (and flag indicating it is a new daymark)
*/
#define DWC$k_mark_default 0            /*  Default */
#define DWC$k_mark_work_day 1           /*  Working day */
#define DWC$k_mark_nonwork 2            /*  Non working day */
#define DWC$k_mark_special 3            /*  Special day */
#define DWC$m_mark_new_style 4          /*  Flag indicating it is */
					/*  a new daymark */
#define DWC$m_mark_type_mask 3          /*  Bits that define day */
					/*  type */

/*
** The virtual record header is the structure that is prefixed in front of
** a database record when the database record is in memory.
*/
struct DWC$db_vm_record
{
    struct DWC$db_vm_record *DWC$a_dbvm_vm_flink; /* Buffer Forward link */
    struct DWC$db_vm_record *DWC$a_dbvm_vm_blink; /* Buffer Backward link */
    struct DWC$db_vm_record *DWC$a_dbvm_lru_flink; /* Cache specific flink  */
    struct DWC$db_vm_record *DWC$a_dbvm_lru_blink; /* Cache specific blink  */
    unsigned long int DWC$l_dbvm_rec_addr; /* Disk address of this record */
    char (*(*DWC$a_dbvm_parent_vm));    /* Pointer to parent field */
    char *DWC$a_dbvm_special;           /* Record specific pointer */
    unsigned long int DWC$l_dbvm_rec_len; /* Length of record, in bytes */
    unsigned long int DWC$l_dbvm_special2; /* Block specific field #2 */
    unsigned char DWC$b_dbvm_special3;  /* Block specific field #3 */
    unsigned char DWC$b_dbvm_special4;  /* Block specific field #4 */
    unsigned short int DWC$w_dbvm_special5; /* Block specific field #5 */

/** here is where we handle the problems with alignment that might occur on **/
/** Alpha osf/1 **/
#if LONG_MAX!=INT_MAX
    unsigned int filler_space;
#endif

/*
** The following define is modified version of offsetof without the ampersand
** because taking the address of the DWC$t_dbvm_data array was causing a 
** compiler warning.
*/
#define DWC$K_VM_HEADER \
    ((size_t)(((struct DWC$db_vm_record *)0)->DWC$t_dbvm_data))
#define DWC$K_VM_LRU_OFFSET \
    (offsetof(struct DWC$db_vm_record, DWC$a_dbvm_lru_flink))

  
/*
** Please note that I can make the below statement because I know that
** the following field will not cause a realignment (as the previous
** field is already aligned). It should also be noted that the data
** field is not part of the header itself.
*/
    char DWC$t_dbvm_data [1];           /* Start of user data */
};

#define DWC$k_dbvm_max_cache 1000       /* Max elements in cache */

/*
** This structure describes the information maintained about each pending
** alarm. This structure is linked into a doubly linked list, with a head
** in the Cab.
*/
struct DWC$db_alarm
{
    struct DWC$db_alarm *DWC$a_dbal_flink; /* Next alarm */
    struct DWC$db_alarm *DWC$a_dbal_blink; /* Previous alarm */
    unsigned long int DWC$l_dbal_org_day; /* Originating day */
    unsigned short int DWC$w_dbal_org_min; /* Originating minute */
    short int DWC$w_dbal_id;            /* Id within org day */
    unsigned long int DWC$l_dbal_trigg_day; /* Triggering day */
    unsigned short int DWC$w_dbal_trigg_min; /* Triggering minute */
    unsigned short int DWC$w_dbal_alarm_idx; /* Index of alarm in item */
};

/*
** This control structure maintains (pointer) information about one repeat
** expression. It is linked off of the Cab.
*/
#define DWC$m_dbrc_daytype 255          /* Bits containing */
					/*  day type */
#define DWC$m_dbrc_valbits 1023         /* Mask of used bits in */
					/*  daycond field */
#define DWC$m_dbrc_movecond 768         /* Move condition bits */

struct DWC$db_repeat_control
{
    struct DWC$db_repeat_control *DWC$a_dbrc_flink; /* Next repeat control  */
						    /* block */
    struct DWC$db_repeat_control *DWC$a_dbrc_blink; /* Previous repeat	    */
						    /* control */
    struct DWC$db_vm_record *DWC$a_dbrc_vector_block; /* Home vector block  */
    struct DWCDB_repeat_expr *DWC$a_dbrc_vector_vm; /* Pointer to repeat */
						    /* expression itself */
    unsigned char DWC$b_dbrc_type;      /* Repeat type */
    unsigned char DWC$b_dbrc_base_month; /* Base month for repeat */
    unsigned char DWC$b_dbrc_month_int; /* Month interval */
    unsigned char DWC$b_dbrc_weekday;   /* Weekday */
    unsigned long int DWC$l_dbrc_n;     /* Type specific interval */
    short int DWC$w_dbrc_id;            /* Unique identification */
    unsigned short int DWC$w_dbrc_daycond; /* Day condition */
};


/*
** This structure maintains the information about one VMS style error
** message. This structure is linked into a queue of alike, hanging off
** the Cab.
*/
struct DWC$db_error_buffer
{
    struct DWC$db_error_buffer *DWC$a_dbeb_flink; /* Next error buffer */
    struct DWC$db_error_buffer *DWC$a_dbeb_blink; /* Previous error buffer  */
    unsigned long int DWC$l_dbeb_ecode; /* Errorcode */
    char *DWC$a_dbeb_eid;               /* Error id text */
    char *DWC$a_dbeb_eptr;              /* ASCIZ error text */
};

/*
** This structure defines one exception day. This is linked off the exception
** head.
*/
struct DWC$db_exception_day
{
    struct DWC$db_exception_day *DWC$a_dbed_flink; /* Flink to next	    */
						   /* exception */
    struct DWC$db_exception_day *DWC$a_dbed_blink; /* Previous exception */
    unsigned long int DWC$l_dbed_day;   /* Excepted day */
};

/*
** This structure defines a saved record number. When repeat expression
** exceptions are read into memory, their associated block's record numbers
** are saved to speed up later purge and deletes.
*/
struct DWC$db_saved_record
{
    struct DWC$db_saved_record *DWC$a_dbdr_flink; /* Next saved record */
    struct DWC$db_saved_record *DWC$a_dbsr_blink; /* Previous saved record  */
    unsigned long int DWC$l_dbsr_record_addr; /* Record number */
};

/*
** This structure is the main exception block. It is pointed at by the
** repeat expression block if the exceptions have been loaded. The later
** can be determined by the fact that the exception pointer is a virtual
** pointer rather than a record address.
*/
struct DWC$db_exception_head
{
    struct DWC$db_exception_day *DWC$a_dbeh_flink; /* Flink to next	    */
						   /* exception */
    struct DWC$db_exception_day *DWC$a_dbeh_blink; /* Previous exception */
    unsigned long int DWC$l_dbeh_first_rec; /* Record for first repeat */
					    /* exception vector */
    struct DWCDB_repeat_exceptions *DWC$a_dbeh_work_buff; /* Pointer to    */
							   /* work buffer */
    unsigned long int DWC$l_dbeh_work_rec; /* Record addr of current */
    struct DWC$db_saved_record *DWC$a_dbeh_rec_flink; /* Next saved record  */
						      /* no */
    struct DWC$db_saved_record *DWC$a_dbeh_rec_blink; /* Previous saved	    */
						      /* record no */
    unsigned long int DWC$l_dbeh_rec_count; /* Count of records */
};

/*
** This is a parsed token block. This structure is linked to the structure
** DWC$db_interchange.
*/
struct DWC$db_itoken
{
    struct DWC$db_itoken *DWC$a_itok_flink; /* Next token */
    struct DWC$db_itoken *DWC$a_itok_blink; /* Previous token */
    char *DWC$a_itok_message;           /* Data associated with token */
    unsigned long int DWC$l_itok_msglen; /* Length of data */
    unsigned long int DWC$l_itok_day;   /* DWC day for item */
    unsigned short int DWC$w_itok_start; /* Starting minute */
    unsigned short int DWC$w_itok_duration; /* Length in minutes */
    unsigned char DWC$b_itok_mode;      /* Data hint */
};

/*
** This is the head for the interchange of data. It contains the queue head
** for the items that have been parsed.
*/
struct DWC$db_interchange
{
    struct DWC$db_itoken *DWC$a_dbin_flink; /* Next token */
    struct DWC$db_itoken *DWC$a_dbin_blink; /* Previous token */
    struct DWC$db_itoken *DWC$a_dbin_next; /* Next for Get_i_item */
    unsigned long int DWC$l_dbin_line;  /* Failing line for parse */
    unsigned long int DWC$l_dbin_dwcday; /* Curren DWC day */
    char *DWC$a_dbin_ntoken;            /* Pointer to next char to */
					/*  parse */
    void *DWC$a_xmstring_context;
};

/*
** This is the calendar database access block. It is used to maintain context
** in between calls to the database acccess routines.
*/
#define DWC$m_dcab_write 1              /*  File open for write */
#define DWC$m_dcab_rscan 2              /*  Rangemap scan in progress */
#define DWC$m_dcab_alarm_ld 4           /*  Initial Alarm load done */
#define DWC$m_dcab_dtcvalid 8           /*  Day Type cache is valid */
#define DWC$m_dcab_rptdisable 16        /*  Repeating event triggering	    */
					/*  disabled */
#define DWC$K_CAB_SIZE 351

struct DWC$db_access_block
{
    unsigned long int DWC$l_dcab_flags; /* Flags : */

/*
**    General file parameters
*/
    FILE			*DWC$l_dcab_fd;		/* File descriptor */
    char			*DWC$a_dcab_filename;	/* Pt to ASCIZ fname */
    unsigned long int		DWC$l_dcab_eof;		/* End of file */
    struct DWC$db_vm_record	*DWC$a_dcab_header;	/* Pointer to header */
							/* block */
    struct DWC$db_vm_record	*DWC$a_dcab_range_flink; /* Flink to next VA */
							/* rangemap */
    struct DWC$db_vm_record	*DWC$a_dcab_range_blink; /* Blink to last VA */
							/* rangemap */
    struct DWC$db_vm_record	*DWC$a_dcab_bitmap_flink; /* Flink to first */
							/* Bitmap */
    struct DWC$db_vm_record	*DWC$a_dcab_bitmap_blink; /* Blink to last */
							/* Bitmap */

/*
**   Error handling
*/
    unsigned long int DWC$l_dcab_errlev; /* Current error depth */
    unsigned long int DWC$l_dcab_errstack [5]; /* Error stack */
    struct DWC$db_error_buffer *DWC$a_dcab_eflink; /* First error buffer */
    struct DWC$db_error_buffer *DWC$a_dcab_eblink; /* Last error buffer */
    unsigned long int DWC$l_dcab_vaxc;  /* Latest saved VAX-C error code */
    unsigned long int DWC$l_dcab_errno; /* Latest saved errno */
    int *DWC$a_dcab_ecallback;          /* Last chance callback routine */
    unsigned long int DWC$l_dcab_callparam; /* Callback param for rtn */

/*
**   Virtual buffer management
*/
    char *DWC$a_dcab_write_buff;        /* Pointer to intermediate wrt buf  */
					/* */
    unsigned long int DWC$l_dcab_cache_count; /* Count of records in cache  */
					      /* */
    struct DWC$db_vm_record *DWC$a_dcab_lru_flink; /* Link to first record  */
						   /* in cache */
    struct DWC$db_vm_record *DWC$a_dcab_lru_blink; /* Link to last record   */
						   /* in cache */
    struct DWC$db_vm_record *DWC$a_dcab_free_flink; /* Flink to next free   */
						    /* block */
    struct DWC$db_vm_record *DWC$a_dcab_free_blink; /* Blink to last free   */
						    /* block */

/*
**   Get_item context
*/
    unsigned long int DWC$l_dcab_current_day; /* Context for get-item. */
    struct DWC$db_vm_record *DWC$a_dcab_current_item_vm; /* Context for	    */
							 /* get-item. */
    struct DWC$db_vm_record *DWC$a_dcab_current_day_vm; /* Context for	    */
							/* get-item. */
    char *DWC$a_dcab_temp_alarm;        /* Temp alarm array */
    char *DWC$a_dcab_temp_text;         /* Temp text array */

/*
**    In memory alarm management fields
*/
    unsigned long int DWC$l_dcab_hi_org_day; /* High day of alarm cache */
    struct DWC$db_alarm *DWC$a_dcab_alarm_flink; /* Flink to next alarm	    */
						 /* block */
    struct DWC$db_alarm *DWC$a_dcab_alarm_blink; /* Blink to prev alarm	    */
						 /* block */
    unsigned long int DWC$l_dcab_alarm_day; /* Last alarm day inquired */
    unsigned short int DWC$w_dcab_alarm_min; /* Last alarm min inquired */
    unsigned short int DWC$w_dcab_fill; /* ** Fill ** */
    struct DWC$db_alarm *DWC$a_dcab_free_alarm_f; /* Flink to first free    */
						  /* alam block */
    struct DWC$db_alarm *DWC$a_dcab_free_alarm_b; /* Blink to prev */
    struct DWC$db_alarm *DWC$a_dcab_alarm_ptr; /* Last entry inquired */
    unsigned long int DWC$l_dcab_alarm_cnt; /* Count of loaded alarms */
    unsigned long int DWC$l_dcab_alarm_rctx; /* Last repeat context */

/*
**   In memory repeat expressions
*/
    struct DWC$db_repeat_control *DWC$a_dcab_repeat_ctl_flink; /* First	    */
							       /* repeat control block */
    struct DWC$db_repeat_control *DWC$a_dcab_repeat_ctl_blink; /* Last	    */
							       /* repeat control block */
    struct DWC$db_vm_record *DWC$a_dcab_repeat_vec_flink; /* First repeat   */
							  /* vector block */
    struct DWC$db_vm_record *DWC$a_dcab_repeat_vec_blink; /* Last repeat    */
							  /* vector block */
    unsigned long int DWC$l_dcab_repeat_ctx; /* VM Repeat expression	    */
					     /*	context */
    unsigned char DWC$b_dcab_workweek;  /* Days in week that are working */
					/* days (by default). TRUE = */
					/* workday. Bit 0 = Sunday. */

/*
**   Day type cache.
*/
    unsigned long int DWC$l_dcab_dtbase; /* Base for daytype vector */
    unsigned char DWC$b_dcab_dtcache [126];
					/* Array of cached daytypes */
					/*  for three submaps */

/* 
**   Get_next_repeat context
*/
    struct DWC$db_repeat_control *DWC$a_dcab_current_repeat; /* Context for */
							     /*	get-item. */

/*
**   The following structure fields must be declare IDENTICAL in the
**   HEADER record.
*/
    char *DWC$t_statistics;
    long int DWC$l_write;
    long int DWC$l_miss;
    long int DWC$l_hit;
    long int DWC$l_allocate;
    long int DWC$l_free;


    char *DWC$a_dcab_temp_icons;	/* Temp icons array */
};


/*
** Constants produced from the above structures
*/
#define DWC$K_MAX_ERRORS 4              /* Max error level */

/*  stat_length are Longwords */
#define DWC$k_statistics_length 6

#endif	/* _dwc_db_work_structures_ */
