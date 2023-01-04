#ifndef _dwc_db_record_structures_h_
#define _dwc_db_record_structures_h_
/* $Header$ */
/* dwc_db_record_structures.h */
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
**
**	Per Hamnqvist, November 1987
**
**  ABSTRACT:
**
**	This module contains the physical layout definitioins of
**	the records in the DECwindows calendar database
**
**--
*/

/*
** Get the definitions for the data types whose sizes are platform independent.
*/
#if !defined(__STDC__)
#define __STDC__ 0
#include <X11/Xmd.h>
#undef __STDC__
#else
#include <X11/Xmd.h>
#endif

/*
** This defines the different records types in the database. Each record
** in the database has its associated code stored in the first byte of the
** record. This allows for one type of sanity check when reading and
** modifying records.
*/
#define DWC$k_db_header 1               /* header block */
#define DWC$k_db_bitmap 2               /* storage allocation bitmap */
#define DWC$k_db_profile 3              /* user profile */
#define DWC$k_db_rangemap 4             /* range to subrange map */
#define DWC$k_db_subrangemap 5          /* subrange to daymap */
#define DWC$k_db_day_data 6             /* day data structure */
#define DWC$k_db_day_extension 7        /* day data structure ext */
#define DWC$k_db_repeat_expr_vec 8      /* repeat expression vector */
#define DWC$k_db_repeat_expr_block 9    /* repeated schedulable item */
#define DWC$k_db_repeat_extension 10    /* extension block for repeat */
#define DWC$k_db_repeat_exception_vec 11 /* repeat exception vector */
#define DWC$k_db_uninitialized 254
#define DWC$k_db_released 255

/*
** The following constants define the known host systems that can create
** a DECwindows Calendar database.
*/
#define DWC$k_db_unknown 0		/* Unknown system */
#define DWC$k_db_vax_vms 1		/* VMS */
#define DWC$k_db_ultrix 2		/* OLD Ultrix */
#define DWC$k_db_mips_ultrix 3		/* New mips Ultrix */
#define DWC$k_db_vax_ultrix 4		/* New vax Ultrix */
#define DWC$k_db_sun 5			/* SparcStation */
#define DWC$k_db_alpha_vms 6		/* Alpha VMS */
#define DWC$k_db_mips_osf1 7		/* mips OSF/1 */
#define DWC$k_db_alpha_osf 8		/* Alpha OSF/1 */

/*
** This structure defines the header record. The header record is the
** first record in the database. The rest of the database can be reached from
** the header block (though pointers).
*/
#define DWC$m_dbhd_shared 1             /* If possible, try and open */
#if 0
struct DWC$db_header {
    unsigned char DWC$b_dbhd_blocktype; /* Blocktype (for debugging) */
    unsigned char DWC$b_dbhd_version;   /* Database version */
    unsigned char DWC$b_dbhd_creator;   /* Creator system */
    unsigned char DWC$b_dbhd_flags;     /* Database flags: */
    unsigned long int DWC$l_dbhd_current_range; /* Current rangemap pointer */
    unsigned long int DWC$l_dbhd_first_range; /* Pointer to first rangemap  */
    unsigned long int DWC$l_dbhd_fill1; /*  MBZ */
    unsigned long int DWC$l_dbhd_repeat_head; /* Pointer to repeat vector   */
					      /* start */
    unsigned long int DWC$l_dbhd_profile; /* Pointer to profile record */
    unsigned long int DWC$l_dbhd_deq;   /* Default extension quantity */
    unsigned long int DWC$l_dbhd_current_base; /* Baseday of the current    */
					       /* Rmap */
    unsigned long int DWC$l_dbhd_fill2;	/* MBZ */

/*
**   The following structure fields must be declared IDENTICAL in the
**   CAB definition.
*/
    char *DWC$t_statistics;             /* MBZ; starting point */
    long int DWC$l_write;
    long int DWC$l_miss;
    long int DWC$l_hit;
    long int DWC$l_allocate;
    long int DWC$l_free;

/*
**   The following is an ASCII identification
*/
    char DWC$t_dbhd_ident [50];         /* ASCII identification */
    char DWC$t_dbhd_creator [40];       /* ASCII database creator */
    char DWC$t_dbhd_align2 [106];
    } ;

/*
** This structure defines the layout of a database block allocation bitmap.
*/
struct DWC$db_bitmap {
    unsigned char DWC$b_dbbi_blocktype; /* Blocktype (for debugging) */
    char DWC$t_dbbi_bitmap [253];       /* The bitmap vector itself */
    unsigned short int DWC$w_dbbi_eob;  /* Last bit used in bitmap */
    } ;


/*
** This is the calendar profile record.
*/
struct DWC$db_profile {
    unsigned char DWC$b_dbpr_blocktype; /* Blocktype (for debugging) */
    unsigned char DWC$b_dbpr_used;      /* Number of bytes used */
/* Max length of user data */
    char DWC$t_dbpr_data [254];         /* Start of variable length */
					/* data */
    } ;


/*
** This structure defines the layout of the rangemap. The purpose of the
** rangemap is to point off to the subrangemaps, which in turn point at
** the day data structure.
**
** The header points off to both the first rangemap defined at all and the
** current rangemap. The current is defined to be the one who maps the time
** around "now" while the other rangemaps are "future" and "past". The first
** rangemap could be the same as the current one. It could also be one that is
** very far back in time.
*/
struct DWC$db_rangemap {
    unsigned char DWC$b_dbra_blocktype; /* Blocktype (for debugging) */
    unsigned char DWC$b_dbra_flags;     /* NOT IN USE. */
    char DWC$t_dbra_align1 [2];         /* fill to longword */
    unsigned long int DWC$l_dbra_baseday; /* (truncated) Baseday */
    unsigned long int DWC$l_dbra_flink; /* Forward link to next rangemap */
    unsigned long int DWC$l_dbra_mbz;   /* Not used (Blink) */
    unsigned long int DWC$l_dbra_subvec [60]; /* Vector of pointers to submaps */
    } ;
#endif

/*
** This structure defines the layout of the subrangemap. The subrangemap points
** off to day data structures, which each handles the records associated with a
** particular day.
*/
#define DWC$m_dbsu_userflags 127        /* Mask of bits that user */
					/*  can play with */
#define DWC$m_dbsu_insignif 128         /* There are only insignificant */
					/*  entries on day */
#if 0
struct DWC$db_subrangemap {
    unsigned char DWC$b_dbsu_blocktype; /* Blocktype (for debugging) */
    unsigned char DWC$b_dbsu_flags;     /* NOT IN USE. */
    unsigned short int DWC$w_dbsu_baseday; /* (truncated) Baseday */
    unsigned long int DWC$l_dbsu_dayvec [42]; /* Vector of pointers to */
					/* day data structures */
    unsigned char DWC$b_dbsu_daymarks [42]; /* Daymark vector */
    unsigned char DWC$b_dbsu_dayflags [42]; /* Dayflags vector */
    } ;
#endif

/*
** The day data structure describes what is happening on a particular day.
** Because there can be an arbitrary number of things happening during a day,
** its length is variable. Furthermore, it contains a variable number of
** variable length records.
*/
#define DWC$m_dbda_iday_item 1          /*   Day starts with item */
					/*    initiated on an earlier */
#define DWC$m_dbda_r9done 2             /*   Rev #9 upgrade done on day. */
					/*    meaningfull only if */
					/*    header indicates #8. */

#if 0
struct DWC$db_day {
    unsigned char DWC$b_dbda_blocktype; /* Blocktype (for debugging) */
    unsigned char DWC$b_dbda_flags;     /* Day flags : */
    unsigned short int DWC$w_dbda_baseday; /* (truncated) Baseday */
    unsigned long int DWC$l_dbda_prev_day; /* if first item starts in */
					/* another day, this is the day */
    short int DWC$w_dbda_prev_idx;      /* Identification of item */
    short int DWC$w_dbda_item_idx;      /* item index (for db sanity) */
    unsigned long int DWC$l_dbda_flink; /* Pointer to first extension */
					/*  block */
    unsigned short int DWC$w_dbda_ext_count; /* Number of extension	    */
					     /*	records. */
    unsigned char DWC$b_dbda_fill1;     /* Mbz */
    char DWC$b_dbda_ext_sanity;         /* Sanity byte for extensions */
    char DWC$t_dbda_data [236];         /* Start of variable length */
					/* data */
    } ;



/*
** The item block is the extension of the day data block. It contains the
** variable length data that did not fit into the "data" part of the
** day data structure. This structure is also used when the repeat block
** needs an extension (however, the blocktype in the record is different).
*/
struct DWC$db_extension {
    unsigned char DWC$b_dbex_blocktype; /* Blocktype (for debugging) */
    char DWC$b_dbex_sanity;             /* Sanity check field */
    unsigned short int DWC$w_dbex_ext_count; /* Number of extension	    */
					     /*	records. */
    char DWC$t_dbex_data [1];           /* Start of variable length data */
    char DWC$t_dbex_align1 [251];
    } ;


/*
** This is a generic entry, aka item
*/
struct DWC$db_item {
    unsigned char DWC$b_dbit_blocktype; /* Record type */
    unsigned char DWC$b_dbit_specific;  /* Entry specific */
    short int DWC$w_dbit_id;            /* Item id (within day) */
    unsigned short int DWC$w_dbit_size; /* Length of item */
    char DWC$t_dbit_data [1];           /* Start of variable data */
    } ;
#endif
#define DWC$k_dbit_not_used 0           /* not used */
#define DWC$k_dbit_entry 1              /* entry record */
#define DWC$k_dbit_unused 2             /* Currently not used */
#define DWC$k_dbit_extension 3          /* next record is in extension. */

/*
** This is a entry record.
*/
#define DWC$m_dben_alarm_present 1      /*   Alarm before */
#define DWC$m_dben_repeat 2             /*   This is a repeat event */
#define DWC$m_dben_insignif 4           /*   Insignificant entry */

#if 0
struct DWC$db_entry {
    unsigned char DWC$b_dben_blocktype; /* Record type */
    unsigned char DWC$b_dben_flags;     /* Flags in an entry (please note */
					/*  that they are the same in */
					/*  repeat block, note and item) */
    short int DWC$w_dben_entry_id;      /* Item id (within day) */
    unsigned short int DWC$w_dben_size; /* Length of text + fixed part */
    unsigned short int DWC$w_dben_start_minute; /* Starting minute */
    unsigned long int DWC$l_dben_delta_days; /* Length in days */
    unsigned short int DWC$w_dben_delta_minutes; /* Length in minutes */
    unsigned short int DWC$w_dben_text_class; /* Text class */
    unsigned char DWC$b_dben_fill_1;    /* Make alarm vec word aligned */
    char DWC$t_dben_data [1];           /* Start of variable text */
    } ;
#endif

/* 
** This structure defines the layout of a repeat expression. Repeat expressions
** are stored in a vector, consisting of multiple vector blocks.  The repeat
** expression determines under what conditions the schedulable item (or other
** entity) is to be repeated.
**
** All repeats have a starting time. The starting time is always the time
** for the first instantiation of the expression. The ending time can be defined
** or open ennded.
**
** A note on repeat interval:
**    It is a bit vector interpretted as follows:
**	bit-nos	    meaning
**	0-4	    day cycle
**	5-8	    month cycle
**	9-12	    base month
**	13-31	    additional bits...???
*/
#define DWC$m_dbre_used 1               /*   This slot is used in vector */
#define DWC$m_dbre_open_ended 2         /*   Infinite repeat expression */
#define DWC$m_dbre_insignif 4           /*   Insignificant repeat */

#if 0
struct DWC$db_repeat_expr {
    unsigned long int DWC$l_dbre_baseday; /* Starting day of repeat. */
    unsigned long int DWC$l_dbre_endday; /* Ending day for repeat */
    unsigned short int DWC$w_dbre_basemin; /* Starting minute for repeat */
    unsigned short int DWC$w_dbre_endmin; /* Ending minute for repeat */
    unsigned long int DWC$l_dbre_repeat_interval; /* Repeat interval (see   */
						  /* flags) */
    unsigned long int DWC$l_dbre_repeat_interval2; /* Additional repeat	    */
						   /* information */
    unsigned long int DWC$l_dbre_exceptions; /* Pointer to exception vector */
    unsigned short int DWC$w_dbre_duration; /* Duration of entry (minutes)  */
    unsigned char DWC$b_dbre_flags;     /* Flags for a repeat expr: */
    unsigned char DWC$b_dbre_reptype;   /* Repeat expression type: */
    } ;



/*
** This structure defines one db record that makes up one segment of the
** repeat vector.
*/
struct DWC$db_repeat_vector {
    unsigned char DWC$b_dbrv_blocktype; /* Blocktype (for debugging) */
    char DWC$t_dbrv_padd [25];          /* Padding before first expr */
    unsigned short int DWC$w_dbrv_size; /* Records in next segment */
    char DWC$t_dbrv_data [224];         /* Vector of repeat expressions */
    unsigned long int DWC$l_dbrv_flink; /* Link to next vector segment */
    } ;
#endif

/* 
** This structure defines the "container" record for the repeated expression.
** It is a crippled daymap. The extension mechanism is very similar to that of
** a daymap. The actual repeated item is a normal item, as in a daymap (using
** the same datastructure).
*/
#define DWC$m_dbrb_alarm 1              /*   Alarm(s) present */
#define DWC$m_dbrb_repeat 2             /*   This is a repeat event */
#define DWC$m_dbrb_r8done 4             /*   Rev #8 upgrade done. */
					/*   Meaningfull only if header */
					/*   indicates rev #7. */
#if 0
struct DWC$db_repeat_block {
    unsigned char DWC$b_dbrb_blocktype; /* Blocktype (for debugging) */
    char DWC$b_dbrb_ext_sanity;         /* Sanity for extensions */
    unsigned short int DWC$w_dbrb_ext_count; /* Number of extension records */
    unsigned long int DWC$l_dbrb_flink; /* Link to first extension */
    unsigned short int DWC$w_dbrb_size; /* Length of variable data part */
    unsigned short int DWC$w_dbrb_text_class; /* Text class */
    unsigned char DWC$b_dbrb_flags;     /* Repeat flags (must be the same */
					/*  as those in dben). */
    char DWC$t_dbrb_data [243];         /* Start of repeated data */
    } ;



/*
** This structure defines a repeat exception vector. This vector contains
** days in which the associated repeat expression is not to be instansiated.
** A pointer to the first vector block is in the repeat expression itself.
** There can be multiple exception vectors. They are linked in a flink queue,
** starting with the first vector.
*/
struct DWC$db_repeat_exceptions {
    unsigned char DWC$b_dbrx_blocktype; /* Blocktype (for debugging) */
    unsigned char DWC$b_dbrx_next_free; /* Index of next free entry */
    char DWC$t_dbrx_align [2];          /* fill to longword */
    unsigned long int DWC$l_dbrx_exvec [62]; /* Vector of exceptions */
    unsigned long int DWC$l_dbrx_flink; /* Link to next vector block */
    } ;
#endif

/*
** Define additional constants, after information has been gathered during
** the build of the structures above.
*/
/* integer database version number */
#define DWC$k_db_version 11

/* Length (in bytes) of one record */
#define DWC$k_db_file_record_length 256

/* define the space for the expanded record in memory. */
#if 0
#if (DWC_LONG_BIT == 64)
#define DWC$k_db_record_length 512
#else
#define DWC$k_db_record_length 256
#endif
#else
#define DWC$k_db_record_length 256
#endif

/* Max known blocktype for database record */
#define DWC$k_db_max_blocktype 11

/* Max number of records one bitmap can handle */
#define DWC$k_db_bitmap_entries 2024

/* Count of submaps one rangemap can point at */
#define DWC$k_db_rangemap_entries 60

/* Count of daymaps that one submap can point at */
#define DWC$k_db_submap_entries 42

/* Number of days that are mapped by one rangemap */
#define DWC$k_db_days_per_rangemap 2520

/* Max user profile data */
#define DWC$k_db_max_profile_data 254

/* Max variable length data that fits into the first day data block */
#define DWC$k_db_max_day_data 236

/* Default file extension quantity */
#define DWC$k_db_deq 30

/* Default day extension quantity */
#define DWC$k_db_day_deq 2

/* Number of repeat expressions that fits into one vector block */
#define DWC$k_db_repeats_per_vector 8

/* Number of bytes of repeat data that fits in the first repeat block */
#define DWC$k_db_repeat_data_len 243

/* Max number of exceptions per vector */
#define DWC$k_db_max_exceptions 62

/* Biggest unsigned 16bit integer */
#define DWC$k_db_max_16bit_unsigned 65535

/* Largest known host system */
#define DWC$k_db_max_host_sys 2

/* Length of one repeat without the trailing padding of vcc. */
#define DWC$k_db_repeat_expr_len 28


/***
****
**** FROM HERE ON DOWN, WE HAVE COPIES OF THE STRUCTURES WHICH ARE
**** INDEPENDENT OF PLATFORM SPECIFIC SIZE DEFINITIONS.
****
***/

/*
** This structure defines the header record. The header record is the
** first record in the database. The rest of the database can be reached from
** the header block (though pointers).
*/
struct DWCDB_header
{
    CARD8	DWC$b_dbhd_blocktype; /* Blocktype (for debugging) */
    CARD8	DWC$b_dbhd_version;   /* Database version */
    CARD8	DWC$b_dbhd_creator;   /* Creator system */
    CARD8	DWC$b_dbhd_flags;     /* Database flags: */
    CARD32	DWC$l_dbhd_current_range; /* Current rangemap pointer */
    CARD32	DWC$l_dbhd_first_range; /* Pointer to first rangemap  */
    CARD32	DWC$l_dbhd_fill1; /*  MBZ */
    CARD32	DWC$l_dbhd_repeat_head; /* Pointer to repeat vector   */
					      /* start */
    CARD32	DWC$l_dbhd_profile; /* Pointer to profile record */
    CARD32	DWC$l_dbhd_deq;   /* Default extension quantity */
    CARD32	DWC$l_dbhd_current_base; /* Baseday of the current    */
					       /* Rmap */
    CARD32	DWC$l_dbhd_fill2;	/* MBZ */

/***
****
**** In the original, this is a (char *) in the file, this is going to be
**** a 32 bit field.  Since it is copied to the location in the memory
**** structure when the size is different, there isn't any big deal.
****
***/

/*
**   The following structure fields must be declared IDENTICAL in the
**   CAB definition.
*/
    CARD32	DWC$t_statistics;             /* MBZ; starting point */

    INT32	DWC$l_write;
    INT32	DWC$l_miss;
    INT32	DWC$l_hit;
    INT32	DWC$l_allocate;
    INT32	DWC$l_free;

/*
**   The following is an ASCII identification
*/
    INT8	DWC$t_dbhd_ident [50];         /* ASCII identification */
    INT8	DWC$t_dbhd_creator [40];       /* ASCII database creator */
    INT8	DWC$t_dbhd_align2 [106];
};


/*
** This structure defines the layout of a database block allocation bitmap.
*/
struct DWCDB_bitmap
{
    CARD8	DWC$b_dbbi_blocktype; /* Blocktype (for debugging) */
    INT8	DWC$t_dbbi_bitmap [253];       /* The bitmap vector itself */
    CARD16	DWC$w_dbbi_eob;  /* Last bit used in bitmap */
};



/*
** This is the calendar profile record.
*/
struct DWCDB_profile
{
    CARD8	DWC$b_dbpr_blocktype; /* Blocktype (for debugging) */
    CARD8	DWC$b_dbpr_used;      /* Number of bytes used */
/* Max length of user data */
    INT8	DWC$t_dbpr_data [254];         /* Start of variable length data */
};


/*
** This structure defines the layout of the rangemap. The purpose of the
** rangemap is to point off to the subrangemaps, which in turn point at
** the day data structure.
**
** The header points off to both the first rangemap defined at all and the
** current rangemap. The current is defined to be the one who maps the time
** around "now" while the other rangemaps are "future" and "past". The first
** rangemap could be the same as the current one. It could also be one that is
** very far back in time.
*/
struct DWCDB_rangemap
{
    CARD8	DWC$b_dbra_blocktype; /* Blocktype (for debugging) */
    CARD8	DWC$b_dbra_flags;     /* NOT IN USE. */
    INT8	DWC$t_dbra_align1 [2];         /* fill to longword */
    CARD32	DWC$l_dbra_baseday; /* (truncated) Baseday */
    CARD32	DWC$l_dbra_flink; /* Forward link to next rangemap */
    CARD32	DWC$l_dbra_mbz;   /* Not used (Blink) */
    CARD32	DWC$l_dbra_subvec [60]; /* Vector of pointers to submaps */
};

/*
** This structure defines the layout of the subrangemap. The subrangemap points
** off to day data structures, which each handles the records associated with a
** particular day.
*/
struct DWCDB_subrangemap
{
    CARD8	DWC$b_dbsu_blocktype; /* Blocktype (for debugging) */
    CARD8	DWC$b_dbsu_flags;     /* NOT IN USE. */
    CARD16	DWC$w_dbsu_baseday; /* (truncated) Baseday */
    CARD32	DWC$l_dbsu_dayvec [42]; /* Vector of pointers to */
					/* day data structures */
    CARD8	DWC$b_dbsu_daymarks [42]; /* Daymark vector */
    CARD8	DWC$b_dbsu_dayflags [42]; /* Dayflags vector */
};

/*
** The day data structure describes what is happening on a particular day.
** Because there can be an arbitrary number of things happening during a day,
** its length is variable. Furthermore, it contains a variable number of
** variable length records.
*/
struct DWCDB_day
{
    CARD8	DWC$b_dbda_blocktype; /* Blocktype (for debugging) */
    CARD8	DWC$b_dbda_flags;     /* Day flags : */
    CARD16	DWC$w_dbda_baseday; /* (truncated) Baseday */
    CARD32	DWC$l_dbda_prev_day; /* if first item starts in */
					/* another day, this is the day */
    INT16	DWC$w_dbda_prev_idx;      /* Identification of item */
    INT16	DWC$w_dbda_item_idx;      /* item index (for db sanity) */
    CARD32	DWC$l_dbda_flink; /* Pointer to first extension */
					/*  block */
    CARD16	DWC$w_dbda_ext_count; /* Number of extension	    */
					     /*	records. */
    CARD8	DWC$b_dbda_fill1;     /* Mbz */
    INT8	DWC$b_dbda_ext_sanity;         /* Sanity byte for extensions */
    INT8	DWC$t_dbda_data [236];         /* Start of variable length */
					/* data */
};



/*
** The item block is the extension of the day data block. It contains the
** variable length data that did not fit into the "data" part of the
** day data structure. This structure is also used when the repeat block
** needs an extension (however, the blocktype in the record is different).
*/
struct DWCDB_extension
{
    CARD8	DWC$b_dbex_blocktype; /* Blocktype (for debugging) */
    INT8	DWC$b_dbex_sanity;             /* Sanity check field */
    CARD16	DWC$w_dbex_ext_count; /* Number of extension	    */
					     /*	records. */
    INT8	DWC$t_dbex_data [1];           /* Start of variable length data */
    INT8	DWC$t_dbex_align1 [251];
};


/*
** This is a generic entry, aka item
*/
struct DWCDB_item
{
    CARD8	DWC$b_dbit_blocktype; /* Record type */
    CARD8	DWC$b_dbit_specific;  /* Entry specific */
    INT16	DWC$w_dbit_id;            /* Item id (within day) */
    CARD16	DWC$w_dbit_size; /* Length of item */
    INT8	DWC$t_dbit_data [1];           /* Start of variable data */
} ;
/*
** This is a entry record.
*/
struct DWCDB_entry
{
    CARD8	DWC$b_dben_blocktype; /* Record type */
    CARD8	DWC$b_dben_flags;     /* Flags in an entry (please note */
					/*  that they are the same in */
					/*  repeat block, note and item) */
    INT16	DWC$w_dben_entry_id;      /* Item id (within day) */
    CARD16	DWC$w_dben_size; /* Length of text + fixed part */
    CARD16	DWC$w_dben_start_minute; /* Starting minute */
    CARD32	DWC$l_dben_delta_days; /* Length in days */
    CARD16	DWC$w_dben_delta_minutes; /* Length in minutes */
    CARD16	DWC$w_dben_text_class; /* Text class */
    CARD8	DWC$b_dben_fill_1;    /* Make alarm vec word aligned */
    INT8	DWC$t_dben_data [1];           /* Start of variable text */
};


/* 
** This structure defines the layout of a repeat expression. Repeat expressions
** are stored in a vector, consisting of multiple vector blocks.  The repeat
** expression determines under what conditions the schedulable item (or other
** entity) is to be repeated.
**
** All repeats have a starting time. The starting time is always the time
** for the first instantiation of the expression. The ending time can be defined
** or open ennded.
**
** A note on repeat interval:
**    It is a bit vector interpretted as follows:
**	bit-nos	    meaning
**	0-4	    day cycle
**	5-8	    month cycle
**	9-12	    base month
**	13-31	    additional bits...???
*/
struct DWCDB_repeat_expr
{
    CARD32	DWC$l_dbre_baseday; /* Starting day of repeat. */
    CARD32	DWC$l_dbre_endday; /* Ending day for repeat */
    CARD16	DWC$w_dbre_basemin; /* Starting minute for repeat */
    CARD16	DWC$w_dbre_endmin; /* Ending minute for repeat */
    CARD32	DWC$l_dbre_repeat_interval; /* Repeat interval (see   */
						  /* flags) */
    CARD32	DWC$l_dbre_repeat_interval2; /* Additional repeat	    */
						   /* information */
    CARD32	DWC$l_dbre_exceptions; /* Pointer to exception vector */
    CARD16	DWC$w_dbre_duration; /* Duration of entry (minutes)  */
    CARD8	DWC$b_dbre_flags;     /* Flags for a repeat expr: */
    CARD8	DWC$b_dbre_reptype;   /* Repeat expression type: */
};



/*
** This structure defines one db record that makes up one segment of the
** repeat vector.
*/
struct DWCDB_repeat_vector
{
    CARD8	DWC$b_dbrv_blocktype; /* Blocktype (for debugging) */
    INT8	DWC$t_dbrv_padd [25];          /* Padding before first expr */
    CARD16	DWC$w_dbrv_size; /* Records in next segment */
    INT8	DWC$t_dbrv_data [224];         /* Vector of repeat expressions */
    CARD32	DWC$l_dbrv_flink; /* Link to next vector segment */
};


/* 
** This structure defines the "container" record for the repeated expression.
** It is a crippled daymap. The extension mechanism is very similar to that of
** a daymap. The actual repeated item is a normal item, as in a daymap (using
** the same datastructure).
*/
struct DWCDB_repeat_block
{
    CARD8	DWC$b_dbrb_blocktype; /* Blocktype (for debugging) */
    INT8	DWC$b_dbrb_ext_sanity;         /* Sanity for extensions */
    CARD16	DWC$w_dbrb_ext_count; /* Number of extension records */
    CARD32	DWC$l_dbrb_flink; /* Link to first extension */
    CARD16	DWC$w_dbrb_size; /* Length of variable data part */
    CARD16	DWC$w_dbrb_text_class; /* Text class */
    CARD8	DWC$b_dbrb_flags;     /* Repeat flags (must be the same */
					/*  as those in dben). */
    INT8	DWC$t_dbrb_data [243];         /* Start of repeated data */
};



/*
** This structure defines a repeat exception vector. This vector contains
** days in which the associated repeat expression is not to be instansiated.
** A pointer to the first vector block is in the repeat expression itself.
** There can be multiple exception vectors. They are linked in a flink queue,
** starting with the first vector.
*/
struct DWCDB_repeat_exceptions
{
    CARD8	DWC$b_dbrx_blocktype; /* Blocktype (for debugging) */
    CARD8	DWC$b_dbrx_next_free; /* Index of next free entry */
    INT8	DWC$t_dbrx_align [2];          /* fill to longword */
    CARD32	DWC$l_dbrx_exvec [62]; /* Vector of exceptions */
    CARD32	DWC$l_dbrx_flink; /* Link to next vector block */
};


#endif /* end of _dwc_db_record_structures_h_ */
