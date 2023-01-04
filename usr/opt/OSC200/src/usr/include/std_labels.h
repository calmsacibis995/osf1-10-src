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
 *	@(#)$RCSfile: std_labels.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:18:57 $
 */ 
/*
 */
#if SEC_BASE && SEC_ENCODINGS
#ifndef __STD_LABELS_H__
#define __STD_LABELS_H__

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*

 * Based on:

 */

#include <sys/types.h>
#include <sys/security.h>
#include <mandatory.h>

/*
 * CMW Labels Release 1.0; 5/30/89: std_labels.h
 */

/*
 * std_labels.h contains definitions and variable declarations needed by l_init.c,
 * std_labels.c, and their callers.
 *
 * The first portion of this file defines how compartments, compartment masks, markings,
 * and marking masks are stored on the system (i.e. how big they are) and the needed
 * operations on them.  These definitions can be changed to accomodate other size
 * compartments and markings, and all using programs can be recompiled to automatically
 * handle other size labels.
 *
 * The remainder of the file contains external variable and structure definitions, as well
 * as other key definitions needed by the routines listed above and their callers.
 *
 * Since it is intended that this file be compiled with other programs that call the
 * subroutines in std_labels.c, all variables, structure names, and structure component
 * names start with "l_" to hopefully distinguish them from other names.  Finally, common
 * looking defines like END and NONE start with L_ to distinguish them also.
 */

/*
 * L_ALIGN aligns integers and pointers passed as arguments
 * to the boundary needed for COMPARTMENTS, MARKINGS,
 * COMPARTMENT_MASKs, and MARKING_MASKs on the machine for which this code is
 * to be compiled.
 */
 
#define L_ALIGN(p) p += ((long) p) % 2

/*
 * The following structure defines the header of a compiled label
 * encodings file.
 */

struct l_dbf_header {
	long	total_space;
	long	table_space;
};

/*
 * The following typedef is for a declarator for storing the internal form
 * of a classification.
 */

typedef long CLASSIFICATION;


/*
 * The following definitions are for declarators and operations on
 * compartment and marking bit masks.  The operations perform normal
 * bit manipulation and testing.  Only needed operations appear below,
 * so not all operations are defined for both compartments and markings.
 */

/* size in octets of a COMPARTMENT_MASK and COMPARTMENTS */
#define COMPARTMENTS_SIZE (CATWORDS * sizeof(mask_t))

/* size in octets of a MARKING_MASK and MARKINGS */
#define MARKINGS_SIZE (MARKWORDS * sizeof(mask_t))

typedef struct { mask_t l_words[1];}  COMPARTMENT_MASK;
typedef struct { mask_t l_words[1];} MARKING_MASK;

/*
 * COMPARTMENT_MASK modification operations.  The first argument is modified.
 */

/*
 * Zero COMPARTMENT_MASK c.
 */

#define COMPARTMENT_MASK_ZERO(c) { register mask_t *rc = (mask_t *)(c);\
                                   register int i = CATWORDS;\
                                   while (--i >= 0) *rc++ = 0; }

/*
 * Copy COMPARTMENT_MASK c2 to COMPARTMENT_MASK c1.
 */

#define COMPARTMENT_MASK_COPY(c1,c2) { register mask_t *rc1 = (mask_t*)(c1);\
                                       register mask_t *rc2 = (mask_t *)(c2);\
                                       register int i = CATWORDS;\
                                       while (--i >= 0) *rc1++ = *rc2++; }

/*
 * Combine the bits in COMPARTMENT_MASK c2 with those in COMPARTMENT_MASK c1.
 */

#define COMPARTMENT_MASK_COMBINE(c1,c2) { register mask_t *rc1=(mask_t *)(c1);\
                                          register mask_t *rc2=(mask_t *)(c2);\
                                          register int i = CATWORDS;\
                                          while (--i >= 0) *rc1++ |= *rc2++; }

/*
 * Set bit b (numbered from the left starting at 0) in COMPARTMENT_MASK c.
 */

#define COMPARTMENT_MASK_BIT_SET(c,b) ((mask_t *)(&(c)))[(b)/32] |=\
                                      ((mask_t) 1 << ((b)%32))

/*
 * COMPARTMENT_MASK testing operations. 
 */

/*
 * Return TRUE iff the  bits in COMPARTMENT_MASK c1 are all on in COMPARTMENT_MASK c2.
 */

#define COMPARTMENT_MASK_IN(c1,c2) mask_t_subset_p(c1, c2, CATWORDS)

/*
 * Return TRUE iff COMPARTMENT_MASKs c1 and c2 have any bits in common.
 */

#define COMPARTMENT_MASK_ANY_BITS_MATCH(c1,c2) mask_t_intersect_p(c1, c2, CATWORDS)

/*
 * Return TRUE iff the bits in COMPARTMENT_MASK c1 dominate those in
 * COMPARTMENT_MASK c2.
 */

#define COMPARTMENT_MASK_DOMINATE(c1,c2) mask_t_subset_p(c2, c1, CATWORDS)

/*
 * Return TRUE iff COMPARTMENT_MASKs c1 and c2 are equal.
 */
        
#define COMPARTMENT_MASK_EQUAL(c1,c2) mask_t_equal_p(c1,c2,CATWORDS)
                                 
/*
 * MARKING_MASK modification operations.  The first argument is modified.
 */

/*
 * Zero MARKING_MASK c.
 */

#define MARKING_MASK_ZERO(m) { register mask_t *rm = (mask_t *)(m);\
                               register int i = MARKWORDS;\
                               while (--i >= 0) *rm++ = 0; }

/*
 * Copy MARKING_MASK c2 to MARKING_MASK c1.
 */

#define MARKING_MASK_COPY(m1,m2) { register mask_t *rm1 = (mask_t *)(m1);\
                                   register mask_t *rm2 = (mask_t *)(m2);\
                                   register int i = MARKWORDS;\
                                   while (--i >= 0) *rm1++ = *rm2++; }

/*
 * Combine the bits in MARKING_MASK c2 with those in MARKING_MASK c1.
 */

#define MARKING_MASK_COMBINE(m1,m2) { register mask_t *rm1 = (mask_t *)(m1);\
                                      register mask_t *rm2 = (mask_t *)(m2);\
                                      register int i = MARKWORDS;\
                                      while (--i >= 0) *rm1++ |= *rm2++; }
                                    
/*
 * MARKING_MASK testing operations.
 */

/*
 * Return TRUE iff the  bits in MARKING_MASK c1 are all on in MARKING_MASK c2.
 */
        
#define MARKING_MASK_IN(m1,m2) mask_t_subset_p(m1, m2, MARKWORDS)
                              
/*
 * Set bit b (numbered from the left starting at 0) in MARKING_MASK c.
 */

#define MARKING_MASK_BIT_SET(m,b) ((mask_t *)(&(m)))[(b)/32] |=\
                                   ((mask_t) 1 << ((b)%32))

/*
 * Return TRUE iff the bits in MARKING_MASK m1 dominate those in
 * MARKING_MASK m2.
 */

#define MARKING_MASK_DOMINATE(m1,m2) mask_t_subset_p(m2, m1, MARKWORDS)
                                    
/*
 * Return TRUE iff MARKING_MASKs c1 and c2 are equal.
 */
        
#define MARKING_MASK_EQUAL(m1,m2) mask_t_equal_p(m1, m2, MARKWORDS)

/*
 * The following definitions are for declarators and operations on 
 * compartment and marking bits (as opposed to compartment or marking
 * MASKS.  The semantics of declaring something
 * as COMPARTMENTS or MARKINGS is that the resultant bits can contain
 * both regular and inverse compartments or markings, and the operations
 * defined below properly operate on these bit strings, taking into
 * account regular and inverse bits.
 *
 * Those operations that have the same meaning for COMPARTMENTS and 
 * COMPARTMENT_MASKS are defined below in terms of the COMPARTMENT_MASK
 * operations above.
 *
 * Those operations that have the same meaning for MARKINGS and 
 * MARKING_MASKS are defined below in terms of the MARKING_MASK
 * operations above.
 */
 
typedef COMPARTMENT_MASK COMPARTMENTS;
typedef MARKING_MASK MARKINGS;

/*
 * COMPARTMENTS modification operations.  The first argument is modified.
 */

/*
 * Zero COMPARTMENTS c.
 */

#define COMPARTMENTS_ZERO(c) COMPARTMENT_MASK_ZERO(c)

/*
 * Copy COMPARTMENTS c2 to COMPARTMENTS c1.
 */
 
#define COMPARTMENTS_COPY(c1,c2) COMPARTMENT_MASK_COPY(c1,c2)

/*
 * Combine the COMPARTMENTS in c2 with those in c1.
 */

#define COMPARTMENTS_COMBINE(c1,c2) COMPARTMENT_MASK_COMBINE(c1,c2)

/*
 * Bitwise AND the COMPARTMENTS in c2 with those in c1.
 */

#define COMPARTMENTS_AND(c1,c2) { register mask_t *rc1 = (mask_t *)(c1);\
                                  register mask_t *rc2 = (mask_t *)(c2);\
                                  register int i = CATWORDS;\
                                  while (--i >= 0) *rc1++ &= *rc2++; }

/*
 * Set bit b (numbered from the right starting at 0) in COMPARTMENTS c.
 */
 
#define COMPARTMENTS_BIT_SET(c,b) COMPARTMENT_MASK_BIT_SET(c,b)

/*
 * Logically set the COMPARTMENTS specified by COMPARTMENT_MASK mask and
 * COMPARTMENTS c2 in COMPARTMENTS c1.  Logically set means that those bits
 * sepcified by mask will be set to the SAME VALUE in c1 as they are in c2.
 */
 
#define COMPARTMENTS_SET(c1,mask,c2) { register mask_t *rc1 = (mask_t *)(c1);\
                                       register mask_t *rc2 = (mask_t *)(c2);\
                                       register mask_t *rm = (mask_t *)(mask);\
                                       register int i = CATWORDS;\
                                       while (--i >= 0) {\
                                           *rc1 &= ~*rm++;\
                                           *rc1++ |= *rc2++; }}

/*
 * COMPARTMENTS testing operations.
 */
 
/*
 * Return TRUE iff the compartments (inverse or normal) specified by the
 * COMPARTMENT_MASK mask and the COMPARTMENTS c2 are logically present
 * in COMPARTMENTS c1.  Logically present means that the bits specified
 * in mask must have the SAME VALUE in c1 as they do in c2.
 */

#define COMPARTMENTS_IN(c1,mask,c2) mask_t_masked_equal_p(c1,mask,c2,CATWORDS)

/*
 * Return TRUE iff the COMPARTMENTS in c1 dominate the COMPARTMENTS in c2.
 */

#define COMPARTMENTS_DOMINATE(c1,c2) COMPARTMENT_MASK_DOMINATE(c1,c2)

/*
 Return TRUE iff the COMPARTMENTS in c1 equal the COMPARTMENTS in c2.
 */

#define COMPARTMENTS_EQUAL(c1,c2) COMPARTMENT_MASK_EQUAL(c1,c2)

/*
 * MARKINGS modification operations.  The first argument is modified.
 */

/*
 * Zero MARKINGS m.
 */
 
#define MARKINGS_ZERO(m) MARKING_MASK_ZERO(m)

/*
 * Copy MARKINGS c2 to MARKINGS c1.
 */
 
#define MARKINGS_COPY(m1,m2) MARKING_MASK_COPY(m1,m2)

/*
 * Combine MARKINGS m2 with MARKINGS m1.
 */

#define MARKINGS_COMBINE(m1,m2) MARKING_MASK_COMBINE(m1,m2)
                                
/*
 * Set bit b (numbered from the left starting at 0) in MARKINGS m.
 */
 
#define MARKINGS_BIT_SET(m,b) MARKING_MASK_BIT_SET(m,b)

/*
 * Logically set the MARKINGS specified by MARKING_MASK mask and
 * MARKINGS m2 in MARKINGS m1.  Logically set means that those bits
 * sepcified by mask will be set to the SAME VALUE in m1 as they are in m2.
 */
 
#define MARKINGS_SET(m1,mask,m2) { register mask_t *rm1 = (mask_t *)(m1);\
                                   register mask_t *rm2 = (mask_t *)(m2);\
                                   register mask_t *rm = (mask_t *)(mask);\
                                   register int i = MARKWORDS;\
                                   while (--i >= 0) {\
                                        *rm1 &= ~*rm++;\
                                        *rm1++ |= *rm2++; }}

/*
 * MARKINGS testing operations.
 */
 
/*
 * Return TRUE iff the markings (inverse or normal) specified by the
 * MARKING_MASK mask and the MARKINGS m2 are logically present
 * in MARKINGS m1.  Logically present means that the bits specified
 * in mask must have the SAME VALUE in m1 as they do in m2.
 */

#define MARKINGS_IN(m1,mask,m2) mask_t_masked_equal_p(m1,mask,m2,MARKWORDS)

/*
 * Return TRUE iff the MARKINGS in m1 dominate the MARKINGS in m2.
 */

#define MARKINGS_DOMINATE(m1,m2) MARKING_MASK_DOMINATE(m1,m2)

/*
 Return TRUE iff the MARKINGS in c1 equal the MARKINGS in c2.
 */

#define MARKINGS_EQUAL(m1,m2) MARKING_MASK_EQUAL(m1,m2)

/*
 * A type that refers to any of the types: COMPARTMENT_MASK, COMPARTMENTS,
 * MARKING_MASK, or MARKINGS.  This union is used in declaring arguments
 * to parse_bits, which modifies any of the above types of bit strings.
 */

typedef union
{
    COMPARTMENT_MASK l_cm;
    COMPARTMENTS l_c;
    MARKING_MASK l_mm;
    MARKINGS l_m;
} COMPARTMENTS_OR_MARKINGS_OR_MASKS;

/*
 * Tables relating to classifications and definitions useful for processing the tables.
 * These tables apply to all types of labels.
 */
 
extern char **l_long_classification;    /* long name of each classification */
extern char **l_short_classification;   /* short name of each classification */
extern MARKINGS **l_in_markings;            /* initial markings for each class */
extern COMPARTMENTS **l_in_compartments;    /* initial compartments for each class */

#define NO_LABEL -1                     /* classification value meaning label no set yet */
#define NO_CLASSIFICATION (char **) 0   /* arg for l_convert sez don't output class */
 
/*
 * The following define is for a test to determine whether a particular classification
 * (cl) is visible given a sepcified maximum classification (mc). This define evaluates
 * to TRUE iff classification cl exists and is visible given classification mc.
 */

#define CLASSIFICATION_VISIBLE(cl,mc) (l_long_classification[cl] && (mc) >= (cl))

/*
 * The following declarations are for the accreditation range specification, which
 * applies only to sensitivity labels.  An accreditation range structure is present for
 * each valid classification.  Each accreditation range specification is of one of four
 * types, as defined by l_ar_type.  Each specification can state that 1) no compartment
 * combinations are valid with this classification (NONE_VALID), 2) that all compartment
 * combinations are valid for this classification except those listed by l_ar_start and
 * l_ar_end (ALL_VALID_EXCEPT), 3) that all compartment combinations are valid for this
 * classification, or that 4) the only valid compartment combinations for this 
 * classification are those listed by l_ar_start and l_ar_end.
 */

struct l_accreditation_range
{
    short l_ar_type;    /* type of range for this classification */
    char *l_ar_start;           /* ptr to start of COMPARTMENTS list */
    char *l_ar_end;             /* ptr beyond end of COMPARTMENTS list */
};

extern struct l_accreditation_range *l_accreditation_range;

/*
 * Acceptable values for l_ar_type.
 */

#define L_NONE_VALID 0
#define L_ALL_VALID_EXCEPT 1
#define L_ALL_VALID 2
#define L_ONLY_VALID 3

/*
 * Tables relating to words in labels other than classification, and definitions useful 
 * for processing the tables.
 */
 
struct l_word       /* the structure of each word table entry */
{
    char *l_w_output_name;              /* used for output only */
    char *l_w_long_name;            /* used for input only */
    char *l_w_short_name;           /* used for input only */
    CLASSIFICATION l_w_min_class;/* min classification needed for this entry */
    CLASSIFICATION l_w_output_min_class;/* min classification needed to output this entry */
    CLASSIFICATION l_w_max_class;/* max classification allowed for this entry */
    COMPARTMENT_MASK *l_w_cm_mask;/* compartment bit mask for this entry */
    COMPARTMENTS *l_w_cm_value; /* compartments value for this entry */
    MARKING_MASK *l_w_mk_mask;  /* marking bit mask for this entry */
    MARKINGS *l_w_mk_value;     /* markings value for this entry */
    short l_w_prefix;           /* l_words index of another word needed to prefix entry,
                                   or IS_PREFIX if this is a prefix entry itself */
    short l_w_suffix;           /* l_words index of another word needed to suffix entry,
                                   or IS_SUFFIX if this is a SUFFIX entry itself */

    short l_w_flags;            /* flags for each entry */
};

#define ALL_ENTRIES 0       /* l_w_flags value to match all l_word entries */
#define ACCESS_RELATED 1    /* l_w_flags value to match only access related entries */

struct l_word_pair  /* the structure of required and invalid combination table entries */
{
    short l_word1;
    short l_word2;
};

struct l_constraints    /* the structure of the combination constraints table */
{
    short l_c_type;     /* type of constraint: NOT_WITH or ONLY_WITH or L_END */
    short *l_c_first_list;  /* the start of the first list */
    short *l_c_second_list; /* ptr to second of two lists */
    short *l_c_end_second_list; /* ptr beyond end of second list */
};

#define NOT_WITH 0
#define ONLY_WITH 1

struct l_tables     /* the information about each type of label (other than classification */
{
    short l_num_entries;        /* total number of entries in l_words table */
    short l_first_main_entry;   /* first non-prefix/suffix entry */
    short l_max_length;         /* maximum length of label l_converted with this l_tables */
    struct l_word *l_words;     /* the l_word table itself */
    struct l_word_pair *l_required_combinations;/* table of required combos of two words */
    struct l_constraints *l_constraints;/* table of combination constraints */
};

/*
 * The l_tables for each type of label or printer banner output string.
 */

extern struct l_tables *l_information_label_tables; /* information label tables */
extern struct l_tables *l_sensitivity_label_tables; /* sensitivity label tables */
extern struct l_tables *l_clearance_tables;         /* clearance tables */
extern struct l_tables *l_channel_tables;           /* handle via channel tables */
extern struct l_tables *l_printer_banner_tables;    /* printer banner tables */

/*
 * The following define is for a test to determine whether a particular word
 * in the word table is visible given a specified maximum classification and
 * compartments. This define assumes that the variable l_tables is a pointer
 * to the appropriate label tables, and evaluates to TRUE iff index i of the
 * word table is visible given classification cl and compartments cm.
 */

#define WORD_VISIBLE(i,cl,cm) ((cl)>=l_tables->l_words[i].l_w_min_class \
      && COMPARTMENTS_DOMINATE (cm, l_tables->l_words[i].l_w_cm_value))

/*
 * The following three definitions are values for the l_w_prefix and
 * l_w_suffix entries of the word structure (respectively), indicating 
 * that the entry in the word table IS a prefix or suffix as opposed
 * to an entry that REQUIRES a prefix or suffix (if value >= 0) or neither
 * IS nor requires a prefix or suffix (value = -1).
 */
 
#define L_IS_PREFIX -2          /* indicates an entry is a prefix */
#define L_IS_SUFFIX -2          /* indicates an entry is a suffix */
#define L_NONE -1                   /* indicates an entry does not need a prefix or suffix */

/*
 * Miscellaneous l_tables-related definitions.
 */

#define L_END -1                    /* indicates end of required or
                                       invalid combination table */

/*
 * The following variables contain the values for various system parameters and
 * useful constant values and temporary compartment and marking variables.
 */

struct l_sensitivity_label
{
    CLASSIFICATION l_classification;
    COMPARTMENTS *l_compartments;
};

extern char *l_version; /* the version string for this version of the encodings */
extern CLASSIFICATION *l_min_classification;/* lowest classification encoded */
extern CLASSIFICATION *l_max_classification;/* highest classification encoded */
extern CLASSIFICATION *l_classification_protect_as; /* for printer banner */
extern COMPARTMENTS *l_hi_compartments; /* highest set of compartments encoded */
extern MARKINGS *l_hi_markings; /* highest set of markings encoded */
extern struct l_sensitivity_label *l_lo_clearance;  /* lowest clearance possible */
extern struct l_sensitivity_label *l_lo_sensitivity_label;  /* lowest SL possible */
extern COMPARTMENTS *l_0_compartments;  /* a source of zero compartment bits for comparisons */
extern MARKINGS *l_0_markings;  /* a source of zero marking bits for comparisons */
extern COMPARTMENTS *l_t_compartments;  /* a temporary compartment bit variable */
extern MARKINGS *l_t_markings;  /* a temporary marking bit variable */
extern COMPARTMENT_MASK *l_comps_handled;   /* compartments handled in output */
extern MARKING_MASK *l_marks_handled;       /* markings handled in output */

/*
 * The following definitions are useful when calling the convert subroutines.
 * NO_PARSE_TABLE, as an argument to l_convert, indicates that the caller does
 * not want l_convert to return a parse table.  SHORT_WORDS and LONG_WORDS, when
 * used as the last argument to l_convert or l_b_convert, indicate whether the
 * short or long names of words should be output.  LONG_WORDS uses the 
 * l_w_output_name, and SHORT_WORDS uses the l_w_short_name if present, else
 * the l_w_output_name.
 */
 
#define NO_PARSE_TABLE (char *) 0
#define SHORT_WORDS 1
#define LONG_WORDS 0
#define	SHORT_CLASS 1
#define	LONG_CLASS 0

/*
 * The following definition is the maximum length of a keyword and its value
 * in the label encodings file.  This is also the maximum length of sensitivity
 * labels and word combinations in the encodings file.
 */

#define MAX_ENCODINGS_LINE_LENGTH 256

/*
 * Useful definitions used in l_init.c and std_labels.c.
 */

#define L_MAX(a, b) (a > b ? a : b)
#define L_MIN(a, b) (a < b ? a : b)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif /* __STD_LABELS_H__ */
#endif /* SEC_BASE && SEC_ENCODINGS */
