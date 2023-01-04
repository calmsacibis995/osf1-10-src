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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: omandlib.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:33 $";
#endif 
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
 *  Copyright (c) 1989 SecureWare, Inc.
 *  All Rights Reserved
 */


/* This code probably seems very, very confusing and convoluted. And it is.
   However it does lead to very easy extension to fully support synonyms and
   more importantly support for DIA Information Labels is built in to 
   the standard Orange Book B1 systems. If you do not understand the above,
   apologies for the convolution below; if you do understand the above the
   following code should be very familar */

/* This code is ripped off from usr/lib/libsecurity/mandlib.c */

#include <sys/secdefines.h>

#if SEC_BASE
/*
 * The Encodings file and SMP+ versions of this file are included
 * as separate alternatives. This file is a cheat. For MAC_OB we
 * mimic the Encodings file environment as much as possible.
 */

#if SEC_MAC_OB

#define MARKWORDS CATWORDS


/*
 *  Mandatory access control subroutine library.
 *
 *  This library of subroutines allows users to map between external and
 *  internal representations for sensitivity and information labels.
 */

#include <sys/types.h>
#include <memory.h>
#include <stdio.h>
#include <search.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/security.h>
#include <sys/secpolicy.h>
#include <mandatory.h>

typedef mand_ir_t	ilb_ir_t;

#define SEC_ENCODINGS 1
#include "ostd_labels.h"

extern char *strdup();

/*
 * Exported global variables:
 */

/* Already exported */
/*
ilb_ir_t	*mand_syslo;	
ilb_ir_t	*mand_syshi;
*/

mand_ir_t	*mand_clrnce;		/* user's clearance */
mand_ir_t	*mand_minclrnce;	/* minimum system clearance */
mand_ir_t	*mand_minsl;		/* minimum useful sensitivity label */

#ifdef NOT_USED
char				*l_version;
CLASSIFICATION			*l_classification_protect_as;
COMPARTMENTS			*l_hi_compartments;
MARKINGS			*l_hi_markings;
struct l_tables			*l_information_label_tables;
struct l_tables			*l_channel_tables;
struct l_tables			*l_printer_banner_tables;
MARKINGS			*l_0_markings;
#endif /* NOT_USED */

CLASSIFICATION			*l_min_classification;
CLASSIFICATION			*l_max_classification;
struct l_sensitivity_label	*l_lo_clearance;
struct l_sensitivity_label	*l_lo_sensitivity_label;
char				**l_long_classification;
char				**l_short_classification;
COMPARTMENTS			**l_in_compartments;
MARKINGS			**l_in_markings;
struct l_accreditation_range	*l_accreditation_range;
struct l_tables			*l_sensitivity_label_tables;
struct l_tables			*l_clearance_tables;
COMPARTMENTS			*l_0_compartments;
MARKINGS			*l_t_markings;
COMPARTMENTS			*l_t_compartments;
COMPARTMENT_MASK		*l_comps_handled;
MARKING_MASK			*l_marks_handled;

/*
 * Check to see if the label encodings have been loaded, and load them if not.
 * Return 1 if successful, 0 if encodings cannot be loaded.
 */

int
l_encodings_initialized()
{
	init_encodings();
	return 1;
}
    
/*
 * The internal subroutine set_compartment is intended to be called by
 * parse_bits (above).  It sets up the compartment mask and value for the
 * passed bit for the passed bit value.
 */

/* 
#ifdef COMPARTMENT_MASK_BIT_SET
#undef COMPARTMENT_MASK_BIT_SET
#endif

#define COMPARTMENT_MASK_BIT_SET(c,b) ((mask_t *)(c))[(b)/32] |=\
                                      ((mask_t) 1 << ((b)%32))
*/

static void
set_compartment(mask_ptr, bit)
    COMPARTMENTS_OR_MARKINGS_OR_MASKS   *mask_ptr;
    int     bit;
{
    COMPARTMENT_MASK_BIT_SET(mask_ptr->l_cm, bit);
}

int
init_encodings()
{
	int i;
	int	num_class;
	int	num_comps;
	int	max_comps_len;
	char	*s;

	/* If already initialized return */
	if (l_min_classification)
		return 0;

	mand_init ();

	/* initialize the tables */
	mand_minsl = mand_alloc_ir();
	mand_clrnce = mand_alloc_ir();
	mand_minclrnce = mand_alloc_ir();

	if (
	    mand_minsl == (mand_ir_t *) 0 || 
	    mand_clrnce == (mand_ir_t *) 0 ||
	    mand_minclrnce == (mand_ir_t *) 0)
	{
		fprintf(stderr,
		"Cannot allocate space for minsl and clearance IRs\n");
		return -1;
	}

	/* Initialize the classification tables */
	l_short_classification = (char **) Malloc 
		( (int) (mand_max_class + 1) * sizeof (char *));
	if (! l_short_classification)
		MemoryError();

	l_long_classification = (char **) Malloc 
		( (int) (mand_max_class + 1)* (int) sizeof (char *));
	if (! l_long_classification)
		MemoryError();

	l_in_compartments = (COMPARTMENTS **) Malloc 
		( (int) (mand_max_class + 1) * sizeof (struct COMPARTMENTS *));
	if (! l_in_compartments)
		MemoryError();

	l_in_markings = (MARKINGS **) Malloc 
		( (int) (mand_max_class + 1) * sizeof (struct MARKINGS *));
	if (! l_in_markings)
		MemoryError();

	l_accreditation_range = (struct l_accreditation_range *)  Malloc 
		( (int) (mand_max_class + 1) * 
			sizeof (struct l_accreditation_range));
	if (! l_accreditation_range)
		MemoryError();


	/* Read the classification table in note that there may be
  	 * some holes in the mand_ob classification table */
	num_class = 0;
	for (i=0; i<=mand_max_class; i++) {
			/* Initially none valid */
		l_accreditation_range[i].l_ar_type = (short) 
				L_NONE_VALID;
#ifdef DEBUG
printf ("Classification %d\n", i);
#endif
		s = mand_cltoname(i);
		/* Only store it if there is a valid classification */
		if (s) {
#ifdef DEBUG
printf ("Classification %d\n", i);
printf ("Classification %s\n", s);
#endif
			l_long_classification[i]  = strdup (s);
			if (! l_long_classification[i])
				MemoryError();
#ifdef DEBUG
printf ("Classification %s\n", l_long_classification[i]);
#endif

			l_short_classification[i] = strdup (s);
			if (! l_short_classification[i])
				MemoryError();

			/* The value should be 0 so nothing to set */
			l_in_compartments[i] = (COMPARTMENTS *) Malloc 
				( (int) mand_max_class * 
					sizeof (struct COMPARTMENTS *));
			if (! l_in_compartments[i])
				MemoryError();
	
			/* The value should be 0 so nothing to set */
			l_in_markings[i] = (MARKINGS *) Malloc 
				( (int) mand_max_class * 
					sizeof (struct MARKINGS *));
			if (! l_in_markings[i])
				MemoryError();

			/* All SLs are valid on a B1 system */
			l_accreditation_range[i].l_ar_type = (short) 
				L_ALL_VALID;
			num_class ++;
		}
		else {
			/* Holes in the table are null strings in the */
			/* classification table */
			l_long_classification[i]  = (char *) 0;
		}
	}

	/* Initialize other stuff */
		/* l_t_markings is never used - but let's be safe */
	l_marks_handled = (MARKING_MASK *) 
		Malloc (sizeof (MARKINGS) * MARKWORDS);
	if (! l_marks_handled)
		MemoryError();

	l_comps_handled = (COMPARTMENT_MASK *) 
		Malloc (sizeof (COMPARTMENTS) * CATWORDS);
	if (! l_comps_handled)
		MemoryError();

	l_t_markings = (MARKINGS *) 
		Malloc (sizeof (MARKINGS) * MARKWORDS);
	if (! l_t_markings)
		MemoryError();

	l_t_compartments = (COMPARTMENTS *) 
		Malloc (sizeof (COMPARTMENTS) * CATWORDS);
	if (! l_t_compartments)
		MemoryError();

	l_0_compartments = (COMPARTMENTS *) 
		Malloc (sizeof (COMPARTMENTS) * CATWORDS);
	if (! l_0_compartments)
		MemoryError();

	l_lo_clearance = (struct l_sensitivity_label *) Malloc 
		(sizeof (struct l_sensitivity_label));
	if (! l_lo_clearance)
		MemoryError();
	l_lo_clearance->l_classification = (CLASSIFICATION) 0;

	l_lo_clearance->l_compartments = (COMPARTMENTS *) 
		Malloc (sizeof (COMPARTMENTS) * CATWORDS);
	if (! l_lo_clearance->l_compartments)
		MemoryError();

	l_lo_sensitivity_label = (struct l_sensitivity_label *) Malloc 
		(sizeof (struct l_sensitivity_label));
	if (! l_lo_sensitivity_label)
		MemoryError();
	l_lo_sensitivity_label->l_classification = (CLASSIFICATION) 0;

	l_lo_sensitivity_label->l_compartments = (COMPARTMENTS *) 
		Malloc (sizeof (COMPARTMENTS) * CATWORDS);
	if (! l_lo_sensitivity_label->l_compartments)
		MemoryError();

	l_min_classification = (CLASSIFICATION *) Malloc 
		(sizeof (CLASSIFICATION));
	if (! l_min_classification)
		MemoryError();
	*l_min_classification = 0;

	l_max_classification = (CLASSIFICATION *) Malloc 
		(sizeof (CLASSIFICATION));
	if (! l_max_classification)
		MemoryError();
	*l_max_classification = mand_max_class;

	l_lo_sensitivity_label->l_classification = (CLASSIFICATION) 0;
	/* Initialize all the tables */
	l_sensitivity_label_tables = (struct l_tables *) Malloc 
		( ((int)mand_max_cat + 1) * sizeof (struct l_tables)); 
	l_sensitivity_label_tables->l_words = (struct l_word *) Malloc
			(((int) mand_max_cat + 1) * sizeof (struct l_word));

	/* Clearances can use the same table on a mac_ob system */
	l_clearance_tables = l_sensitivity_label_tables;

	num_comps = 0;
	max_comps_len = 0;
	/* Read in the name of the compartments */
	for (i=0; i<=mand_max_cat; i++) {
		s = mand_cattoname(i);
		/* Only store valid compartments */
		if (s) {
		l_sensitivity_label_tables->l_words[num_comps].l_w_output_name =
			 strdup (s);
		l_sensitivity_label_tables->l_words[num_comps].l_w_long_name =
			strdup (s);
		/* Never set the short name */
		/*
		l_sensitivity_label_tables->l_words[num_comps].l_w_short_name =
			strdup (s);
		*/
		max_comps_len += strlen(s);
		/* printf ("Labels are %s %s %s\n",
		l_sensitivity_label_tables->l_words[num_comps].l_w_output_name,
		l_sensitivity_label_tables->l_words[num_comps].l_w_long_name,
		l_sensitivity_label_tables->l_words[num_comps].l_w_short_name);
		*/
		/* Set the min and max classifications */
		l_sensitivity_label_tables->l_words[num_comps].l_w_min_class = 
			(CLASSIFICATION) 0;
		l_sensitivity_label_tables->l_words[num_comps].l_w_output_min_class = 
			(CLASSIFICATION) -1;
		l_sensitivity_label_tables->l_words[num_comps].l_w_max_class = 
			(CLASSIFICATION) mand_max_class;

		/* Set up the compartment and markings mask */
		l_sensitivity_label_tables->l_words[num_comps].l_w_cm_mask = 
			(COMPARTMENT_MASK *) Malloc 
			(sizeof (COMPARTMENT_MASK) * CATWORDS);
		if (! l_sensitivity_label_tables->l_words[num_comps].l_w_cm_mask)
			MemoryError();
	set_compartment (
	l_sensitivity_label_tables->l_words[num_comps].l_w_cm_mask, i);

		l_sensitivity_label_tables->l_words[num_comps].l_w_cm_value = 
			(COMPARTMENTS *) Malloc 
			(sizeof (COMPARTMENTS) * CATWORDS);
		if (! l_sensitivity_label_tables->l_words[num_comps].l_w_cm_value)
			MemoryError();

#ifdef DEBUG
#define COMPARTMENT_MASK_PRINT(c) { register mask_t *rc = (mask_t *)(c);\
                                   register int iii = CATWORDS;\
				   printf ("COMP PRINT \n"); \
                                   while (--iii >= 0) {\
					printf ("xxxxxx\n"); \
					printf ("iii = %d\n", iii); \
					printf ("r = %x\n", rc); \
					printf ("X = %x\n", *rc++); } }
COMPARTMENT_MASK_PRINT (l_sensitivity_label_tables->l_words[num_comps].l_w_cm_value);
#endif /* DEBUG */

	set_compartment (
l_sensitivity_label_tables->l_words[num_comps].l_w_cm_value, i);

		/* Markings */
		/* Not really needed but ... */
		l_sensitivity_label_tables->l_words[num_comps].l_w_mk_mask = 
			(MARKING_MASK *) Malloc 
			(sizeof (MARKING_MASK) * MARKWORDS);
		if (! l_sensitivity_label_tables->l_words[num_comps].l_w_mk_mask)
			MemoryError();

		l_sensitivity_label_tables->l_words[num_comps].l_w_mk_value = 
			(MARKINGS *) Malloc 
			(sizeof (MARKINGS) * MARKWORDS);
		if (! l_sensitivity_label_tables->l_words[num_comps].l_w_mk_value)
			MemoryError();

		/* No prefix or suffix for Orange Book */
		l_sensitivity_label_tables->l_words[num_comps].l_w_prefix =
			(short) L_NONE;

		l_sensitivity_label_tables->l_words[num_comps].l_w_suffix =
			(short) L_NONE;

		l_sensitivity_label_tables->l_required_combinations = 
			(struct l_word_pair *) Malloc 
			(sizeof (struct l_word_pair));
		if (! l_sensitivity_label_tables->l_required_combinations)
			MemoryError();
		l_sensitivity_label_tables->l_required_combinations->l_word1 = 
			L_END; 

		l_sensitivity_label_tables->l_constraints = 
			(struct l_constraints *) Malloc 
			(sizeof (struct l_constraints));
		if (! l_sensitivity_label_tables->l_constraints)
			MemoryError();
		l_sensitivity_label_tables->l_constraints->l_c_type = 
			L_END; 

		num_comps ++;
		}
	}

	l_sensitivity_label_tables->l_num_entries = num_comps;
	l_sensitivity_label_tables->l_first_main_entry = 0;
	l_sensitivity_label_tables->l_max_length = max_comps_len; /* deduce at end */

	mand_minclrnce->class = l_lo_clearance->l_classification;
	COMPARTMENTS_COPY(mand_minclrnce->cat, l_lo_clearance->l_compartments);

/*
	mand_syslo->class = *l_min_classification;
	COMPARTMENTS_COPY(mand_syslo->cat,
				l_in_compartments[*l_min_classification]);
	MARKINGS_COPY(&mand_syslo->cat[CATWORDS],
				l_in_markings[*l_min_classification]);

	mand_syshi->class = *l_max_classification;
	COMPARTMENTS_COPY(mand_syshi->cat, l_hi_compartments);
	MARKINGS_COPY(&mand_syshi->cat[CATWORDS], l_hi_markings);
*/

	mand_minsl->class = l_lo_sensitivity_label->l_classification;
	COMPARTMENTS_COPY(mand_minsl->cat,
				l_lo_sensitivity_label->l_compartments);

/*
	if (getclrnce(mand_clrnce) == -1)
		memcpy(mand_clrnce, mand_syshi, mand_bytes());
*/

	return 0;
}

/*
 * Return TRUE if m1 and m2 are equal, else return FALSE.
 */
mask_t_equal_p(m1, m2, n)
	register mask_t	*m1, *m2;
	register int	n;
{
	while (--n >= 0)
		if (*m1++ != *m2++)
			return 0;
	return 1;
}

/*
 * Return TRUE if m1 is a subset of m2, else return FALSE.
 */
mask_t_subset_p(m1, m2, n)
	register mask_t	*m1, *m2;
	register int	n;
{
	while (--n >= 0) {
		if ((*m1 & *m2) != *m1)
			return 0;
		++m1; ++m2;
	}
	return 1;
}

/*
 * Return TRUE if m1 and m2 intersect, else return FALSE.
 */
mask_t_intersect_p(m1, m2, n)
	register mask_t	*m1, *m2;
	register int	n;
{
	while (--n >= 0)
		if (*m1++ & *m2++)
			return 1;
	return 0;
}

/*
 * Return TRUE if m1 and m2 are equal in all bits specified by
 * the mask m, else return FALSE.
 */
mask_t_masked_equal_p(m1, m, m2, n)
	register mask_t	*m1, *m, *m2;
	register int	n;
{
	while (--n >= 0)
		if ((*m1++ ^ *m2++) & *m++)
			return 0;
	return 1;
}

void
mac_ob_init()
{
	int	i;

	i = init_encodings ();
}

#endif /* SEC_MAC_OB */
#endif /* SEC_BASE */
