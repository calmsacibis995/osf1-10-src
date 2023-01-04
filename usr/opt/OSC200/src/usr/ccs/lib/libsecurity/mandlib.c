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
static char	*sccsid = "@(#)$RCSfile: mandlib.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:17:27 $";
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


/*

 */

#include <sys/secdefines.h>
#include "libsecurity.h"

#if SEC_MAC
/*
 * The Encodings file and SMP+ versions of this file are included
 * as separate alternatives.
 */

#if SEC_ENCODINGS /*{*/

/*
 *  Mandatory access control subroutine library.
 *
 *  This library of subroutines allows users to map between external and
 *  internal representations for sensitivity and information labels.
 */

#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
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

#define LABEL_ENCODINGS "/etc/policy/macilb/Encodings.db"

extern char *malloc(), *realloc();

/*
 * Exported global variables:
 */

static char	*mand_er = NULL;/* label conversion buffer */
static int	mand_ersz = 0;	/* size of label conversion buffer */

ilb_ir_t	*mand_syslo;	/* system low */
ilb_ir_t	*mand_syshi;	/* system high */

mand_ir_t	*mand_clrnce;		/* user's clearance */
mand_ir_t	*mand_minclrnce;	/* minimum system clearance */
mand_ir_t	*mand_minsl;		/* minimum useful sensitivity label */

char				*l_version;
CLASSIFICATION			*l_min_classification;
CLASSIFICATION			*l_max_classification;
CLASSIFICATION			*l_classification_protect_as;
COMPARTMENTS			*l_hi_compartments;
MARKINGS			*l_hi_markings;
struct l_sensitivity_label	*l_lo_clearance;
struct l_sensitivity_label	*l_lo_sensitivity_label;
char				**l_long_classification;
char				**l_short_classification;
COMPARTMENTS			**l_in_compartments;
MARKINGS			**l_in_markings;
struct l_accreditation_range	*l_accreditation_range;
struct l_tables			*l_information_label_tables;
struct l_tables			*l_sensitivity_label_tables;
struct l_tables			*l_clearance_tables;
struct l_tables			*l_channel_tables;
struct l_tables			*l_printer_banner_tables;
COMPARTMENTS			*l_0_compartments;
MARKINGS			*l_0_markings;
COMPARTMENTS			*l_t_compartments;
MARKINGS			*l_t_markings;
COMPARTMENT_MASK		*l_comps_handled;
MARKING_MASK			*l_marks_handled;

init_encodings()
{
	/* Load the label encodings if not already loaded */
	if (!l_encodings_initialized()) {
		return -1;
	}

	mand_syslo = ilb_alloc_ir();
	mand_syshi = ilb_alloc_ir();
	mand_minsl = mand_alloc_ir();
	mand_clrnce = mand_alloc_ir();
	mand_minclrnce = mand_alloc_ir();
	if (mand_syslo == (ilb_ir_t *) 0 || mand_syshi == (ilb_ir_t *) 0 ||
	    mand_minsl == (mand_ir_t *) 0 || mand_clrnce == (mand_ir_t *) 0 ||
	    mand_minclrnce == (mand_ir_t *) 0)
	{
		fprintf(stderr,
			MSGSTR(MANDLIB_1, "Cannot allocate space for syslo and syshi IRs\n"));
		return -1;
	}

	mand_minclrnce->class = l_lo_clearance->l_classification;
	COMPARTMENTS_COPY(mand_minclrnce->cat, l_lo_clearance->l_compartments);

	mand_syslo->class = *l_min_classification;
	COMPARTMENTS_COPY(mand_syslo->cat,
				l_in_compartments[*l_min_classification]);
	MARKINGS_COPY(&mand_syslo->cat[CATWORDS],
				l_in_markings[*l_min_classification]);

	mand_syshi->class = *l_max_classification;
	COMPARTMENTS_COPY(mand_syshi->cat, l_hi_compartments);
	MARKINGS_COPY(&mand_syshi->cat[CATWORDS], l_hi_markings);

	mand_minsl->class = l_lo_sensitivity_label->l_classification;
	COMPARTMENTS_COPY(mand_minsl->cat,
				l_lo_sensitivity_label->l_compartments);

	if (getclrnce(mand_clrnce) == -1)
		memcpy(mand_clrnce, mand_syshi, mand_bytes());

	return 0;
}

void
mand_end ()
{
}

mand_ir_t *
mand_alloc_ir ()
{
	if (mand_init() != 0)
		return ((mand_ir_t *) 0);

	return (mand_ir_t *) calloc (mand_bytes(), 1);
}

void
mand_free_ir (mand)
	mand_ir_t	*mand;
{
	free ((char *) mand);
}

ilb_ir_t *
ilb_alloc_ir ()
{
	if (mand_init() != 0)
		return (ilb_ir_t *) 0;

	return (ilb_ir_t *) calloc (ilb_bytes(), 1);
}

void
ilb_free_ir (ilb)
	ilb_ir_t	*ilb;
{
	free ((char *) ilb);
}

/*
 * Generic ir to er conversion routine
 */

char *
mand_convert(ir, irtype, which_table, cllen, wdlen, flags)
	ilb_ir_t	*ir;
	int		irtype;	/* MAND_IL or MAND_SL */
	int		which_table;	/* see case statement below */
	int		cllen;	/* LONG_CLASS or SHORT_CLASS */
	int		wdlen;	/* LONG_WORDS or SHORT_WORDS */
	int		flags;
{
	char		**classwords;
	mask_t		*markings;
	struct l_tables	*table;

	if (mand_init() != 0)
		return NULL;

	switch (which_table) {
		case MAND_CLEARANCE_TABLE:
			table = l_clearance_tables;
			break;
		case MAND_IL_TABLE:
			table = l_information_label_tables;
			break;
		case MAND_SL_TABLE:
			table = l_sensitivity_label_tables;
			break;
		case MAND_CHANNEL_TABLE:
			table = l_channel_tables;
			break;
		case MAND_BANNER_TABLE:
			table = l_printer_banner_tables;
			break;
		default
			return NULL;
	}

	if (mand_er == NULL) {
		mand_er = malloc(table->l_max_length);
		mand_ersz = table->l_max_length;
	} else if (mand_ersz < table->l_max_length) {
		char *new = realloc(mand_er, table->l_max_length);

		if (new) {
			mand_er = new;
			mand_ersz = table->l_max_length;
		}
	}
	if (mand_er == NULL) {
		mand_ersz = 0;
		return NULL;
	}

	markings = (irtype == MAND_IL) ? &ir->cat[CATWORDS]
				       : (mask_t *) l_0_markings;

	classwords = (cllen == LONG_WORDS) ? l_long_classification
					   : l_short_classification;

	l_convert(mand_er, ir->class, classwords, ir->cat, markings, table,
			NO_PARSE_TABLE, wdlen, flags);

	return mand_er;
}

/* convert a sensitivity label internal representation to an
 * external representation.
 */

char *
mand_ir_to_er(ir)
	mand_ir_t	*ir;
{
	if (mand_init() != 0)
		return NULL;

	return mand_convert(ir, MAND_SL, MAND_SL_TABLE,
				LONG_CLASS, LONG_WORDS, ALL_ENTRIES);
}

/* convert a clearance label internal representation to an
 * external representation.
 */

char *
clearance_ir_to_er(ir)
	mand_ir_t	*ir;
{
	if (mand_init() != 0)
		return NULL;

	return mand_convert(ir, MAND_SL, MAND_CLEARANCE_TABLE,
				LONG_CLASS, LONG_WORDS, ALL_ENTRIES);
}

/* convert an information label internal representation to an
 * external representation
 */

char *
ilb_ir_to_er(ir)
	ilb_ir_t	*ir;
{
	if (mand_init() != 0)
		return NULL;

	return mand_convert(ir, MAND_IL, MAND_IL_TABLE,
				LONG_CLASS, LONG_WORDS, ALL_ENTRIES);
}

/*
 * Generic er to ir conversion routine
 */

ilb_ir_t *
mand_parse(er, which_table, minir, maxir, irtype, retir)
	char		*er;
	int		which_table;
	mand_ir_t	*minir;
	mand_ir_t	*maxir;
	int		irtype;		/* MAND_IL or MAND_SL */
	ilb_ir_t	*retir;
{
	ilb_ir_t	*ir;
	mask_t		*marks;
	struct l_tables	*table;

	if (mand_init() != 0)
		return (ilb_ir_t *) 0;
	
	if (retir)
		ir = retir;
	else
		ir = (irtype == MAND_SL) ? (ilb_ir_t *) mand_alloc_ir()
					 : ilb_alloc_ir();

	if (ir == (ilb_ir_t *) 0)
		return ir;

	if (irtype == MAND_SL) {
		MARKINGS_ZERO(l_t_markings);
		marks = (mask_t *) l_t_markings;
	} else
		marks = &ir->cat[CATWORDS];

	switch (which_table) {
		case MAND_CLEARANCE_TABLE:
			table = l_clearance_tables;
			break;
		case MAND_IL_TABLE:
			table = l_information_label_tables;
			break;
		case MAND_SL_TABLE:
			table = l_sensitivity_label_tables;
			break;
		case MAND_CHANNEL_TABLE:
			table = l_channel_tables;
			break;
		case MAND_BANNER_TABLE:
			table = l_printer_banner_tables;
			break;
		default
			return (ilb_ir_t *) 0;
	}

	if (l_parse(er, &ir->class, ir->cat, marks, table, minir->class,
			minir->cat, maxir->class, maxir->cat) != -1) {
		if (!retir)
			free(ir);
		return (ilb_ir_t *) 0;
	}

	return ir;
}

mand_ir_t *
mand_er_to_ir(er)
	char	*er;
{
	if (mand_init() != 0)
		return (mand_ir_t *) 0;

	return (mand_ir_t *) mand_parse(er, MAND_SL_TABLE,
					mand_syslo, mand_syshi, MAND_SL,
					(ilb_ir_t *) 0);
}

mand_ir_t *
clearance_er_to_ir(er)
	char	*er;
{
	if (mand_init() != 0)
		return (mand_ir_t *) 0;

	return (mand_ir_t *) mand_parse(er, MAND_CLEARANCE_TABLE,
					mand_minclrnce, mand_syshi, MAND_SL,
					(ilb_ir_t *) 0);
}

ilb_ir_t *
ilb_er_to_ir(er)
	char	*er;
{
	if (mand_init() != 0)
		return (ilb_ir_t *) 0;

	return mand_parse(er, MAND_IL_TABLE, mand_syslo,
			  mand_syshi, MAND_IL, (ilb_ir_t *) 0);
}

mand_in_accred_range(ir)
	mand_ir_t	*ir;
{
	return l_in_accreditation_range(ir->class, ir->cat);
}

/*
 * Check to see if the label encodings have been loaded, and load them if not.
 * Return 1 if successful, 0 if encodings cannot be loaded.
 */

l_encodings_initialized()
{
    int fd, ret;
    privvec_t oldprivs;
    extern priv_t *privvec();

    if (l_min_classification)	/* see if already initialized */
	return 1;
    
    if (forceprivs(privvec(SEC_ALLOWMACACCESS, SEC_ILNOFLOAT, -1), oldprivs))
	return 0;
    fd = open(LABEL_ENCODINGS, O_RDONLY);
    if (fd >= 0) {
	ret = l_dbload(fd);
	close(fd);
    }
    seteffprivs(oldprivs, (priv_t *) 0);

    return ret == 0;
}

/*
 * Load encodings from the specified open file
 */

static
l_dbload(fd)
    int fd;
{
	struct l_dbf_header h;
	char		*base;
	register char	*bp;
	register int	i;
	COMPARTMENTS	*junk;

#define Alloc(type,count,p)	(type *)p; p += (count) * sizeof(type)
#define Adjust(ptr,inc)		*(unsigned long *)&(ptr) += (unsigned long)(inc)
#define AdjustWords(table,inc) {\
	register struct l_word  *wp = table->l_words;\
	for (i = table->l_num_entries; --i >= 0; ) {\
		Adjust(wp[i].l_w_output_name, inc);\
		Adjust(wp[i].l_w_long_name, inc);\
		if (wp[i].l_w_short_name)\
			Adjust(wp[i].l_w_short_name, inc);\
		Adjust(wp[i].l_w_cm_mask, inc);\
		Adjust(wp[i].l_w_cm_value, inc);\
		Adjust(wp[i].l_w_mk_mask, inc);\
		Adjust(wp[i].l_w_mk_value, inc);\
	}}
#define AdjustConstraints(table,inc)	{\
	register struct l_constraints	*cp;\
	for (cp = table->l_constraints; cp->l_c_type != L_END; ++cp) {\
		Adjust(cp->l_c_first_list, inc);\
		Adjust(cp->l_c_second_list, inc);\
		Adjust(cp->l_c_end_second_list, inc);\
	}}

    if (read(fd, &h, sizeof h) != sizeof h ||
	    h.total_space <= h.table_space || h.table_space < 0)
	return -1;

    base = malloc(h.total_space + 1);
    if (base == (char *) 0)
	return -1;

    if (read(fd, base, h.total_space + 1) != h.total_space) {
	free(base);
	return -1;
    }

    /*
     * Initialize variables that point into the table space just read
     */

    bp = base;

    l_min_classification = Alloc(CLASSIFICATION, 1, bp);
    l_max_classification = Alloc(CLASSIFICATION, 1, bp);
    l_classification_protect_as = Alloc(CLASSIFICATION, 1, bp);
    l_lo_clearance = Alloc(struct l_sensitivity_label, 1, bp);
    l_lo_sensitivity_label = Alloc(struct l_sensitivity_label, 1, bp);
    l_long_classification = Alloc(char *, *l_max_classification + 1, bp);
    l_short_classification = Alloc(char *, *l_max_classification + 1, bp);
    l_in_compartments = Alloc(COMPARTMENTS *, *l_max_classification + 1, bp);
    l_in_markings = Alloc(MARKINGS *, *l_max_classification + 1, bp);
    l_accreditation_range = Alloc(struct l_accreditation_range,
				    *l_max_classification + 1, bp);
    l_information_label_tables = Alloc(struct l_tables, 1, bp);
    l_sensitivity_label_tables = Alloc(struct l_tables, 1, bp);
    l_clearance_tables = Alloc(struct l_tables, 1, bp);
    l_channel_tables = Alloc(struct l_tables, 1, bp);
    l_printer_banner_tables = Alloc(struct l_tables, 1, bp);

    /*
     * Check the size of the compartments and markings read from the
     * file for consistency with the configuration parameters.  Since
     * I neglected to include these parameters in the file header, we
     * must deduce the sizes by comparing pointers to adjacent items.
     * We rely on the fact that for the Nth classification word, the
     * items pointed to by l_in_compartments[N], l_in_markings[N],
     * l_long_classification[N], and l_short_classification[N] are
     * all contiguous in string space
     */

    if (((u_long) l_in_markings[*l_min_classification]
	    - (u_long) l_in_compartments[*l_min_classification])
		!= CATWORDS * sizeof(mask_t) ||
	((u_long) l_long_classification[*l_min_classification]
	    - (u_long) l_in_markings[*l_min_classification])
		!= MARKWORDS * sizeof(mask_t))
    {
	l_min_classification = (CLASSIFICATION *) 0;
	free(base);
	return -1;
    }

    /*
     * Initialize variables that point into the string space just read
     */

    bp = base + h.table_space;

    l_hi_compartments = (COMPARTMENTS *) Alloc(mask_t, CATWORDS, bp); 
	l_hi_markings = (MARKINGS *) Alloc(mask_t, MARKWORDS, bp);
    l_t_compartments = (COMPARTMENTS *) Alloc(mask_t, CATWORDS, bp);
    l_t_markings = (MARKINGS *) Alloc(mask_t, MARKWORDS, bp);
    l_comps_handled = (COMPARTMENT_MASK *) Alloc(mask_t, CATWORDS, bp);
    l_marks_handled = (MARKING_MASK *) Alloc(mask_t, MARKWORDS, bp);
    l_0_compartments = (COMPARTMENTS *) Alloc(mask_t, CATWORDS, bp);
    l_0_markings = (MARKINGS *) Alloc(mask_t, MARKWORDS, bp);
    /*
     * Skip over space used for l_lo_clearance->l_compartments and
     * l_lo_sensitivity_label->l_compartments.	Values for these
     * variables are read in from the file and adjusted below.
     */
    junk = (COMPARTMENTS *) Alloc(mask_t, CATWORDS, bp);
    junk = (COMPARTMENTS *) Alloc(mask_t, CATWORDS, bp);
    l_version = (char *) bp;

    /*
     * Now adjust embedded pointers which are stored in the file
     * as byte offsets from the base of allocated storage
     */

    Adjust(l_lo_clearance->l_compartments, base);
    Adjust(l_lo_sensitivity_label->l_compartments, base);

    for (i = 0; i <= *l_max_classification; ++i) {
	if (l_long_classification[i]) {
	    Adjust(l_long_classification[i], base);
	    Adjust(l_short_classification[i], base);
	    Adjust(l_in_compartments[i], base);
	    Adjust(l_in_markings[i], base);
	}
	if (l_accreditation_range[i].l_ar_start) {
	    Adjust(l_accreditation_range[i].l_ar_start, base);
	    Adjust(l_accreditation_range[i].l_ar_end, base);
	}
    }

    Adjust(l_information_label_tables->l_words, base);
    AdjustWords(l_information_label_tables, base);
    Adjust(l_information_label_tables->l_required_combinations, base);
    Adjust(l_information_label_tables->l_constraints, base);
    AdjustConstraints(l_information_label_tables, base);

    Adjust(l_sensitivity_label_tables->l_words, base);
    AdjustWords(l_sensitivity_label_tables, base);
    Adjust(l_sensitivity_label_tables->l_required_combinations, base);
    Adjust(l_sensitivity_label_tables->l_constraints, base);
    AdjustConstraints(l_sensitivity_label_tables, base);

    Adjust(l_clearance_tables->l_words, base);
    AdjustWords(l_clearance_tables, base);
    Adjust(l_clearance_tables->l_required_combinations, base);
    Adjust(l_clearance_tables->l_constraints, base);
    AdjustConstraints(l_clearance_tables, base);

    Adjust(l_channel_tables->l_words, base);
    AdjustWords(l_channel_tables, base);

    Adjust(l_printer_banner_tables->l_words, base);
    AdjustWords(l_printer_banner_tables, base);

    return 0;

#undef	Alloc
#undef	Adjust
#undef	AdjustWords
#undef	AdjustConstraints

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

#else /*}{*/

/*
 *  Mandatory access control subroutine library, SMP+ configuration files.
 *
 *  This library of subroutines allows users to map between external and
 *  internal representations for mandatory access control sensitivity
 *  labels.  The routines access the classes, categories, and synonyms
 *  databases.
 */



#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
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

/* Mandatory access control library routines:
 *	mand_nametocl()
 *	mand_cltoname()
 *	mand_nametocat()
 *	mand_cattoname()
 *	mand_end()
 *	mand_er_to_ir()
 *	mand_ir_to_er()
 *	mand_alloc_ir()
 *	mand_free_ir()
 *	mand_init()
 */

/* name table - stores a name and a number */
struct	name_table {
	char	*name;	/* pointer to string table */
	int	number;	/* value of name */
};

static unsigned mand_n_classes = 0;	/* number of classifications */
static unsigned mand_null_classes = 0;	/* number of null classifications */
static struct name_table *mand_class_by_name = (struct name_table *) 0;
static struct name_table *mand_class_by_number = (struct name_table *) 0;
static char *mand_class_strings = (char *) 0;

#define CLASS_SEPARATOR	':'

static unsigned mand_n_cats = 0;	/* number of categories */
static unsigned mand_null_cats = 0;	/* number of null categories */
static struct name_table *mand_cat_by_name = (struct name_table *) 0;
static struct name_table *mand_cat_by_number = (struct name_table *) 0;
static char *mand_cat_strings = (char *) 0;

#define CAT_SEPARATOR	':'

/* Buffer for use by ir to er conversion routine */

static char		*mand_er = NULL;	/* working er buffer */
static int		mand_ersz = 0;		/* number of bytes in it */
static int		syn_db_is_valid = 1;	/* indicates if syn DB valid */

#define	INIT_BUFSIZE	128

/* System low and system high definitions */

mand_ir_t	*mand_syslo, *mand_syshi;

/* Number of times to re-try for synonyms database */

#define SYN_LOCK_RETRIES	3

/* function definitions */

mand_ir_t	*er_to_ir();
static	char		*tempdbname();
static	int		getval();
static	void		print_one_syn();

/* external declarations */

extern char *malloc(), *realloc();

/* comparison function for quick sort.  Sorts by string value. */

static int
ccompar (ct1, ct2)
struct	name_table	*ct1;
struct	name_table	*ct2;
{
	return (strcmp (ct1->name, ct2->name));
}

/* comparison function for quick sort.  Sorts by number value. */

static int
ncompar (ct1, ct2)
struct	name_table	*ct1;
struct	name_table	*ct2;
{
	return (ct1->number - ct2->number);
}

/* make the classifications table from the classifications file
 * return 0 if successful, otherwise return -1.
 */

int
mand_make_class_table ()
{
	struct	stat	sb;
	register char	*cp, *cp1;
	int	line;
	struct	name_table	*mp;
	FILE	*fp;
	int	i;

	if (mand_class_strings != (char *) 0)
		return 0;

	if (stat (MAND_CLASS_FILE, &sb) < 0)  {
		fprintf(stderr, MSGSTR(MANDLIB_2, "Cannot stat MAC classification file "));
		perror(MAND_CLASS_FILE);
		return -1;
	}
	mand_class_strings = malloc (sb.st_size);
	if (mand_class_strings == (char *) 0)  {
		fprintf (stderr,
		    MSGSTR(MANDLIB_3, "Cannot allocate MAC classification string table\n"));
		return -1;
	}

	/* read the file into the buffer */
	fp = fopen (MAND_CLASS_FILE, "r");
	if (fp == (FILE *) 0)  {
		fprintf(stderr, MSGSTR(MANDLIB_4, "Cannot open MAC classification file "));
		perror(MAND_CLASS_FILE);
		goto out;
	}
	if (fread (mand_class_strings, sb.st_size, 1, fp) != 1)  {
		fprintf(stderr, MSGSTR(MANDLIB_5, "Cannot read MAC classification file "));
		perror(MAND_CLASS_FILE);
		fclose(fp);
		goto out;
	}
	fclose (fp);

	/* count up the number of classifications in the file */
	mand_n_classes = 0;
	for (cp = mand_class_strings; cp < mand_class_strings+sb.st_size; cp++)
		if (*cp == '\n')
			mand_n_classes++;

	/* allocate table space for the classification tables */
	mand_class_by_name = (struct name_table *)
	   calloc (mand_n_classes, sizeof (struct name_table));
	if (mand_class_by_name == (struct name_table *) 0) {
		fprintf (stderr,
			MSGSTR(MANDLIB_6, "Cannot allocate memory for MAC class name map\n"));
		goto out;
	}
	mand_class_by_number = (struct name_table *)
	   calloc (mand_n_classes, sizeof (struct name_table));
	if (mand_class_by_number == (struct name_table *) 0) {
		fprintf (stderr,
			MSGSTR(MANDLIB_7, "Cannot allocate memory for MAC class number map\n"));
		goto out;
	}
	/* parse classifications into class table */
	cp = mand_class_strings;
	mp = mand_class_by_name;
	
	mand_null_classes = 0;
	for (line = 1; cp < &mand_class_strings[sb.st_size]; line++, mp++) {
		cp1 = strchr (cp, CLASS_SEPARATOR);
		if (cp1 == (char *) 0)  {
			fprintf(stderr, MSGSTR(MANDLIB_8, "MAC classification file %s:\n"),
			  MAND_CLASS_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_9, "\tFormat error on line %d, no separator\n"), line);
			goto out;
		}
		*cp1 = '\0';
		/* parse the initial number */
		cp = &cp[strspn (cp, " \t")];
		if (!isdigit (*cp)) {
			fprintf(stderr, MSGSTR(MANDLIB_8, "MAC classification file %s:\n"),
			  MAND_CLASS_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_10, "\tFormat error on line %d, must start with digit\n"),
			  line);
			goto out;
		}
		mp->number = atoi (cp);
		cp1++;
		mp->name = cp1;
		cp = strchr (cp1, '\n');
		if (cp == (char *) 0)  {
			fprintf(stderr, MSGSTR(MANDLIB_8, "MAC classification file %s:\n"),
			  MAND_CLASS_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_11, "\tFormat error on line %d, no new-line\n"),
			  line);
			goto out;
		}
		*cp++ = '\0';
		/* check for a null classification, AFTER clearing \n */
		if (mp->name[0] == '\0')
			mand_null_classes++;
		if (mp->number > mand_max_class) {
			fprintf(stderr, MSGSTR(MANDLIB_8, "MAC classification file %s:\n"),
			  MAND_CLASS_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_12, "\tClassification %s, number %d is too large.\n"),
			  mp->name, mp->number);
			goto out;
		}
	}
	/* sort classification tables */
	qsort (mand_class_by_name, mand_n_classes, sizeof (struct name_table),
	  ccompar);
	(void) memcpy ((char *) mand_class_by_number,
		 (char *) mand_class_by_name,
		 sizeof (struct name_table) * mand_n_classes);
	qsort (mand_class_by_number, mand_n_classes,
	  sizeof (struct name_table), ncompar);
	/* check for duplicate classification numbers */
	for (i = 1; i < mand_n_classes; i++)
		if (mand_class_by_number[i].number ==
		  mand_class_by_number[i-1].number)  {
			fprintf(stderr, MSGSTR(MANDLIB_8, "MAC classification file %s:\n"),
			  MAND_CLASS_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_13, "\tClassification number %d duplicate.\n"),
			  mand_class_by_number[i].number);
			goto out;
		}
	return 0;
out:
	if (mand_class_strings != (char *) 0)
		free (mand_class_strings);
	if (mand_class_by_name != (struct name_table *) 0)
		free ((char *) mand_class_by_name);
	if (mand_class_by_number != (struct name_table *) 0)
		free ((char *) mand_class_by_number);
	mand_class_strings = (char *) 0;
	mand_class_by_name = (struct name_table *) 0;
	mand_class_by_number = (struct name_table *) 0;
	return -1;
}

/* lookup function for classification name.
 * returns a pointer to a static area, which should NOT be overwritten.
 */

char *
mand_cltoname (number)
int	number;
{
	register struct name_table	*mp;
	struct	name_table	table;

	if (mand_make_class_table() != 0)
		return (char *) 0;
	if (number < 0)
		return (char *) 0;
	if (number > mand_max_class)
		return (char *) 0;

	table.number = number;
	mp = (struct name_table *) bsearch ((char *) &table,
						(char *) mand_class_by_number,
						mand_n_classes, sizeof *mp,
						ncompar);
	if (mp == (struct name_table *) 0)
		return (char *) 0;
	if (mp->name[0] == '\0')
		return (char *) 0;

	return mp->name;
}

/* lookup function for a classification number.
 * returns the classification number, or -1 if not found
 */

int
mand_nametocl (name)
char	*name;
{
	register struct name_table	*mp, *startmp;
	struct	name_table	table;

	if (mand_make_class_table() != 0)
		return MAND_INVALID_CLASS;
	if (name == (char *) 0)
		return MAND_INVALID_CLASS;
	if (*name == '\0')
		return MAND_INVALID_CLASS;

	/* skip null classifications (will all sort to front) */
	startmp = &mand_class_by_name[mand_null_classes];
	table.name = name;
	mp = (struct name_table *) bsearch ((char *) &table, (char *) startmp,
					mand_n_classes - mand_null_classes,
					sizeof *mp, ccompar);
	if (mp == (struct name_table *) 0)
		return MAND_INVALID_CLASS;

	return mp->number;
}

/* make the categories table from the categories file
 * return 0 if successful, otherwise return -1.
 */

int
mand_make_cat_table ()
{
	struct	stat	sb;
	char	*cp, *cp1;
	int	line;
	struct	name_table	*mp;
	FILE	*fp;
	int	i;
	
	if (mand_cat_strings != (char *) 0)
		return 0;

	if (stat (MAND_CAT_FILE, &sb) < 0)  {
		fprintf(stderr, MSGSTR(MANDLIB_14, "Cannot stat MAC category file"));
		perror(MAND_CAT_FILE);
		return -1;
	}
	mand_cat_strings = malloc (sb.st_size);
	if (mand_cat_strings == (char *) 0)  {
		fprintf (stderr,
			MSGSTR(MANDLIB_15, "Cannot allocate MAC category string table\n"));
		return -1;
	}

	/* read the file into the buffer */
	fp = fopen (MAND_CAT_FILE, "r");
	if (fp == (FILE *) 0)  {
		fprintf(stderr, MSGSTR(MANDLIB_16, "Cannot open MAC category file "));
		perror(MAND_CAT_FILE);
		goto out;
	}
	if (fread (mand_cat_strings, sb.st_size, 1, fp) != 1)  {
		fprintf(stderr, MSGSTR(MANDLIB_17, "Cannot read MAC category file "));
		perror(MAND_CAT_FILE);
		fclose(fp);
		goto out;
	}
	fclose (fp);
	/* count up the number of categories in the file */
	mand_n_cats = 0;
	for (cp = mand_cat_strings; cp < &mand_cat_strings[sb.st_size]; cp++)
		if (*cp == '\n')
			mand_n_cats++;
	/* allocate table space for the classification tables */
	mand_cat_by_name = (struct name_table *)
	   calloc (mand_n_cats, sizeof (struct name_table));
	if (mand_cat_by_name == (struct name_table *) 0) {
		fprintf (stderr,
			MSGSTR(MANDLIB_18, "Cannot allocate memory for MAC category name map\n"));
		goto out;
	}
	mand_cat_by_number = (struct name_table *)
	   calloc (mand_n_cats, sizeof (struct name_table));
	if (mand_cat_by_number == (struct name_table *) 0) {
		fprintf (stderr,
			MSGSTR(MANDLIB_19, "Cannot allocate memory for MAC category number map\n"));
		goto out;
	}
	/* parse classifications into class table */
	cp = mand_cat_strings;
	mp = mand_cat_by_name;
	
	mand_null_cats = 0;
	for (line = 1; cp < &mand_cat_strings[sb.st_size]; line++, mp++) {
		cp1 = strchr (cp, CAT_SEPARATOR);
		if (cp1 == (char *) 0)  {
			fprintf(stderr, MSGSTR(MANDLIB_20, "MAC category file %s:\n"),
			  MAND_CAT_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_9, "\tFormat error on line %d, no separator\n"),
			  line);
			goto out;
		}
		*cp1 = '\0';
		/* parse the initial number */
		cp = &cp[strspn (cp, " \t")];
		if (!isdigit (*cp)) {
			fprintf(stderr, MSGSTR(MANDLIB_20, "MAC category file %s:\n"),
			  MAND_CAT_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_10, "\tFormat error on line %d, must start with digit\n"),
			  line);
			goto out;
		}
		mp->number = atoi (cp);
		cp1++;
		mp->name = cp1;
		cp = strchr (cp1, '\n');
		if (cp == (char *) 0)  {
			fprintf(stderr, MSGSTR(MANDLIB_20, "MAC category file %s:\n"),
			  MAND_CAT_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_11, "\tFormat error on line %d, no new-line\n"),
			  line);
			goto out;
		}
		*cp++ = '\0';
		/* check for a null category, AFTER clearing \n */
		if (mp->name[0] == '\0')
			mand_null_cats++;
		/* check for a number which doesn't agree with parameter file */
		if (mp->number > mand_max_cat) {
			fprintf(stderr, MSGSTR(MANDLIB_20, "MAC category file %s:\n"),
			  MAND_CAT_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_21, "\tCategory %s has number %d which is too large.\n"),
			  mp->name, mp->number);
			goto out;
		}
	}
	/* sort classification tables */
	qsort (mand_cat_by_name, mand_n_cats, sizeof (struct name_table),
	  ccompar);
	(void) memcpy ((char *) mand_cat_by_number,
		 (char *) mand_cat_by_name,
		 sizeof (struct name_table) * mand_n_cats);
	qsort (mand_cat_by_number, mand_n_cats,
	  sizeof (struct name_table), ncompar);
	/* check for duplicate category numbers */
	for (i = 1; i < mand_n_cats; i++)
		if (mand_cat_by_number[i].number ==
		  mand_cat_by_number[i-1].number)  {
			fprintf(stderr, MSGSTR(MANDLIB_20, "MAC category file %s:\n"),
			  MAND_CAT_FILE);
			fprintf(stderr,
			  MSGSTR(MANDLIB_22, "\tCategory number %d duplicate.\n"),
			  mand_cat_by_number[i].number);
			goto out;
		}
	return 0;
out:
	if (mand_cat_strings != (char *) 0)
		free (mand_cat_strings);
	if (mand_cat_by_name != (struct name_table *) 0)
		free ((char *) mand_cat_by_name);
	if (mand_cat_by_number != (struct name_table *) 0)
		free ((char *) mand_cat_by_number);
	mand_cat_strings = (char *) 0;
	mand_cat_by_name = (struct name_table *) 0;
	mand_cat_by_number = (struct name_table *) 0;
	return -1;
}

/* lookup function for category name.
 * returns a pointer to a static area, which should NOT be overwritten.
 */

char *
mand_cattoname (number)
int	number;
{
	register struct name_table	*mp;
	struct	name_table	table;

	if (mand_make_cat_table() != 0)
		return (char *) 0;
	if (number < 0)
		return (char *) 0;
	if (number > mand_max_cat)
		return (char *) 0;

	table.number = number;
	mp = (struct name_table *) bsearch ((char *) &table,
						(char *) mand_cat_by_number,
						mand_n_cats, sizeof *mp,
						ncompar);
	if (mp == (struct name_table *) 0)
		return (char *) 0;
	if (mp->name[0] == '\0')
		return (char *) 0;

	return (mp->name);
}

/* lookup function for a category number.
 * returns the category number, or -1 if not found
 * Note that all categories without name definitions also return -1.
 */

int
mand_nametocat (name)
char	*name;
{
	register struct name_table	*mp, *startmp;
	struct	name_table	table;

	if (mand_make_cat_table() != 0)
		return MAND_INVALID_CAT;
	if (name == (char *) 0)
		return MAND_INVALID_CAT;
	if (*name == '\0')
		return MAND_INVALID_CAT;

	/* skip null categories (will all sort to front) */
	startmp = &mand_cat_by_name[mand_null_cats];
	table.name = name;
	mp = (struct name_table *) bsearch ((char *) &table, (char *) startmp,
						mand_n_cats - mand_null_cats,
						sizeof *mp, ccompar);
	if (mp == (struct name_table *) 0)
		return MAND_INVALID_CAT;

	return (mp->number);
}

/* routine to look up a literal category.
 * must be digits, followed by colon, and nothing else.
 */

static int
mand_literal_cat (name)
char	*name;
{
	register int len = strlen (name) - 1;
	register int cat;

	if (!isdigit (name[0]) || strspn (name, "0123456789") != len ||
	    name[len] != CAT_SEPARATOR || (cat = atoi(name)) > mand_max_cat)
		return MAND_INVALID_CAT;
	return cat;
}

/* subroutine to read the synonym file into memory, from its binary
 * representation.
 * returns number of synonyms on success, -1 on failure.
 */

static struct synonym	*syn_table = (struct synonym *) 0;
static int	nsynonyms;

struct synonym {
	char		*name;
	mask_t		*cat_set;
	class_ir_t	class;
	int		type;
};

/*
 * structure of a synonym table synonym definition-used for mand_syns
 */

struct synonym_tbl {
	int	type;
	char	*name;
	long	class;
	mask_t	*cat_set;
	struct synonym_tbl *next;
};


int
mand_make_synonyms (fp)
FILE	*fp;
{
	int	nstrings;
	int	ncategories;
	register int	i;
	struct	syn_file	syn;
	mask_t	*cat_masks = (mask_t *) 0;
	char	*string_table = (char *) 0;

	/* classes and cats must be initialized for CATWORDS to be defined.  */
	if (mand_init() != 0)
		return -1;

	/* read parameters out of file */
	nsynonyms = getw (fp);
	ncategories = getw (fp);
	nstrings = getw (fp);
	if (nsynonyms < 0 || ncategories < 0 || nstrings < 0 || ferror (fp)) {
		nsynonyms = 0;
		goto read_out;
	}

	/* synonym count may be zero-return 0 count */

	if(nsynonyms == 0)
		return(nsynonyms);

	/* allocate space */
	syn_table = (struct synonym *) calloc (nsynonyms, sizeof (*syn_table));
	cat_masks = (mask_t *) calloc (ncategories, CATWORDS * sizeof(mask_t));
	string_table = malloc (nstrings);
	if (syn_table == (struct synonym *) 0 || cat_masks == (mask_t *) 0 ||
	    string_table == (char *) 0) {
		fprintf (stderr, MSGSTR(MANDLIB_23, "Memory allocation error reading synonyms.\n"));
		goto out;
	}

	/* read in synonyms */
	for (i = 0; i < nsynonyms; i++) {
		if (fread ((char *) &syn, sizeof (syn), 1, fp) != 1)
			goto read_out;
		syn_table[i].name = &string_table[syn.name];
		if (syn.type != CLASS_SYN)
			syn_table[i].cat_set = &cat_masks[CATWORDS*syn.cat_set];
		else
			syn_table[i].cat_set = (mask_t *) 0;
		syn_table[i].class = syn.class;
		syn_table[i].type = syn.type;
	}

	/* read in categories */
	if (fread ((char *) cat_masks, sizeof (mask_t),
			ncategories * CATWORDS, fp) != ncategories * CATWORDS)
		goto read_out;

	/* read in string table */
	if (fread (string_table, nstrings, 1, fp) != 1)
		goto read_out;
	
	return (nsynonyms);

read_out:
	fprintf (stderr, MSGSTR(MANDLIB_24, "Read error on synonym file.\n"));
out:
	if (syn_table != (struct synonym *) 0)
		free ((char *) syn_table);
	if (cat_masks != (mask_t *) 0)
		free ((char *) cat_masks);
	if (string_table != (char *) 0)
		free (string_table);
	nsynonyms = 0;
	syn_db_is_valid = 0;
	return -1;
}

static
syn_compar (syn1, syn2)
struct synonym *syn1, *syn2;
{
	return (strcmp (syn1->name, syn2->name));
}

/* lookup routine for data structure built in last program.
 * returns a pointer to a newly-allocated mandatory ir on success,
 * a NULL pointer on failure.
 */

static struct synonym *
mand_lookup_synonym (word)
char	*word;
{
	register int	i;
	register struct synonym	*retsyn;
	struct synonym	syn;
	FILE	*fp = (FILE *) 0;
	extern int errno;
	struct flock fl;
	struct stat	statbuf;

	if (stat(MAND_SYN_DB, &statbuf) == -1) {
		syn_db_is_valid = 0;
		return((struct synonym *) 0);
	}

	if (nsynonyms == 0) { /* synonyms not read in yet. */
		/* open the file and put a read lock on it */
		fl.l_type = F_RDLCK;
		fl.l_whence = fl.l_start = fl.l_len = 0;
		for (i = 0; i < SYN_LOCK_RETRIES && fp == (FILE *) 0; i++) {
			fp = fopen (MAND_SYN_DB, "r");
			if (fp != (FILE *) 0) {
				if (fcntl (fileno (fp), F_SETLK, &fl) < 0)
					if (errno == EAGAIN ||
					    errno == EACCES) {
						(void) fclose (fp);
						fp = (FILE *) 0;
						sleep (1);
						continue;
					}
					else {
						(void) fclose (fp);
						return
						 ((struct synonym *) 0);
					}
				else
					break;
			}
			else
				sleep (1);
		}
		if (i == SYN_LOCK_RETRIES) {
			if (fp != (FILE *) 0)
				(void) fclose (fp);
			return ((struct synonym *) 0);
		}	
		i = mand_make_synonyms (fp);

		/* unlock the file */
		fl.l_type = F_UNLCK;
		fl.l_whence = fl.l_start = fl.l_len = 0;
		(void) fcntl (fileno(fp), F_SETLK, &fl);
		(void) fclose (fp);
		if (i == -1)
			return ((struct synonym *) 0);
	}
	syn.name = word;
	retsyn = (struct synonym *) bsearch ((char *) &syn,
	  (char *) syn_table, nsynonyms, sizeof (syn), syn_compar);
	return (retsyn);
}

void
mand_lookup_and_print_syn (word)
char	*word;
{
	register struct synonym *syn;
	register int i;

	syn = mand_lookup_synonym(word);
	if (syn == (struct synonym *) 0) {
		printf(MSGSTR(MANDLIB_25, "Synonym %s not found\n"), word);
		return;
	}

	if (mand_init() != 0)
		return;

	print_one_syn(syn);
}

/*
 * print a single synonym on standard output.  Use printbuf() to print
 * the category set.
 */

static void
print_one_syn(syn)
register struct synonym *syn;
{
	register int j;

	switch (syn->type) {
	   case	CLASS_SYN:
		printf(MSGSTR(MANDLIB_26, "Synonym: %s Type: "),syn->name);
		printf(MSGSTR(MANDLIB_27, "Classification synonym\n\tClassification: %s\n\n"),
			mand_cltoname(syn->class));
		break;
	   case	CATEGORY_SYN:
	   {
		int bufsize = 0;

		printf(MSGSTR(MANDLIB_26, "Synonym: %s Type: "),syn->name);
		printf(MSGSTR(MANDLIB_28, "Category synonym\n\tCategories:     "));
		for (j = 0; j <= mand_max_cat; j++)
			if (ISBITSET(syn->cat_set, j))
				bufsize += strlen(mand_cattoname(j)) + 1;
		if (bufsize == 0)
			putchar('\n');
		else {
			char *buf;

			if ((buf = malloc(bufsize)) == NULL) {
				printf(MSGSTR(MANDLIB_29, "Memory Allocation Error.\n"));
				break;
			}
			buf[0] = '\0';
			for (j = 0; j <= mand_max_cat; j++)
				if (ISBITSET(syn->cat_set, j)) {
					if (buf[0] != '\0')
						strcat(buf, " ");
					strcat(buf, mand_cattoname(j));
				}
			printbuf(buf, 24, " ");
			free(buf);
		}
		putchar('\n');
		break;
	   }
	   case	SENS_LABEL_SYN:
	   {
		int bufsize = 0;

		printf(MSGSTR(MANDLIB_26, "Synonym: %s Type: "),syn->name);
		printf(MSGSTR(MANDLIB_30, "Sensitivity Label synonym\n\tClassification: %s"),
			mand_cltoname(syn->class));
		printf(MSGSTR(MANDLIB_31, "\n\tCategories:     "));
		for (j = 0; j <= mand_max_cat; j++)
			if (ISBITSET(syn->cat_set, j))
				bufsize += strlen(mand_cattoname(j)) + 1;
		if (bufsize == 0)
			putchar('\n');
		else {
			char *buf;

			if ((buf = malloc(bufsize)) == NULL) {
				printf(MSGSTR(MANDLIB_29, "Memory Allocation Error.\n"));
				break;
			}
			buf[0] = '\0';
			for (j = 0; j <= mand_max_cat; j++)
				if (ISBITSET(syn->cat_set, j)) {
					if (buf[0] != '\0')
						strcat(buf, " ");
					strcat(buf, mand_cattoname(j));
				}
			printbuf(buf, 24, " ");
			free(buf);
		}
		putchar ('\n');
		break;
	   }
	   } /* end switch */
}

/*
 * Print all of the synonyms in the synonym list.
 */

void
mand_print_all_syns()
{
	register int i;

	/* Cycle through the entire synonym list */

	for(i=0; i < nsynonyms; i++)
		print_one_syn(&syn_table[i]);
}

/*
 * Create the synonym database file from the internal list
 * into the file specified by the argument.
 */

mand_store_syns(dbfile,mand_syns)
char *dbfile;
struct synonym_tbl *mand_syns;
{
	struct stat	stb;
	FILE		*fp;
	char		*newdbfile;
	int		ret, fd;

	if (dbfile == NULL)
		return 1;

	newdbfile = tempdbname(dbfile, "mand");
	if (newdbfile == NULL)
		return 1;

	fd = open(newdbfile, O_RDWR|O_CREAT|O_EXCL, 0600);
	if (fd < 0) {
		free(newdbfile);
		return 1;
	}
	fp = fdopen(fd, "r+");
	if (fp == NULL) {
		close(fd);
		return 1;
	}

	ret = mand_rewrite_synonym_db(fp,mand_syns);
	fclose(fp);

	if (ret == 0) {
		if (stat(dbfile, &stb) == 0) {
			chmod(newdbfile, stb.st_mode);
			chown(newdbfile, stb.st_uid, stb.st_gid);
			unlink(dbfile);
		}
		if (link(newdbfile, dbfile) != 0)
			ret = 1;
	}

	unlink(newdbfile);
	free(newdbfile);
	return ret;
}

/*
 * Re-write the synonym list to a file from the internal table. This
 * routine is used to rewrite the database after it has been read but
 * the yyparse() routine is not called to build mand_syns.
 * returns 0 on success, 1 on write error
 */

int
mand_rewrite_synonym_db (fp,mand_syns)
FILE *fp;
struct synonym_tbl *mand_syns;
{
	struct synonym_tbl *newsyn;		/* synonym list pointer */
	int	ncategories = 0;	/* number of synonyms with cats */
	int	nstrings = 0;		/* total size of string table */
	struct	syn_file	syn;	/* buffer for output file */
	int	cat_ctr = 0;		/* running counter, category masks */
	int	str_ctr = 0;		/* running counter, string length */
	int	i;			/* syn_table index */
	int	old_synonyms;		/* original synonym count */
	int	count = 0;		/* synonym rewrite loop counter */

	/* count up total sizes for strings, categories, and synonyms */

	old_synonyms = nsynonyms;

	for (i=0; i < old_synonyms; i++) {
		if (syn_table[i].type != CLASS_SYN)
			ncategories++;
		nstrings += strlen (syn_table[i].name) + 1;
	}

	/* do the mand_syn list built by yyparse() if non-null */

	for(newsyn = mand_syns; newsyn != (struct synonym_tbl *) 0; 
	    newsyn = newsyn->next) {
		if (newsyn->type != CLASS_SYN)
			ncategories++;
		nstrings += strlen (newsyn->name) + 1;
		nsynonyms++;
	}

	/* write the word parameters to the file */

	(void) putw (nsynonyms, fp);
	(void) putw (ncategories, fp);
	(void) putw (nstrings, fp);
	if (ferror (fp))
		goto write_error;

	/* write headers-merge from syn_table[] and mand_syns */

	i = 0;
	newsyn = mand_syns;

	for (count=0; count < nsynonyms; count++) {

	  if((i < old_synonyms) && (newsyn)) {

	/* Look for duplicates and terminate if found. */

	     if(strcmp(syn_table[i].name,newsyn->name) == 0) {
		fprintf(stderr,MSGSTR(MANDLIB_32, "mandsyn: duplicate synonym-%s\n"),newsyn->name);
		return(1);
	     }

	     if(strcmp(syn_table[i].name,newsyn->name) < 0) {
		syn.class = syn_table[i].class;
		syn.name = str_ctr;
		syn.cat_set = (syn_table[i].type == CLASS_SYN) ? 0 : cat_ctr;
		syn.type = syn_table[i].type;
		if (fwrite ((char *) &syn, sizeof (syn), 1, fp) != 1)
			goto write_error;
		str_ctr += strlen (syn_table[i].name) + 1;
		if (syn_table[i].type != CLASS_SYN)
			cat_ctr++;
		i++;
	     }
	     else {
		syn.class = newsyn->class;
		syn.name = str_ctr;
		syn.cat_set = (newsyn->type == CLASS_SYN) ? 0 : cat_ctr;
		syn.type = newsyn->type;
		if (fwrite ((char *) &syn, sizeof (syn), 1, fp) != 1)
			goto write_error;
		str_ctr += strlen (newsyn->name) + 1;
		if (newsyn->type != CLASS_SYN)
			cat_ctr++;
		newsyn = newsyn->next;
	     }
	  }
	  else if(i < old_synonyms) {	/* only syn_table elements left */
		syn.class = syn_table[i].class;
		syn.name = str_ctr;
		syn.cat_set = (syn_table[i].type == CLASS_SYN) ? 0 : cat_ctr;
		syn.type = syn_table[i].type;
		if (fwrite ((char *) &syn, sizeof (syn), 1, fp) != 1)
			goto write_error;
		str_ctr += strlen (syn_table[i].name) + 1;
		if (syn_table[i].type != CLASS_SYN)
			cat_ctr++;
		i++;
	  }
	  else {	/* only mand_syns elements left */
		syn.class = newsyn->class;
		syn.name = str_ctr;
		syn.cat_set = (newsyn->type == CLASS_SYN) ? 0 : cat_ctr;
		syn.type = newsyn->type;
		if (fwrite ((char *) &syn, sizeof (syn), 1, fp) != 1)
			goto write_error;
		str_ctr += strlen (newsyn->name) + 1;
		if (newsyn->type != CLASS_SYN)
			cat_ctr++;
		newsyn = newsyn->next;
	  }
	}

	/* write categories-merge from syn_table[] and mand_syn */

	i = 0;
	newsyn = mand_syns;

	for (count=0; count < nsynonyms; count++) {
	  if((i < old_synonyms) && (newsyn)) {
	     if(strcmp(syn_table[i].name,newsyn->name) < 0) {
		if (syn_table[i].type != CLASS_SYN)
			if(fwrite ((char *) syn_table[i].cat_set,sizeof(mask_t),
			  CATWORDS, fp) != CATWORDS)
				goto write_error;
		i++;
	     }
	     else {
		if (newsyn->type != CLASS_SYN)
			if(fwrite ((char *) newsyn->cat_set,sizeof(mask_t),
			  CATWORDS, fp) != CATWORDS)
				goto write_error;
		newsyn = newsyn->next;
	     }
	  }
	  else if(i < old_synonyms) {	/* only syn_table elements left */
		if (syn_table[i].type != CLASS_SYN)
			if(fwrite ((char *) syn_table[i].cat_set,sizeof(mask_t),
			  CATWORDS, fp) != CATWORDS)
				goto write_error;
		i++;
	  }
	  else {	/* only mand_syn elements left */
		if (newsyn->type != CLASS_SYN)
			if(fwrite ((char *) newsyn->cat_set,sizeof(mask_t),
			  CATWORDS, fp) != CATWORDS)
				goto write_error;
		newsyn = newsyn->next;
	  }
	}

	/* write strings-merge from syn_table[] and mand_syns */

	i = 0;
	newsyn = mand_syns;

	for (count=0; count < nsynonyms; count++) {
	  if((i < old_synonyms) && (newsyn)) {
	     if(strcmp(syn_table[i].name,newsyn->name) < 0) {
		if (fwrite (syn_table[i].name,
				strlen (syn_table[i].name) + 1, 1, fp) != 1)
			goto write_error;
		i++;
	     }
	     else {
		if (fwrite (newsyn->name,
				strlen (newsyn->name) + 1, 1, fp) != 1)
			goto write_error;
		newsyn = newsyn->next;
	     }
	  }
	  else if(i < old_synonyms) {	/* only syn_table[] elements left */
		if (fwrite (syn_table[i].name,
				strlen (syn_table[i].name) + 1, 1, fp) != 1)
			goto write_error;
		i++;
	  }
	  else {	/* only mand_syn elements left */
		if (fwrite (newsyn->name,
				strlen (newsyn->name) + 1, 1, fp) != 1)
			goto write_error;
		newsyn = newsyn->next;
	  }
	}

	(void) fclose (fp);
	return (0);

write_error:
	(void) fclose (fp);
	(void) fprintf (stderr, MSGSTR(MANDLIB_33, "Write error on synonym database file.\n"));
	return (1);
}

/* Delete a synonym entry when found */

mand_delete_syn(dbfile,synonym)
char *dbfile;
char *synonym;
{
	register struct synonym *syn, *syntab;
	register int i, found = 0;

	if((syn_table == NULL) || (nsynonyms <= 0))
		return(-1);

	syn = mand_lookup_synonym(synonym);

	if(syn == (struct synonym *) 0) {
		fprintf(stderr,MSGSTR(MANDLIB_34, "Unable to locate synonym for deletion: %s\n"),
			synonym);
		return(-1);
	}

	/* Search the table and compress the entries */

	for(syntab = syn_table; syntab < syn_table + nsynonyms; syntab++) {

		if(syntab == syn) {
			for(syn++;syn < syn_table + nsynonyms; syn++,syntab++) {
				syntab->name = syn->name;
				syntab->type = syn->type;
				syntab->class = syn->class;
				syntab->cat_set = syn->cat_set;
			}
			found = 1;
			break;
		}
	}

	nsynonyms--;

	if(!found) {
		fprintf(stderr,MSGSTR(MANDLIB_35, "Could not delete the specified synonym-%s\n"),
			synonym);
		return(-1);
	}

	/* Rewrite the file from the internal list */

	if(mand_store_syns(dbfile,(struct synonym_tbl *) 0))
		return(-1);

	return(0);
}

/*
 * Make a temporary filename from the supplied pathname.
 * The temp file is in the same directory as the original
 * so that it can be renamed to replace the original.
 */
static char *
tempdbname(path, seed)
	char	*path, *seed;
{
	register char	*cp;
	char		*newfile;

	/*
	 * Make a copy of the original pathname.
	 * Get enough extra space to hold the temp
	 * component name (length of seed string +
	 * 5 bytes for pid + 1 byte for $ + 1 byte
	 * for null terminator.
	 */
	newfile = malloc(strlen(path) + strlen(seed) + 7);
	if (newfile == NULL)
		return NULL;
	strcpy(newfile, path);

	cp = strrchr(newfile, '/');
	if (cp && cp[1] == '\0') {	/* allow for / at end of pathname */
		while (cp > newfile && *--cp == '/')
			;
		cp[1] = '\0';
		cp = strrchr(newfile, '/');
	}
	if (cp == NULL)
		cp = newfile;
	else
		++cp;
	sprintf(cp, "%s$%d", seed, getpid());
	return newfile;
}

/* free up space occupied by the databases */

void
mand_end ()
{
	register struct synonym *this;
	register int i;

	if (mand_class_strings) {
		free (mand_class_strings);
		free ((char *) mand_class_by_number);
		free ((char *) mand_class_by_name);
		mand_class_strings   = (char *) 0;
		mand_class_by_number = (struct name_table *) 0;
		mand_class_by_name   = (struct name_table *) 0;
	}
	if (mand_cat_strings) {
		free (mand_cat_strings);
		free ((char *) mand_cat_by_name);
		free ((char *) mand_cat_by_number);
		mand_cat_strings   = (char *) 0;
		mand_cat_by_name   = (struct name_table *) 0;
		mand_cat_by_number = (struct name_table *) 0;
	}
	if (nsynonyms > 0) {
		free (syn_table->name);
		for (i = 0, this = syn_table; i < nsynonyms; i++, this++)
			if (this->type != CLASS_SYN) {
				free ((char *) this->cat_set);
				break;
			}
		free ((char *) syn_table);
		syn_table = (struct synonym *) 0;
		nsynonyms = 0;
	}
	if (mand_er) {
		free (mand_er);
		mand_er = (char *) 0;
		mand_ersz = 0;
	}

	return;
}

/* convert a mandatory access control internal representation to an
 * external representation.
 * The function returns:
 *   a pointer to a static buffer containing the external representation
 *   NULL if the internal rep contains categories which are NULL or invalid
 */

char *
mand_ir_to_er (mand_ir)
mand_ir_t	*mand_ir;
{
	char	*word;
	int	first_cat = 1;
	int	i, cc;

	if (mand_init() != 0)
		return NULL;

	/*
	 * Allocate a buffer to hold the external representation.
	 * We retain this buffer between calls and grow it as
	 * needed.
	 */
	if (mand_er == NULL) {
		mand_er = malloc(INIT_BUFSIZE);
		if (mand_er == NULL)
			return NULL;
		mand_ersz = INIT_BUFSIZE;
	}

	word = mand_cltoname (mand_ir->class);
	if (word == (char *) 0)
		return NULL;

	cc = strlen (word);
	while (cc + 4 > mand_ersz) {	/* leave room for at least " //\0" */
		char	*nbp = realloc(mand_er, mand_ersz + INIT_BUFSIZE);

		if (nbp == NULL)
			return NULL;
		mand_er = nbp;
		mand_ersz += INIT_BUFSIZE;
	}
	(void) strcpy (mand_er, word);
	strcat (mand_er, " /");
	cc += 2;

	/* now get categories */
	for (i = 0; i <= mand_max_cat; i++)
		if (ISBITSET (mand_ir->cat, i)) {
			if (!first_cat)	/* save room for ", " separator */
				cc += 2;
			word = mand_cattoname (i);
			cc += strlen (word);
			while (cc + 2 > mand_ersz) {	/* trailing "/\0" */
				char	*nbp;

				nbp = realloc(mand_er,
						mand_ersz + INIT_BUFSIZE);
				if (nbp == NULL)
					return NULL;
				mand_er = nbp;
				mand_ersz += INIT_BUFSIZE;
			}
			if (!first_cat)
				strcat (mand_er, ", ");
			strcat (mand_er, word);
			first_cat = 0;
		}

	/* append the final slash */
	strcat (mand_er, "/");

	return mand_er;
}

/*
 * Clearance internal to external representation mapping routine.
 * For the non-Encodings based version of this routine, return
 * mand_ir_to_er()
 */

char *
clearance_ir_to_er (mand_ir)
mand_ir_t	*mand_ir;
{
	return mand_ir_to_er(mand_ir);
}

/*
 * mand_convert translates an internal representation to an external
 * one.  This is a wrapper routine for mand_ir_to_er().
 */

char *
mand_convert(ir, irtype, which_table, cllen, wdlen, flags)
	mand_ir_t	*ir;
	int		irtype;
	int		which_table;
	int		cllen;
	int		wdlen;
	int		flags;
{
	return mand_ir_to_er(ir);
}

/* allocate a mandatory internal representation.
 * returns a pointer to it on success, or NULL pointer on failure
 */

mand_ir_t *
mand_alloc_ir ()
{
	register mand_ir_t	*mand;
	register int		i;

	if (mand_init() != 0)
		return ((mand_ir_t *) 0);

	mand = (mand_ir_t *) malloc (mand_bytes());
	if (mand != (mand_ir_t *) 0)
		for (i = 0; i < CATWORDS; i++)
			mand->cat[i] = (mask_t) 0;
	return (mand);
}

/* free a mandatory internal representation.
 */

void
mand_free_ir (mand)
mand_ir_t	*mand;
{
	free ((char *) mand);
}

/* convert an external to an internal representation, consulting the
 * synonym list if check_syns is set.
 * returns:
 *   a pointer to a newly-allocated mandatory structure or a NULL Pointer.
 */

static	char lexword[100];	/* for communication with lexval */

/* lexical analyzer tokens */
#define COMMA 1
#define WORD 2
#define SLASH 3
#define NEWLINE 4

/* external interface to er_to_ir.
 * always check synonyms, which doesn't allow literal specification
 * of categories ("n:").
 */

mand_ir_t *
mand_er_to_ir (word)
char	*word;
{
	if (mand_init() != 0)
		return (mand_ir_t *) 0;

	return (er_to_ir (word, 1));
}

/*
 * clearance version of external to internal translation
 * for non-Encodings systems, same as mand_er_to_ir()
 */

mand_ir_t *
clearance_er_to_ir (word)
char	*word;
{
	if (mand_init() != 0)
		return (mand_ir_t *) 0;

	return (er_to_ir (word, 1));
}

/*
 * mand_parse converts an external representation to an internal one.
 * For non-Encodings systems, call mand_er_to_ir() appropriately.
 */

mand_ir_t *
mand_parse(word, which_table, minir, maxir, irtype, retir)
char *word;
int which_table;
mand_ir_t *minir, *maxir;
int irtype;
mand_ir_t *retir;
{
	mand_ir_t *ir;

	if (mand_init() != 0)
		return (mand_ir_t *) 0;

	ir = er_to_ir(word, 1);
	if (retir != (mand_ir_t *) 0 && ir != (mand_ir_t *) 0) {
		memcpy(retir, ir, mand_bytes());
		free(ir);
		return retir;
	}
	return ir;
}

mand_ir_t *
er_to_ir (word, check_syns)
char	*word;
int	check_syns;
{
	int	token;
	register int	i;
	char	buffer[100];
	mand_ir_t	*mand;
	struct synonym	*syn;
	int	class_number;
	int	cat_number;
	int	have_cats;

	if (mand_init() != 0)
		return (mand_ir_t *) 0;

	/* first get words until you've found a classification, a
	 * classification synonym, or a sensitivity label synonym
	 */

	buffer[0] = '\0';
	token = getval (word);
	for ( ;; token = getval ((char *) 0)) {
		if (token != WORD)
			return ((mand_ir_t *) 0);
		else {
			if (buffer[0] == '\0')
				strcpy (buffer, lexword);
			else {
				strcat (buffer, " ");
				strcat (buffer, lexword);
			}
		}
		class_number = mand_nametocl (buffer);
		if (class_number != MAND_INVALID_CLASS) {
			mand = mand_alloc_ir ();
			if (mand == (mand_ir_t *) 0)
				return (mand);
			mand->class = class_number;
			break;
		}
		if (check_syns) {
			syn = mand_lookup_synonym (buffer);
			if (syn == (struct synonym *) 0)
				continue;
			if (syn->type == SENS_LABEL_SYN) {
				mand = mand_alloc_ir ();
				if (mand == (mand_ir_t *) 0)
					return (mand);
				token = getval ((char *) 0);
				if (token != NEWLINE) {
					mand_free_ir (mand);
					return ((mand_ir_t *) 0);
				}
				mand->class = syn->class;
				for (i = 0; i < CATWORDS; i++)
					mand->cat[i] = syn->cat_set[i];
				return (mand);
			} else if (syn->type == CLASS_SYN) {
				mand = mand_alloc_ir ();
				if (mand == (mand_ir_t *) 0)
					return (mand);
				mand->class = syn->class;
				break;
			} else
				return ((mand_ir_t *) 0);
		}
	}
			
	/* now add a category literal or a category synonym */
	buffer[0] = '\0';

	token = getval ((char *) 0);
	if (token == SLASH) {  /* category literal list */
		have_cats = 0; /* no categories parsed yet */
		do {
			/* collect words into tokens */
			while ((token = getval ((char *) 0)) == WORD) {
				if (buffer[0] != '\0')
					strcat (buffer, " ");
				strcat (buffer, lexword);
			}
			/* check for null category list (//) */
			if (buffer[0] == '\0' && token == SLASH && !have_cats)
				break;
			cat_number = mand_nametocat (buffer);

			/* if not checking synonyms, check for a literal
			 * category number.
			 */
			if (check_syns == 0 && cat_number == MAND_INVALID_CAT)
				cat_number = mand_literal_cat (buffer);

			if (cat_number == MAND_INVALID_CAT ||
			    !(token == COMMA || token == SLASH)) {
				mand_free_ir (mand);
				return ((mand_ir_t *) 0);
			}
			ADDBIT (mand->cat, cat_number);
			buffer[0] = '\0';
			have_cats = 1;  /* a category has been parsed */
		} while (token == COMMA);

		/* we ate the categories. now check the end of line */
		if (token == SLASH) {
			token = getval ((char *) 0);
			if (token == NEWLINE)
				return (mand);
			else {
				mand_free_ir (mand);
				return ((mand_ir_t *) 0);
			}
		}
	} else if (token == WORD && check_syns) {
		/* must be a category synonym */
		buffer[0] = '\0';
		do {
			if (buffer[0] != '\0')
				strcat (buffer, " ");
			strcat (buffer, lexword);
		} while ((token = getval ((char *) 0)) == WORD);
		syn = mand_lookup_synonym (buffer);
		if ( token != NEWLINE ||
		     syn == (struct synonym *) 0 ||
		     syn->type != CATEGORY_SYN )  {
			mand_free_ir (mand);
			return ((mand_ir_t *) 0);
		}
		for (i = 0; i < CATWORDS; i++)
			mand->cat[i] = syn->cat_set[i];
		return (mand);
	} else {
		mand_free_ir (mand);
		return ((mand_ir_t *) 0);
	}
}

/* lexical analyzer for mand_er_to_ir().
 * returns a token and sets lexword to current token */

static int
getval(word)
char *word;
{
	int	this_token;
	static	char	*cp;
	char	*begin_token, *end_token;
	int	size;
	char	c;
	char	*tokens = "/,";
	char	*white = " \t";

	if (word != (char *) 0) {
		/* skip white space at beginning of line */
		cp = &word[strspn (word, white)];
		if (*cp == '\0')
			return (NEWLINE);
	}
	switch (*cp) {
	case	'/':
		this_token = SLASH;
		break;
	case	',':
		this_token = COMMA;
		break;
	case	'\0':
		this_token = NEWLINE;
		break;
	default:
		begin_token = cp;
		this_token = WORD;
		break;
	}

	/* In all simple token cases, skip over trailing whitespace */
	if (this_token != WORD) {
		if (*cp != '\0')
			cp++;
		if (*cp != '\0')
			cp = &cp[strspn (cp, white)];
		return (this_token);
	}
	/* At this point, this_token is a WORD.
	 * Skip forward to the next token, ignoring whitespace at the end.
	 */
	for ( ;; cp++ ) {
		c = *cp;
		/* return word and advance past white space */
		if (strchr (white, c)) {
			end_token = cp;
			cp = &cp[strspn (cp, white)];
			break;
		}
		/* return word and leave cp where it is */
		if (c == '\0' || strchr (tokens, c)) {
			end_token = cp;
			break;
		}
	}
	size = end_token - begin_token + 1;
	strncpy (lexword, begin_token, size - 1);
	lexword[size - 1] = '\0';
	return (WORD);
}

/*
 *
 *	Return a value that indicates if the synonym data base is 
 *	valid.  A return value of zero (0) indicates that the synonym
 *	data base is invalid.  Any non-zero value (!= 0) indicates that
 *	the synonym data base is valid.
 *
 */

int
mand_syndb_valid()

{
	return(syn_db_is_valid);
}
#endif /*}*/
#endif
