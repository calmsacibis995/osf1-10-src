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
static char	*sccsid = "@(#)$RCSfile: labelcomp.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:28:46 $";
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
 * Copyright (c) 1989 SecureWare, Inc.
 * All Rights Reserved.
 *
 * This program is derived from the l_init.c module of Woodward's CMW
 * label demonstration program.
 */



/*
 * This program compiles the ascii label encodings file into a binary form
 * that can be quickly loaded into memory.  The encodings are first compiled
 * into a contiguous memory block whose layout is illustrated below, and
 * then written to a file with embedded pointers converted to file offsets.
 * The ascii encodings file is scanned twice, the first time to determine the
 * proper size for the memory block, and the second time to perform the
 * compilation.
 *
 * The memory block contains, in order:
 *
 *  Table space:
 *	l_min_classification
 *	l_max_classification
 *	l_classification_protect_as
 *	l_lo_clearance
 *	l_lo_sensitivity_label
 *	l_long_classification
 *	l_short_classification
 *	l_in_compartments
 *	l_in_markings
 *	l_accreditation_range
 *	l_information_label_tables
 *	l_sensitivity_label_tables
 *	l_clearance_tables
 *	l_channel_tables
 *	l_printer_banner_table
 *	the information label word table
 *	the sensitivity label word table
 *	the clearance word table
 *	the channel word table
 *	the printer banner word table
 *	the information label required combinations table
 *	the information label combination constraints table
 *	the sensitivity label required combinations table
 *	the sensitivity label combination constraints table
 *	the clearance required combinations table
 *	the clearance combination constraints table
 *
 *  String Space:
 *	l_hi_compartments
 *	l_hi_markings
 *	l_t_compartments
 *	l_t_markings
 *	l_comps_handled
 *	l_marks_handled
 *	l_0_compartments
 *	l_0_markings
 *	l_version
 *	compartment bits, marking bits, long name, and short name for each
 *	    classification
 *	output name, long name, compartment mask, compartment value, marking
 *	    mask, marking value, and optional short name for each IL word
 *	word pairs for required combinations of IL words
 *	the accreditation range specifications
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include "std_labels.h"
#ifdef MSG
#include "labelcomp_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LABELCOMP,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#define	ASCII_ENCODINGS_FILE	"/etc/policy/macilb/Encodings"
#define	BINARY_ENCODINGS_FILE	"/etc/policy/macilb/Encodings.db"

extern char *calloc(), *malloc();
extern char *strchr(), *strrchr();

/*
 * The macro CONVERT_TO_DECIMAL converts the characters pointed to by its
 * first argument (s) into a decimal number in its second argument (d).
 * It leaves (s) pointing to the first non-decimal-digit character after
 * the number.	If (s) does not originally point to a decimal digit, (d)
 * is zeroed.
 */

#define CONVERT_TO_DECIMAL(s,d) \
    for (d = 0; *(s) >= '0' && *(s) <= '9'; (s)++)\
	d = (10 * d) + (*(s) - '0')

/*
 * The macro TABLES_ALLOCATE(type, variable, size) allocates space within the
 * buffer pointed to by "tables", setting "variable" to the current value of
 * "tables", and incrementing "tables" beyond "size" entries of the specified
 * "type".
 */

#define TABLES_ALLOCATE(type,variable,size) \
    variable = (type *) tables;\
    tables += (size) * sizeof(type)

/*
 * The macro COMPARTMENTS_ALLOCATE(variable) allocates space within the
 * buffer pointed to by "strings", setting "variable" to the current value
 * of "strings", and incrementing "strings" beyond the space required for
 * a COMPARTMENTS bit string.
 */

#define COMPARTMENTS_ALLOCATE(variable) \
    variable = (COMPARTMENTS *) strings;\
    strings += COMPARTMENTS_SIZE

/*
 * The macro MARKINGS_ALLOCATE(variable) allocates space within the
 * buffer pointed to by "strings", setting "variable" to the current
 * value of "strings", and incrementing "strings" beyond the space
 * required for a MARKINGS bit string.
 */

#define MARKINGS_ALLOCATE(variable) \
    variable = (MARKINGS *) strings;\
    strings += MARKINGS_SIZE

/*
 * Keyword lists for calling next_keyword (below).  They are defined here in
 * order of their first usage below.  These lists define keywords that are
 * found in the encodings file to define various aspects of the encodings.
 */

#define LIST_END (char *) 0

static char *version[] = {
    "VERSION=",
    LIST_END
};

static char *classifications[] = {
    "CLASSIFICATIONS:",
    LIST_END
};

static char *class_keywords[] = {
    "VALUE=",
    "NAME=",
    "SNAME=",
    "INITIAL COMPARTMENTS=",
    "INITIAL MARKINGS=",
    LIST_END
};

#define CVALUE 0
#define CNAME 1
#define CSNAME 2
#define INITIAL_COMPARTMENTS 3
#define INITIAL_MARKINGS 4

static char *information_labels[] = {
    "INFORMATION LABELS:",
    LIST_END
};

static char *words[] = {
    "WORDS:",
    LIST_END
};

static char *name[] = {
    "NAME=",
    LIST_END
};

static char *label_keywords[] = {
    "SNAME=",
    "MINCLASS=",
    "OMINCLASS=",
    "MAXCLASS=",
    "COMPARTMENTS=",
    "MARKINGS=",
    "PREFIX=",
    "SUFFIX=",
    "PREFIX",
    "SUFFIX",
    "ACCESS RELATED",
    "FLAGS=",
    LIST_END
};

#define WSNAME 0
#define WMINCLASS 1
#define WOMINCLASS 2
#define WMAXCLASS 3
#define WCOMPARTMENTS 4
#define WMARKINGS 5
#define WNEEDS_PREFIX 6
#define WNEEDS_SUFFIX 7
#define WIS_PREFIX 8
#define WIS_SUFFIX 9
#define WACCESS_RELATED 10
#define WFLAGS 11

static char *required_combinations[] = {
    "REQUIRED COMBINATIONS:",
    LIST_END
};

static char *combination_constraints[] = {
    "COMBINATION CONSTRAINTS:",
    LIST_END
};

static char *sensitivity_labels[] = {
    "SENSITIVITY LABELS:",
    LIST_END
};

static char *clearances[] = {
    "CLEARANCES:",
    LIST_END
};

static char *channels[] = {
    "CHANNELS:",
    LIST_END
};

static char *printer_banners[] = {
    "PRINTER BANNERS:",
    LIST_END
};

static char *accreditation_range[] = {
    "ACCREDITATION RANGE:",
    LIST_END
};

static char *min_clearance[] = {
    "MINIMUM CLEARANCE=",
    "CLASSIFICATION=",
    LIST_END
};

#define MINIMUM_CLEARANCE 0

static char **classification = &min_clearance[1];

static char *ar_types[] = {
    "ALL COMPARTMENT COMBINATIONS VALID EXCEPT:",
    "ALL COMPARTMENT COMBINATIONS VALID",
    "ONLY VALID COMPARTMENT COMBINATIONS:",
    LIST_END
};

/*
 * Note that the index into the above array is also the
 * l_type_accreditation_range defined in std_labels.h
 */

static char *min_sensitivity_label[] = {
    "MINIMUM SENSITIVITY LABEL=",
    LIST_END
};

static char *min_protect_as_classification[] = {
    "MINIMUM PROTECT AS CLASSIFICATION=",
    LIST_END
};

/*
 * Global variables
 */

static unsigned long	    size_tables;
static unsigned long	    size_strings;
static char		    *strings;
static char		    *tables = 0;
static int		    counting = TRUE;
static int		    first_main_entry;
static FILE		    *f;
static char		    buffer[MAX_ENCODINGS_LINE_LENGTH];
static char		    *scan_ptr;
static short		    line_continues;
static int		    index;
static char		    *dp;
static struct l_constraints *end_ptr;
static int		    max_classification_length;
static char		    *sl_buffer = 0;
static char		    *cl_buffer = 0;
static short		    num_information_label_words;
static short		    num_sensitivity_label_words;
static short		    num_clearance_words;
static short		    num_channel_words;
static short		    num_printer_banner_words;

char			    *l_version;
CLASSIFICATION		    *l_min_classification;
CLASSIFICATION		    *l_max_classification;
CLASSIFICATION		    *l_classification_protect_as;
COMPARTMENTS		    *l_hi_compartments;
MARKINGS		    *l_hi_markings;
struct l_sensitivity_label  *l_lo_clearance;
struct l_sensitivity_label  *l_lo_sensitivity_label;
char			    **l_long_classification;
char			    **l_short_classification;
COMPARTMENTS		    **l_in_compartments;
MARKINGS		    **l_in_markings;
struct l_accreditation_range	*l_accreditation_range;
struct l_tables		    *l_information_label_tables;
struct l_tables		    *l_sensitivity_label_tables;
struct l_tables		    *l_clearance_tables;
struct l_tables		    *l_channel_tables;
struct l_tables		    *l_printer_banner_tables;
COMPARTMENTS		    *l_0_compartments;
MARKINGS		    *l_0_markings;
COMPARTMENTS		    *l_t_compartments;
MARKINGS		    *l_t_markings;
COMPARTMENT_MASK	    *l_comps_handled;
MARKING_MASK		    *l_marks_handled;


main(argc, argv)
    int argc;
    char    *argv[];
{
    int		dbfd, opt, cflag = 0;
    char	*dbfilename = NULL, *filename, *cp;
    struct stat fstb, dbstb;
    extern char *optarg;
    extern int	optind, opterr;
#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_LABELCOMP,NL_CAT_LOCALE);
#endif

    opterr = 0;
    while ((opt = getopt(argc, argv, "co:")) != EOF)
	switch (opt) {
	case 'c':
	    cflag++;
	    break;
	case 'o':
	    dbfilename = optarg;
	    break;
	case '?':
	    usage();
	}

    if (optind == argc) {
	filename = ASCII_ENCODINGS_FILE;
	if (dbfilename == (char *) 0)
	    dbfilename = BINARY_ENCODINGS_FILE;
    } else if (optind != argc - 1)
	usage();
    else
	filename = argv[optind];

    if (dbfilename == (char *) 0) {

	/*
	 * Construct the pathname for the compiled encodings file
	 * by appending ".db" to the end of the supplied pathname.
	 * Make sure not to exceed the maximum component name length.
	 */

	dbfilename = malloc(strlen(filename) + 4);
	if (dbfilename == (char *) 0) {
	    fprintf(stderr, MSGSTR(LABELCOMP_1, "Can't allocate memory for database pathname\n"));
	    exit(1);
	}
	strcpy(dbfilename, filename);
	if ((cp = strrchr(dbfilename, '/')) != (char *) 0)
	    ++cp;
	else
	    cp = dbfilename;
	if (strlen(cp) > NAME_MAX - 3)
	    cp[NAME_MAX - 3] = '\0';    /* allow room for suffix */
	strcat(cp, ".db");
    }

    /*
     * Get the last modification time for the encodings source file.
     */

    if (stat(filename, &fstb) != 0) {
	perror(filename);
	exit(1);
    }

    /*
     * If the -c options was specified and if the compiled encodings
     * file exists and is newer than the source file exit without
     * doing anything.
     */

    if (cflag && stat(dbfilename, &dbstb) == 0 &&
	    fstb.st_mtime <= dbstb.st_mtime)
	exit(0);

    if (mand_init_params(0) != 0)
	exit(1);

    if (l_parse_encodings(filename) != 0)
	exit(1);

    opt = 1;

    dbfd = open(dbfilename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dbfd >= 0) {
	opt = l_dbstore(dbfd);
	close(dbfd);
    } else
	perror(dbfilename);

    exit(opt);
}


usage()
{
    fprintf(stderr, MSGSTR(LABELCOMP_2, "usage: labelcomp [-c] [-o dbfile] [encodings]\n"));
    exit(1);
}


#ifdef DEBUG
static char *
mask_t_display(m, n)
    register mask_t *m;
    register int    n;
{
    register char   *buf, *bp;

    buf = bp = (char *) malloc(n * 8 + 1);
    while (--n >= 0) {
	sprintf(bp, "%08x", *m++);
	bp += 8;
    }
    return buf;
}
#endif

/*
 * The internal subroutine error prints an approrpriate error message about a
 * fatal conversion error, then frees any allocated storage, and closes the
 * encodings file.
 */

static void
error(format, v1, v2, v3, v4, v5)
    char    *format, *v1, *v2, *v3, *v4, *v5;
{
    fprintf(stderr, format, v1, v2, v3, v4, v5);

    /*
     * Free the two parsing buffers if they have been allocated,
     * and close the encodings file.
     */

    if (cl_buffer) {
	free(cl_buffer);
	cl_buffer = (char *) 0;
    }
    if (sl_buffer) {
	free(sl_buffer);
	sl_buffer = (char *) 0;
    }

    if (f) {
	fclose(f);
	f = (FILE *) 0;
    }

    /*
     * Free the table space if allocated.
     */

    if (tables) {
	free(l_min_classification);
	tables = (char *) 0;
    }
    l_min_classification = (CLASSIFICATION *) 0;
}



/*
 * The internal subroutine next_keyword finds the next keyword in the input
 * file, given a table of valid keywords.  next_keyword uses the global
 * variable scan_ptr to remember where it should continue looking in the
 * global buffer each time it is called.  If needed to find the next keyword,
 * the next line of the file will be read into the buffer, and the scan_ptr
 * to where to look next will be adjusted.  The index of the keyword found
 * is returned, or a -1 if no keyword is found, or a -2 if EOF is reached.
 * Completely blank lines are ignored, as are comment lines, which must
 * start with a *.
 *
 * If a keyword is found, a pointer to the data following the keyword is also
 * found, and returned by setting the global variable dp (for data ptr) to
 * point to it.	 The data will be in the form of a zero-terminated string.
 * The end of the data is denoted in the encodings file by either a semicolon
 * or the end of a line.  The semicolon is replaced by a zero to denote end
 * of data, as is the newline at the end of the line.
 *
 * If data is found following a keyword that does not end in =, then
 * next_keyword returns as if the keyword was not found.
 *
 * The first time next_keyword is called, scan_ptr to should point to a '\0'
 * in buffer.
 *
 * next_keyword READS GLOBAL variables: f, buffer, scan_ptr, dp.
 * next_keyword WRITES GLOBAL variables: buffer, scan_ptr, dp.
 */

static int
next_keyword(keywords)
    char    *keywords[];
{
    register char   *bp;
    register int    i;
    register int    keyword_length;
    int		    have_blank;
    int		    c;

    bp = scan_ptr;  /* set fast ptr to place to start scan for keyword */

    if (*bp == ' ') /* skip over blank, if any */
	bp++;

    /*
     * If we are at the end of this input line, then another line's worth
     * of input is read, leading blanks are removed, multiple blanks or
     * tabs are replaced with a single blank, and alphabetic characters
     * are forced to upper case.  The \n at the end of the line is
     * replaced with a \0, and bp is left at the start of the line.  If a
     * line longer than MAX_ENCODINGS_LINE_LENGTH characters is found, it
     * is replaced in the buffer by an error message (see below) that will
     * end of getting printed in an error message by the caller of
     * next_keyword.
     */

    while (*bp == '\0') {
	have_blank = TRUE;  /* causes leading blanks to be ignored */
	bp = buffer;

	while (EOF != (c = (char) fgetc(f))) {
	    if (bp == (buffer + MAX_ENCODINGS_LINE_LENGTH)) {
		sprintf(buffer, MSGSTR(LABELCOMP_3, "// Line longer than %d characters //"),
			    MAX_ENCODINGS_LINE_LENGTH);
		scan_ptr = buffer;
		return (-1);
	    }

	    if (c != '\n') {
		if (!have_blank && (c == ' ' || c == '\t')) {
		    have_blank = TRUE;
		    *bp++ = ' ';
		} else if (c != ' ' && c != '\t') {
		    have_blank = FALSE;
		    if (c >= 'a' && c <= 'z')
			*bp++ = c & ~('a' - 'A');
		    else
			*bp++ = c;
		}
	    } else
		break;
	}

	if (c == EOF && bp == buffer)
	    return (-2);
	*bp = '\0';
	bp = buffer;
	if (*bp == '*')
	    *bp = '\0';
    }

    /*
     * Now look for each keyword passed in the buffer.
     */

    scan_ptr = bp;	/* in case an error, return scanptr here */

    for (i = 0; keywords[i]; i++) {
	keyword_length = strlen(keywords[i]);

	if (0 == strncmp(keywords[i], bp, keyword_length)) {
	    bp += keyword_length;   /* skip over keyword */
	    if (*bp == ' ')	/* and blank */
		bp++;
	    /* error if data follows a non = keyword */
	    if (keywords[i][keyword_length-1] != '=' && *bp && *bp != ';')
		return (-1);
	    dp = bp;	/* point to data after keyword */
	    while (*bp && *bp != ';')
		bp++;
	    if (*bp) {
		*bp = '\0';
		scan_ptr = bp + 1;
	    } else
		scan_ptr = bp;
	    return (i); /* return index of keyword */
	}
    }
    return (-1);	/* return keyword not found */
}


/*
 * The internal subroutine parse_bits parses the bit indication strings that
 * follow COMPARTMENTS, MARKINGS, or FLAGS keywords.  The indication strings
 * can have one of the forms:
 *
 *  [~]N
 *  [~]N1-N2
 *
 * or any of the above forms separated by blanks.  N, N1, and N2 are decimal bit
 * numbers, starting with 0 as the leftmost bit.  The optional ~ means that the
 * indicated bit(s) should be OFF as opposed to the default of ON.
 *
 * This subroutine is passed a ptr to the string to parse, the maximum
 * permissable value for a bit to be processed, the routine
 * that actually sets the bits as specified, and pointer to the
 * bit mask and value to be operated on by the passed routine.
 * See below for the routines applicable to COMPARTMENTS, MARKINGS, anf FLAGS.
 *
 * TRUE is returned if everything parsed OK.  FALSE is returned if the string
 * was not in the proper format.
 */

static int
parse_bits(string, max_bit, routine, mask_ptr, value_ptr)
    register char   *string;
    int		    max_bit;
    void	    (*routine)();
    COMPARTMENTS_OR_MARKINGS_OR_MASKS	*mask_ptr;
    COMPARTMENTS_OR_MARKINGS_OR_MASKS	*value_ptr;
{
    short	    bit_on;
    unsigned int    first_bit;
    unsigned int    last_bit;

    if (*string == ' ')
	string++;   /* skip leading blank, if any */

    while (*string) {
	if (*string == '~') {
	    bit_on = FALSE;
	    string++;
	} else
	    bit_on = TRUE;

	/* must be decimal digit */
	if (*string < '0' || *string > '9')
	    return (FALSE);

	CONVERT_TO_DECIMAL(string, first_bit);

	if (first_bit > max_bit)
	    return (FALSE);	/* bit # too large */

	if (*string == '-') {	/* bit range specified? */
	    string++;	    /* skip over the - */
	    /* must be decimal digit */
	    if (*string < '0' || *string > '9')
		return (FALSE);
	    CONVERT_TO_DECIMAL(string, last_bit);
	    if (last_bit > max_bit || last_bit <= first_bit)
		return (FALSE);
	} else
	    last_bit = first_bit;

	for (; first_bit <= last_bit; first_bit++)
	    (*routine)(mask_ptr, value_ptr, first_bit, bit_on);

	if (*string == ' ')
	    string++;	/* skip trailing blank, if any */
    }
    return (TRUE);
}


/*
 * The internal subroutine set_compartment is intended to be called by
 * parse_bits (above).	It sets up the compartment mask and value for the
 * passed bit for the passed bit value.
 */

static void
set_compartment(mask_ptr, value_ptr, bit, bit_on)
    COMPARTMENTS_OR_MARKINGS_OR_MASKS	*mask_ptr;
    COMPARTMENTS_OR_MARKINGS_OR_MASKS	*value_ptr;
    int	    bit;
    short   bit_on;
{
    COMPARTMENT_MASK_BIT_SET(mask_ptr->l_cm, bit);
    if (bit_on)
	COMPARTMENTS_BIT_SET(value_ptr->l_c, bit);
}


/*
 * The internal subroutine set_marking is intended to be called by
 * parse_bits (above).	It sets up the marking mask and value for the
 * passed bit for the passed bit value.
 */

static void
set_marking(mask_ptr, value_ptr, bit, bit_on)
    COMPARTMENTS_OR_MARKINGS_OR_MASKS	*mask_ptr;
    COMPARTMENTS_OR_MARKINGS_OR_MASKS	*value_ptr;
    int	    bit;
    short   bit_on;
{
    MARKING_MASK_BIT_SET(mask_ptr->l_mm, bit);
    if (bit_on)
	MARKINGS_BIT_SET(value_ptr->l_m, bit);
}


/*
 * The internal subroutine set_flags is intended to be called by
 * parse_bits (above).	It sets the flags passed by the value_ptr argument
 * if bit_on is specified.  The mask_ptr argument is ignored, and is included
 * only for compatibility with the other routines called by parse_bits.
 */

static void
set_flags(mask_ptr, value_ptr, bit, bit_on)
    short   *mask_ptr;
    short   *value_ptr;
    int	    bit;
    short   bit_on;
{
    if (bit_on)
	*value_ptr |= 1 << bit;
}


/*
 * The internal subroutine do_words is called to handle the WORDS keywords
 * for each type of label.  It is passed a character string name of the section
 * whose words are being converted, for use in error messages, a pointer to
 * the appropriate count of words handled to update, and a pointer to the
 * appropriate l_tables.  The output_only flag indicates that the words being
 * processed will not be used for input, and that therefore the output name
 * and the long name can be the same, saving space in the tables.
 *
 * do_words is called during both the counting and conversion passes, after
 * the keyword that starts each section has been found by next_keyword.	 The
 * words in each section are handled in the same manner except for the
 * INFORMATION LABELS section, for which the additional ACCESS RELATED keyword
 * is handled.
 *
 * Each NAME keyword starts a new l_words entry.  For the counting pass,
 * do_words counts the number of l_word entries, and updates size_strings
 * and size_tables to account for the strings and tables spaces that will be
 * needed during the conversion pass.
 *
 * For the conversion pass, each keyword is handled in its own unique manner.
 *
 * TRUE is returned if everythings converts OK.	 Otherwise FALSE is returned
 * after an appropriate error message is printed.
 */

#define OUTPUT_ONLY 1	    /* output_only argument value */
#define INPUT_OUTPUT 0	    /* output_only argument value */

static int
do_words(type, count_ptr, l_tables, output_only)
    char	    *type;
    short	    *count_ptr;
    struct l_tables *l_tables;
    short	    output_only;
{
    register int    j;
    int		    keyword;
    int		    well_formed;

    /*
     * Make sure encodings start with WORDS keyword.
     */

    if (0 > next_keyword(words)) {
	error(MSGSTR(LABELCOMP_4, "Can't find %s WORDS specification.\n"));
	error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), type, scan_ptr);
	return (FALSE);
    }

    /*
     * WORDS parse loop initialization.
     */

    if (counting)
	*count_ptr = 0;	    /* initialize count of words in table */
    else {
	index = -1; /* initialize index into IL l_word table */
	first_main_entry = -1;	/* first main entry not found yet */
    }

    /*
     * The NAME keyword must start each WORDS entry.  This loop is for each
     * NAME keyword.  A subordinate loop finds and processes the other
     * keywords associated with a word.
     */

    while (0 == next_keyword(name)) {

	/*
	 * LABEL WORDS NAME processing for counting pass.  Count the
	 * number of label words.  Adjust size_tables to account for
	 * this word entry, and size_strings for the size of the name
	 * string.  If the name string has a / in it, its size must be
	 * counted twice because the output_name for this word will be
	 * different that the long_name.  Account in size_strings for
	 * the space needed to allocate the COMPARTMENTS and MARKINGS
	 * masks and value strings.
	 */

	if (counting) {
	    ++(*count_ptr);
	    size_tables += sizeof(struct l_word);
	    j = strlen(dp) + 1;
	    size_strings += j;

	    /* if string contains a /, count its length twice */
	    if (!output_only && strchr(dp, '/'))
		size_strings += j;

	    L_ALIGN(size_strings);
	    size_strings += 2 * COMPARTMENTS_SIZE + 2 * MARKINGS_SIZE;
	}

	/*
	 * LABEL WORDS NAME processing for conversion pass.
	 */
	else {

	    /*
	     * If this is not the first word about to be processed, and if
	     * the first main (non-prefix/suffix) entry has not yet been
	     * found, check whether the previous entry was a prefix or
	     * suffix, and if not, set first_main_entry to be the index of
	     * the previous word.
	     */

	    if (-1 != index && -1 == first_main_entry &&
		    l_tables->l_words[index].l_w_prefix != L_IS_PREFIX &&
		    l_tables->l_words[index].l_w_suffix != L_IS_SUFFIX)
		first_main_entry = index;

	    /*
	     * Now increment index for word about to be processed, and
	     * initialize those l_words entries that have non-zero default
	     * values.	Then copy the NAME string to the strings buffer,
	     * and set the flag to indicate that we have not yet processed
	     * compartments or markings for this word.
	     */

	    index++;
	    well_formed = FALSE;
	    l_tables->l_words[index].l_w_cm_mask = NULL;
	    l_tables->l_words[index].l_w_mk_mask = NULL;
	    l_tables->l_words[index].l_w_prefix = L_NONE;
	    l_tables->l_words[index].l_w_suffix = L_NONE;
	    l_tables->l_words[index].l_w_min_class = -1;
	    l_tables->l_words[index].l_w_output_min_class = -1;
	    l_tables->l_words[index].l_w_max_class = *l_max_classification + 1;
	    l_tables->l_words[index].l_w_output_name = strings;
	    while (*strings++ = *dp++)
		;

	    /*
	     * Now, if these words are not used for output only, then if
	     * the long name contains any / characters, then it must be
	     * stored again with the slashes turned to blanks as the long
	     * name, so that it can match canonicalized input that has
	     * slashes turned to blanks.  If there is no /, then the long
	     * name is the same as the output name.
	     */

	    dp = l_tables->l_words[index].l_w_output_name;
	    if (!output_only && strchr(dp, '/')) {
		l_tables->l_words[index].l_w_long_name = strings;
		for (; *dp; dp++) {
		    if (*dp == '/')
			*strings++ = ' ';
		    else
			*strings++ = *dp;
		}
		*strings++ = '\0';
	    } else
		l_tables->l_words[index].l_w_long_name = dp;

	    /*
	     * Allocate space for the COMPARTMENTS and MARKINGS masks and
	     * values, and save pointers to them in the word structure.
	     */

	    L_ALIGN(strings);

	    COMPARTMENTS_ALLOCATE(l_tables->l_words[index].l_w_cm_mask);
	    COMPARTMENTS_ALLOCATE(l_tables->l_words[index].l_w_cm_value);
	    MARKINGS_ALLOCATE(l_tables->l_words[index].l_w_mk_mask);
	    MARKINGS_ALLOCATE(l_tables->l_words[index].l_w_mk_value);
	}

	/*
	 * Now that the NAME keyword has been processed, search for
	 * and handle the remaining keywords associated with the
	 * word.
	 */

	while (0 <= (keyword = next_keyword(label_keywords))) {

	    /*
	     * If the counting pass, just count string sizes for the short
	     * name.
	     */

	    if (counting) {
		if (keyword == WSNAME)
		    size_strings += strlen(dp) + 1;
	    }

	    /*
	     * If the conversion pass, each keyword is fully coverted into
	     * the internal tabular format.  Various error checks are
	     * made.  The SNAME is copied to the strings area, and a
	     * pointer to it left in the l_word structure.  The minimum
	     * and output minimum classifications (if any) are looked up
	     * in the classification table, and any required prefix or
	     * suffix is looked up in the l_words table, and must
	     * therefore appear before any references to it.  Compartment
	     * and marking bits are converted into internal form and
	     * stored in the table entry.  If the PREFIX or SUFFIX keyword
	     * appears, this fact is so noted.	Finally, the ACCESS
	     * RELATED keyword is processed only if the l_tables passed is
	     * the l_information_label_tables, as this keyword only
	     * applies in information labels.
	     */

	    else {
		switch (keyword) {
		case WSNAME:

		    if (l_tables->l_words[index].l_w_short_name) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_7, "Duplicate keyword \"%s%s\").\n"),
			    label_keywords[keyword], dp);
			return (FALSE);
		    }

		    l_tables->l_words[index].l_w_short_name = strings;
		    while (*strings++ = *dp++)
			;
		    break;

		case WMINCLASS:

		    if (l_tables->l_words[index].l_w_min_class != -1) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_7, "Duplicate keyword \"%s%s\").\n"),
			    label_keywords[keyword], dp);
			return (FALSE);
		    }

		    for (j = 0; j <= *l_max_classification; j++)
			if (l_long_classification[j]) {
			    if (0 == strcmp(dp, l_long_classification[j]))
				break;
			    if (0 == strcmp(dp, l_short_classification[j]))
				break;
			}
		    if (j <= *l_max_classification)	/* if a match found */
			l_tables->l_words[index].l_w_min_class = j;
		    else {
			error(MSGSTR(LABELCOMP_8, "MINIMUM CLASSIFICATION %s not found.\n"), dp);
			return (FALSE); /* error if not found */
		    }
		    break;

		case WOMINCLASS:

		    if (l_tables->l_words[index].l_w_output_min_class != -1) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_7, "Duplicate keyword \"%s%s\").\n"),
			    label_keywords[keyword], dp);
			return (FALSE);
		    }

		    for (j = 0; j <= *l_max_classification; j++)
			if (l_long_classification[j]) {
			    if (0 == strcmp(dp, l_long_classification[j]))
				break;
			    if (0 == strcmp(dp, l_short_classification[j]))
				break;
			}
		    if (j <= *l_max_classification)	/* if a match found */
			l_tables->l_words[index].l_w_output_min_class = j;
		    else {
			error(MSGSTR(LABELCOMP_9, "OUTPUT MINIMUM CLASSIFICATION %s not found.\n"),
			    dp);
			return (FALSE); /* error if not found */
		    }
		    break;

		case WMAXCLASS:

		    if (l_tables->l_words[index].l_w_max_class !=
						*l_max_classification + 1)
		    {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_7, "Duplicate keyword \"%s%s\").\n"),
			    label_keywords[keyword], dp);
			return (FALSE);
		    }

		    for (j = 0; j <= *l_max_classification; j++)
			if (l_long_classification[j]) {
			    if (0 == strcmp(dp, l_long_classification[j]))
				break;
			    if (0 == strcmp(dp, l_short_classification[j]))
				break;
			}
		    if (j <= *l_max_classification)	/* if a match found */
			l_tables->l_words[index].l_w_max_class = j;
		    else {
			error(MSGSTR(LABELCOMP_10, "MAXIMUM CLASSIFICATION %s not found.\n"), dp);
			return (FALSE); /* error if not found */
		    }
		    break;

		case WCOMPARTMENTS:

		    well_formed = TRUE;

		    if (!COMPARTMENT_MASK_EQUAL(l_tables->l_words[index].l_w_cm_mask,
						l_0_compartments))
		    {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_7, "Duplicate keyword \"%s%s\").\n"),
			    label_keywords[keyword], dp);
			return (FALSE);
		    }

		    if (!parse_bits(dp, mand_max_cat, set_compartment,
				    l_tables->l_words[index].l_w_cm_mask,
				    l_tables->l_words[index].l_w_cm_value))
		    {
			error(MSGSTR(LABELCOMP_11, "Invalid COMPARTMENTS specification: %s.\n"), dp);
			return (FALSE);
		    }
		    COMPARTMENTS_COMBINE(l_hi_compartments,
					 l_tables->l_words[index].l_w_cm_mask);
		    break;

		case WMARKINGS:

		    if (l_tables != l_information_label_tables) {
			error(MSGSTR(LABELCOMP_12, "Keyword MARKINGS does not apply to %s words.\n"),
			    type);
			return (FALSE);
		    }

		    well_formed = TRUE;

		    if (!MARKING_MASK_EQUAL(l_tables->l_words[index].l_w_mk_mask,
					    l_0_compartments))
		    {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_7, "Duplicate keyword \"%s%s\").\n"),
			    label_keywords[keyword], dp);
			return (FALSE);
		    }

		    if (!parse_bits(dp, mand_max_mark, set_marking,
				    l_tables->l_words[index].l_w_mk_mask,
				    l_tables->l_words[index].l_w_mk_value))
		    {
			error(MSGSTR(LABELCOMP_13, "Invalid MARKINGS specification: %s.\n"), dp);
			return (FALSE);
		    }
		    MARKINGS_COMBINE(l_hi_markings,
				     l_tables->l_words[index].l_w_mk_mask);
		    break;

		case WNEEDS_PREFIX:

#ifdef notdef
		    if (l_tables->l_words[index].l_w_suffix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n",
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_14, "Cannot mix PREFIX and SUFFIX keywords for the same word.\n"));
			    return (FALSE);
		    }
#endif
		    if (l_tables->l_words[index].l_w_prefix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_15, "Duplicate keyword \"PREFIX\").\n");
			return (FALSE);
		    }

		    for (j = 0; j < index; j++) {
			if (!strcmp(dp, l_tables->l_words[j].l_w_output_name))
			    break;
			if (l_tables->l_words[j].l_w_short_name &&
			    !strcmp(dp, l_tables->l_words[j].l_w_short_name))
			    break;
		    }
		    if (j < index)	/* if a match found */
			l_tables->l_words[index].l_w_prefix = j;
		    else {
			error(MSGSTR(LABELCOMP_16, "PREFIX %s not found.\n"), dp);
			return (FALSE); /* error if not found */
		    }
		    break;

		case WNEEDS_SUFFIX:

#ifdef notdef
		    if (l_tables->l_words[index].l_w_prefix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n",
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_14, "Cannot mix PREFIX and SUFFIX keywords for the same word.\n"));
			return (FALSE);
		    }
#endif
		    if (l_tables->l_words[index].l_w_suffix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_17, "Duplicate keyword \"SUFFIX\").\n");
			return (FALSE);
		    }

		    for (j = 0; j < index; j++) {
			if (!strcmp(dp, l_tables->l_words[j].l_w_output_name))
			    break;
			if (l_tables->l_words[j].l_w_short_name &&
			    !strcmp(dp, l_tables->l_words[j].l_w_short_name))
			    break;
		    }
		    if (j < index)	/* if a match found */
			l_tables->l_words[index].l_w_suffix = j;
		    else {
			error(MSGSTR(LABELCOMP_18, "SUFFIX %s not found.\n"), dp);
			return (FALSE); /* error if not found */
		    }
		    break;

		case WIS_PREFIX:

		    well_formed = TRUE;

#ifdef notdef
		    if (l_tables->l_words[index].l_w_suffix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n",
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_14, "Cannot mix PREFIX and SUFFIX keywords for the same word.\n"));
			return (FALSE);
		    }
#endif
		    if (l_tables->l_words[index].l_w_prefix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_15, "Duplicate keyword \"PREFIX\").\n");
			return (FALSE);
		    }

		    l_tables->l_words[index].l_w_prefix = L_IS_PREFIX;
		    break;

		case WIS_SUFFIX:

		    well_formed = TRUE;

#ifdef notdef
		    if (l_tables->l_words[index].l_w_prefix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n",
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_14, "Cannot mix PREFIX and SUFFIX keywords for the same word.\n"));
			return (FALSE);
		    }
#endif
		    if (l_tables->l_words[index].l_w_suffix != L_NONE) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_17, "Duplicate keyword \"SUFFIX\").\n");
			return (FALSE);
		    }

		    l_tables->l_words[index].l_w_suffix = L_IS_SUFFIX;
		    break;

		case WACCESS_RELATED:

		    if (l_tables != l_information_label_tables) {
			error(MSGSTR(LABELCOMP_19, "Keyword ACCESS RELATED does not apply to %s words.\n"),
			    type);
			return (FALSE);
		    }

		    if (l_tables->l_words[index].l_w_flags & ACCESS_RELATED) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_20, "Duplicate keyword \"ACCESS RELATED\").\n");
			return (FALSE);
		    }

		    l_tables->l_words[index].l_w_flags |= ACCESS_RELATED;

		    break;

		case WFLAGS:

		    if (l_tables->l_words[index].l_w_flags & ~ACCESS_RELATED) {
			error(MSGSTR(LABELCOMP_6, "In %s WORDS, word \"%s\"):\n"),
			    type, l_tables->l_words[index].l_w_output_name);
			error(MSGSTR(LABELCOMP_21, "Duplicate keyword \"FLAGS\").\n");
			return (FALSE);
		    }

		    if (!parse_bits(dp, 15, set_flags,
			&l_tables->l_words[index].l_w_flags,
			&l_tables->l_words[index].l_w_flags))
		    {
			error(MSGSTR(LABELCOMP_22, "Invalid FLAGS specification: %s.\n"), dp);
			return (FALSE);
		    }

		}
	    }


	}

	/*
	 * Now that we are done with the non-NAME keywords for this
	 * word, we must make sure that COMPARTMENTS or MARKINGS were
	 * specified unless this entry is a PREFIX or SUFFIX itself.
	 * The flag well_formed is set above if COMPARTMENTS or
	 * MARKINGS were specified, or if the word is a PREFIX or a
	 * SUFFIX.
	 */

	if (!counting && !well_formed) {
	    error(MSGSTR(LABELCOMP_23, "%s WORD %s does not have COMPARTMENTS or MARKINGS,\n"),
		type, l_tables->l_words[index].l_w_output_name);
	    error(MSGSTR(LABELCOMP_24, "and is not a PREFIX or SUFFIX.\n"));
	    return (FALSE);
	}
    }

    /*
     * If this was the second pass, set l_num_entries to be the count of
     * words processed.	 Then, if any words were specified, and the first
     * main (non-prefix/suffix) entry has not been found yet, check the
     * last word to see if it is the first main entry.	If not, process an
     * error.  If so, save the index of the first main entry in the table.
     * If there were no words entered, save 0 as the index of the first
     * main entry.
     */

    if (!counting) {
	l_tables->l_num_entries = *count_ptr;	/* save number of words */

	if (*count_ptr != 0)	    /* if at least one word found */ {
	    if (-1 == first_main_entry) {
		if (l_tables->l_words[index].l_w_prefix != L_IS_PREFIX &&
		    l_tables->l_words[index].l_w_suffix != L_IS_SUFFIX)
		    first_main_entry = index;

		else {
		    error(MSGSTR(LABELCOMP_25, "No %s WORDS non-prefix/suffix entries.\n"), type);
		    return (FALSE);
		}
	    }
	    l_tables->l_first_main_entry = first_main_entry;
	} else	/* no words were entered */
	    l_tables->l_first_main_entry = 0;
    }

    return (TRUE);
}


/*
 * The internal subroutine word_index parses the text pointed to by the
 * global scan_ptr by looking it up in the passed l_tables, and returns the
 * index of the word in the table or -1 if the word is not found.  scan_ptr
 * is updated to point after the text looked up.  Note that this routine is
 * called only after the text has been processed by next_keyword, which means
 * that the text is assumed to be already normalized.  The algorithm used
 * here is essentially the same as that used in l_parse.
 */

static int
word_index(l_tables)
    register struct l_tables	*l_tables;
{
    register int    i;
    register char   *s = scan_ptr;	/* fast ptr to place to scan */
    int		    len_matched;
    int		    prefix = L_NONE;
    int		    suffix = L_NONE;
    int		    return_index;	/* the index to return */

    /*
     * This is the start of the main loop to check each
     * remaining part of the string against the word table.
     */

    while (*s != '\0') {

	/*
	 * Now, try to match the next part of the string against each
	 * word in the word table that is visible given the maximum
	 * classification and compartments.
	 */

	for (i = 0; i < l_tables->l_num_entries; i++) {

	    /*
	     * Continue in loop (ignoring this word in table) if we are
	     * not parsing after a prefix and this word requires a
	     * prefix.
	     */

	    if (prefix == L_NONE && l_tables->l_words[i].l_w_prefix >= 0)
		continue;

	    /*
	     * Continue in loop (ignoring this word in table) if we ARE
	     * parsing after a prefix and this word does not require this
	     * prefix.
	     */

	    if (prefix >= 0 && prefix != l_tables->l_words[i].l_w_prefix)
		continue;

	    /*
	     * Continue in loop (ignoring this word in table) if we are
	     * not parsing after a word that requires a suffix and this
	     * word IS a suffix.
	     */

	    if (suffix == L_NONE
		    && l_tables->l_words[i].l_w_suffix == L_IS_SUFFIX)
		continue;

	    /*
	     * Continue in loop (ignoring this word in table) if we ARE
	     * parsing after a word that requires a suffix and this word
	     * is not a suffix or another word that requires the same
	     * suffix.
	     */

	    if (suffix >= 0 && suffix != l_tables->l_words[i].l_w_suffix
		    && suffix != i)
		continue;

	    /*
	     * If this word is not to be ignored, then compare the string
	     * being parsed to this word.
	     */

	    len_matched = strlen(l_tables->l_words[i].l_w_output_name);
	    if (!strncmp(s, l_tables->l_words[i].l_w_output_name, len_matched)
		    && (s[len_matched] == ' ' || s[len_matched] == '\0'))
		break;

	    if (l_tables->l_words[i].l_w_short_name) {
		len_matched = strlen(l_tables->l_words[i].l_w_short_name);
		if (!strncmp(s, l_tables->l_words[i].l_w_short_name,
				len_matched)
			&& (s[len_matched] == ' ' || s[len_matched] == '\0'))
		    break;
	    }
	}

	/*
	 * Find out if string matches word in table.
	 */

	if (i < l_tables->l_num_entries) {
	    s += len_matched;	/* set ptr to rest to parse */
	    if (*s == ' ')
		s++; /* skip over blank, if any */
	    scan_ptr = s;

	    /*
	     * If we are parsing after having found a prefix, then this
	     * entry REQUIRES this prefix, so return the index of this
	     * word.
	     */

	    if (prefix >= 0)
		return (i);

	    /*
	     * If the word matched IS a prefix, we must record that we
	     * have encountered a prefix and continue in the main
	     * loop.
	     */

	    if (l_tables->l_words[i].l_w_prefix == L_IS_PREFIX) {
		if (suffix >= 0)
		    break;  /* no prefix allowed where suffix needed */
		prefix = i; /* save prefix we found */
		continue;   /* back to main loop; nothing more to do for this word */
	    }
	    /*
	     * Now, if the word REQUIRES a prefix, and we didn't have one
	     * or had the wrong one, we must process the error.
	     */

	    if (l_tables->l_words[i].l_w_prefix >= 0)
		break;

	    /*
	     * If we have previously encountered a word that requires a
	     * suffix, see if we have found it now.  If not, process an
	     * error.
	     */

	    if (suffix >= 0) {
		if (i != suffix)    /* if needed suffix not found */
		    break;
	    }

	    /*
	     * If the word matched IS a suffix, then make an error return
	     * because we are not expecting a suffix at this point.
	     */

	    else if (l_tables->l_words[i].l_w_suffix == L_IS_SUFFIX)
		break;

	    /*
	     * If this word REQUIRES a suffix, we should record that fact
	     * and fall through the bottom of the parse loop to check for
	     * the suffix.
	     */

	    else if (l_tables->l_words[i].l_w_suffix >= 0) {
		suffix = l_tables->l_words[i].l_w_suffix;
		return_index = i;
		continue;
	    }

	    /*
	     * If this words neither is nor requires a prefix or suffix,
	     * remember its index to return.
	     */

	    else
		return_index = i;

	    /*
	     * Now that we have handled the special cases for prefixes and
	     * suffixes, this is a valid word, so return its index.
	     */

	    return (return_index);
	} else
	    break; /* error, leave loop if word not found */
    }
    return (-1);
}


/*
 * The internal subroutine check_continuation checks whether the current line in
 * buffer (at the scan_ptr) is ended and has been continued on the next line.
 * If so, it reads a new line by calling next_keyword, and then resets the
 * line_continues (global) flag appropriately for this line.
 */

static void
check_continuation(next_section)
    char    *next_section;
{
    register char   *cp;

    if (*scan_ptr == ' ')
	scan_ptr++; /* skip any blank present */

    if (line_continues && *scan_ptr == '\0') {
	next_keyword(next_section);

	cp = scan_ptr + strlen(scan_ptr) - 1;
	if (*cp == '\\')
	    *cp = '\0';
	else
	    line_continues = FALSE;
    }
}


/*
 * The internal subroutine do_combinations converts the REQUIRED COMBINATIONS
 * and COMBINATION CONSTRAINTS for each type of label.	It is passed a string
 * with the name of the section being handled, the appropriate l_tables, and
 * the keyword list for the NEXT section, which acts as the end of the
 * COMBINATION CONSTRAINTS for this section.
 *
 * TRUE is returned if everything converted OK; otherwise FALSE is returned
 * after an appropriate error message is printed.
 */

static int
do_combinations(type, l_tables, next_section)
    char    *type;
    struct l_tables *l_tables;
    char    *next_section[];
{
    register int    i, j;
    int keyword;
    register char   *cp;
    short   num_words;
    short   word;

    /*
     * The next part of the file must be the REQUIRED COMBINATIONS
     * keyword.	 Return an error if not.  The end of the REQUIRED
     * COMBINATIONS section comes when the COMBINATION CONSTRAINTS
     * keyword is found.
     */

    if (0 > next_keyword(required_combinations)) {
	error(MSGSTR(LABELCOMP_26, "Can't find %s REQUIRED COMBINATIONS specification.\n"), type);
	error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	return (FALSE);
    }

    if (!counting)
	l_tables->l_required_combinations = (struct l_word_pair *) tables;

    while (-1 == (keyword = next_keyword(combination_constraints))) {

	/*
	 * If counting pass, adjust size_tables to account for the
	 * combination on each line.
	 */

	if (counting) {
	    size_tables += sizeof(struct l_word_pair);
	    scan_ptr += strlen(scan_ptr);   /* start next scan w/ next line */
	}

	/*
	 * If conversion pass, make sure input line has exactly two
	 * words on it that can be looked up in the tables.  If so,
	 * store these two words in an l_word_pair structure pointed
	 * to by tables.
	 */

	else {
	    if (-1 == (i = word_index(l_tables))
		    || -1 == (j = word_index(l_tables)) || *scan_ptr)
	    {
		error(MSGSTR(LABELCOMP_27, "Unrecognized %s REQUIRED COMBINATION: %s.\n"),
		    type, buffer);
		return (FALSE);
	    }

	    ((struct l_word_pair *) tables) ->l_word1 = i;
	    ((struct l_word_pair *) tables) ->l_word2 = j;
	    tables += sizeof(struct l_word_pair);
	}
    }

    if (counting)
	size_tables += sizeof(struct l_word_pair);  /* acct for end */
    else {
	((struct l_word_pair *) tables)->l_word1 = L_END;
	tables += sizeof(struct l_word_pair);
    }

    /*
     * At this point we know the COMBINATION CONSTRAINTS keyword has been
     * found if keyword is 0.  If not, print an error message and return.
     * The end of this section comes when the next_section keyword is
     * found.
     */

    if (keyword != 0) {
	error(MSGSTR(LABELCOMP_28, "Can't find %s COMBINATION CONSTRAINTS specification.\n"), type);
	error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	return (FALSE);
    }

    if (!counting)  /* set pointer to constraints table */
	l_tables->l_constraints = (struct l_constraints *) tables;

    while (-1 == (keyword = next_keyword(next_section))) {

	/*
	 * Counting pass processing for each line.  Loop through the
	 * line and its continuation lines (if any), counting the
	 * number of words on the line.	 Once the number of words are
	 * determined, the amount of space needed to store the
	 * l_constraint structure and the (variable number of) word
	 * indexes that follow it is determined and reserved in the
	 * tables.
	 *
	 * A constraint can take any of the forms:
	 *
	 *  words1 ! words2
	 *  words1 & words2
	 *  words1 &
	 *
	 * where words1 and words2 are either a single word or
	 * multiple words separated by a | character.  The first form
	 * means that none of the words in words1 can be combined with
	 * any of the words in words2.	The second form means that
	 * any of the words in words1 can be combined only with words
	 * in words2.  The third form means that any of the words in
	 * words1 cannot be combined with any other words.
	 *
	 * A line containing a constraint can be continued onto the
	 * next line by placing a \ at the end of each line to be
	 * continued.  The \ cannot fall arbitrarily in the line;
	 * it must fall between a word and a delimiter (!, &, or |),
	 * not within a word.
	 */

	if (counting) {
	    num_words = 1;  /* number of words */
	    for (;;) {
		for (cp = scan_ptr; *cp; cp++)
		    switch (*cp) {
		    case '|':
		    case '!':
			num_words++;
			break;

		    case '&':
			if (*++cp == ' ')
			    cp++; /* skip over blank if any */
			if (*cp)
			    num_words++;
			else
			    cp--;
			break;
		    }

		if (*--cp == '\\') {
		    scan_ptr = cp + 1;
		    if (-1 != (keyword = next_keyword(next_section))) {
			error(MSGSTR(LABELCOMP_29, "%s cannot appear in a continuation line.\n"),
			    next_section[0]);
			return (FALSE);
		    }
		    continue;	/* continue to process the new line read */
		}
		break;
	    }

	    L_ALIGN(size_strings);
	    size_strings += num_words * sizeof(short);
	    size_tables += sizeof(struct l_constraints);
	    scan_ptr += strlen(scan_ptr);   /* start next scan w/ next line */
	}

	/*
	 * Counting pass processing for each line.  Loop through the
	 * line and its continuation lines parsing a single
	 * combination constraint.  Once parsed, the combination
	 * constraint is stored as an l_constraint structure which
	 * contains pointers to two lists of word indices.  For
	 * example, given the constraint:
	 *
	 *  A | B ! C | D
	 *
	 * the following would be stored in the tables, pointed to by
	 * l_tables->l_constraints:
	 *
	 *  short l_c_type:		the type of constraint; in this case,
	 *				NOT_WITH
	 *  short *l_c_first_list:	ptr to the index of the first word in
	 *				the first list, in this case the index
	 *				of A
	 *  short *l_c_second_list:	ptr to second of two lists of words;
	 *				in this case, a ptr to the index of C
	 *  short *l_c_end_second_list: ptr beyond end of second list; in this
	 *				case, a ptr after the index of D
	 *
	 * and the following would be stored in the strings area:
	 *
	 *  short: the index of A
	 *  short: the index of B
	 *  short: the index of C
	 *  short: the index of D
	 */

	else {
	    short   first_half = TRUE;
	    short   *lp;

	    line_continues = FALSE;
	    cp = scan_ptr + strlen(scan_ptr) - 1;   /* get ptr to last char */
	    if (*cp == '\\') {
		line_continues = TRUE;
		*cp = '\0'; /* terminate before \ */
	    }

	    L_ALIGN(strings);
	    lp = (short *) strings;
	    ((struct l_constraints *) tables)->l_c_first_list = lp;

	    for (;;) {
		if (*scan_ptr == ' ')
		    scan_ptr ++;
		word = word_index(l_tables);
		if (word == -1) {
		    error(MSGSTR(LABELCOMP_30, "Missing or unrecognized word in %s COMBINATION CONSTRAINTS:\n"),
			type);
		    error("%s.\n", buffer);
		    return (FALSE);
		}

		*lp++ = word;

		check_continuation(next_section);

		switch (*scan_ptr) {
		case '|':

		    scan_ptr ++;
		    check_continuation(next_section);
		    continue;

		case '\0':

		    if (first_half) {
			error(MSGSTR(LABELCOMP_31, "Missing ! or & in %s COMBINATION CONSTRAINTS:\n"),
			    type);
			error("%s.\n", buffer);
			return (FALSE);
		    } else {
			((struct l_constraints *) tables)->l_c_end_second_list
			    = lp;
			strings = (char *) lp;
			break;
		    }

		default:

		    if (*scan_ptr != '&' && *scan_ptr != '!') {
			error(MSGSTR(LABELCOMP_32, "Missing |, ! or & in %s COMBINATION CONSTRAINTS\n"),
			    type);
			error("%s.\n", buffer);
			return (FALSE);
		    }

		    ((struct l_constraints *) tables)->l_c_type =
			(*scan_ptr == '!') ? NOT_WITH : ONLY_WITH;

		    if (first_half) {
			first_half = FALSE;
			((struct l_constraints *) tables)->l_c_second_list = lp;
		    } else {	/* multiple & or ! */
			error(MSGSTR(LABELCOMP_33, "Multiple &s and/or !s in %s COMBINATION CONSTRAINTS\n"),
			    type);
			error("%s.\n", buffer);
			return (FALSE);
		    }

		    if (*scan_ptr++ == '&') {
			if (*scan_ptr == ' ')
			    scan_ptr ++;    /* skip blank, if any */
			if (!line_continues && *scan_ptr == '\0') {
			    ((struct l_constraints *) tables)->l_c_end_second_list = lp;
			    strings = (char *) lp;
			    break;
			}
		    }

		    check_continuation(next_section);

		    continue;
		}
		break;	/* out of for loop if switch broken out of */
	    }
	}


    }
    if (counting)
	size_tables += sizeof(struct l_constraints);	/* acct for end */
    else {
	((struct l_constraints *) tables)->l_c_type = L_END;
	end_ptr = (struct l_constraints *) tables;
	tables += sizeof(struct l_constraints);
    }

    return (TRUE);
}


/*
 * The internal subroutine compute_max_length computes the maximum length
 * of a human-readable label converted using the passed l_tables, and fills
 * this value into the l_max_length in the passed l_tables.
 *
 * The maximum length is the maximum classification length, plus the sum
 * of the lengths of each word in l_tables with their prefix or suffix,
 * plus spaces in between.  compute_max_length performs a worst case length
 * computation, because hierarchies and combination constraints that could
 * shorten the longest label are ignored.
 */

static void
compute_max_length(l_tables)
struct l_tables *l_tables;
{
    register int    i;

    int prefix = L_NONE;    /* indicates output of words after a prefix */
    int suffix = L_NONE;    /* indicates suffix must be output */

    /*
     * First count longest classification and space for \0 at end.
     */

    l_tables->l_max_length = max_classification_length + 1;

    /*
     * Loop through each entry in the l_words table to add the size of
     * this word (plus its prefix or suffix if necessary) to the
     * l_max_length.
     */

    for (i = l_tables->l_first_main_entry; i < l_tables->l_num_entries; i++) {

	/*
	 * Ignore words if they are prefixes or suffixes themselves.
	 */

	if (l_tables->l_words[i].l_w_prefix == L_IS_PREFIX)
	    continue;
	if (l_tables->l_words[i].l_w_suffix == L_IS_SUFFIX)
	    continue;

	/*
	 * If the previous word required a suffix, and this word does
	 * not required the same one, then add the length of the
	 * previous word's suffix to the length.
	 */

	if (suffix >= 0 && suffix != l_tables->l_words[i].l_w_suffix)
	    l_tables->l_max_length += strlen(l_tables->l_words[suffix].l_w_output_name) + 1;

	/*
	 * Now, remember the suffix this word needs (if any) for later
	 * usage (above).
	 */

	suffix = l_tables->l_words[i].l_w_suffix;

	/*
	 * Now, if this word requires a prefix, add the prefix length
	 * unless the prefix was output for a previous word.
	 */

	if (l_tables->l_words[i].l_w_prefix >= 0
		&& prefix != l_tables->l_words[i].l_w_prefix)
	    l_tables->l_max_length +=
		strlen(l_tables->l_words[l_tables->
			l_words[i].l_w_prefix].l_w_output_name) + 1;

	/*
	 * Now, remember the prefix this word needs (if any) for later
	 * usage (above).
	 */

	prefix = l_tables->l_words[i].l_w_prefix;

	/*
	 * Now, add the length of this word itself.
	 */

	l_tables->l_max_length +=
	    strlen(l_tables->l_words[i].l_w_output_name) + 1;
    }

    /*
     * If the previous word required a suffix, then add the length of the
     * previous word's suffix to the length.
     */

    if (suffix >= 0)
	l_tables->l_max_length += strlen(l_tables->l_words[suffix].l_w_output_name) + 1;
}


/*
 * The internal subroutine l_parse_encodings reads the Label Encodings source
 * file and constructs the corresponding tables in memory.  It returns 0 for
 * success and -1 otherwise.
 */

static
l_parse_encodings(filename)
char	*filename;
{
    register int    i;
    int		    keyword;
    CLASSIFICATION  value;
    char	    *long_name;
    char	    *short_name;
    CLASSIFICATION  ar_class;
    char	    *ar;
    CLASSIFICATION  dummy_class;
    COMPARTMENTS    *comps;
    MARKINGS	    *marks;
    int		    num_classifications;
    CLASSIFICATION  min_classification = mand_max_class;
    CLASSIFICATION  max_classification = 0;

    /*
     * Open the file with the human-readable encodings and initialize
     * variables so that next_keyword can read the file properly.
     */

    f = fopen(filename, "r");

    if (f == NULL) {
	error(MSGSTR(LABELCOMP_34, "Encodings file \"%s\") not found.\n"), filename);
	return (-1);
    }

    buffer[0] = '\0';
    scan_ptr = buffer;

    /*
     * Initialize size_tables to account for non-variable size portions:
     * the min and max classifications, the minimum "protect as"
     * classification, the lo clearance and sensitivity label, and the 5
     * l_tables.
     */

    size_tables = 3 * sizeof(CLASSIFICATION)
		+ 2 * sizeof(struct l_sensitivity_label)
		+ 5 * sizeof(struct l_tables);

    /*
     * Initialize size_strings to account for the space for:
     *
     *	l_hi_compartments
     *	l_hi_markings
     *	l_t_compartments
     *	l_t_markings
     *	l_comps_handled
     *	l_marks_handled
     *	l_0_compartments
     *	l_0_markings
     *	l_lo_clearance.l_compartments
     *	l_lo_sensitivity_label.l_compartments
     */

    size_strings = (6 * COMPARTMENTS_SIZE) + (4 * MARKINGS_SIZE);

    /*
     * The following loop is used to scan through the encodings file two
     * times.  The first time, indicated by the flag counting being TRUE,
     * we scan through the encodings, counting size of all character
     * strings in size_strings and counting the size of variable-length
     * tables and increasing size_tables accordingly.  The second time
     * through the values are converted from the file and stored in the
     * areas allocated after the first pass.  Other than parsing the
     * CLASSIFICATIONS section of the encodings, the subroutines above do
     * most of the work.
     */

    for (;;) {

	if (!counting) {

	    /*
	     * At this point we are done with the first pass through
	     * then encodings file and have counted the amount of space
	     * to allocate.  Now allocate the space, producing an error
	     * upon failure.
	     */

	    tables = calloc(size_tables + size_strings, 1);
	    if (tables == NULL) {
		error(MSGSTR(LABELCOMP_35, "Can't allocate %ld bytes for encodings.\n"),
		    size_tables + size_strings);
		fclose(f);
		return (-1);
	    }

	    strings = tables + size_tables;

	    /*
	     * Now that the space is allocated and tables and strings are
	     * set to the respective places to put tables and strings,
	     * allocate the fixed tables using the TABLES_ALLOCATE macro.
	     */

	    TABLES_ALLOCATE(CLASSIFICATION, l_min_classification, 1);
	    TABLES_ALLOCATE(CLASSIFICATION, l_max_classification, 1);
	    TABLES_ALLOCATE(CLASSIFICATION, l_classification_protect_as, 1);
	    TABLES_ALLOCATE(struct l_sensitivity_label, l_lo_clearance, 1);
	    TABLES_ALLOCATE(struct l_sensitivity_label, l_lo_sensitivity_label, 1);
	    TABLES_ALLOCATE(char *, l_long_classification, num_classifications);
	    TABLES_ALLOCATE(char *, l_short_classification, num_classifications);
	    TABLES_ALLOCATE(COMPARTMENTS *, l_in_compartments, num_classifications);
	    TABLES_ALLOCATE(MARKINGS *, l_in_markings, num_classifications);
	    TABLES_ALLOCATE(struct l_accreditation_range , l_accreditation_range,
		num_classifications);
	    TABLES_ALLOCATE(struct l_tables, l_information_label_tables, 1);
	    TABLES_ALLOCATE(struct l_tables, l_sensitivity_label_tables, 1);
	    TABLES_ALLOCATE(struct l_tables, l_clearance_tables, 1);
	    TABLES_ALLOCATE(struct l_tables, l_channel_tables, 1);
	    TABLES_ALLOCATE(struct l_tables, l_printer_banner_tables, 1);
	    TABLES_ALLOCATE(struct l_word, l_information_label_tables->l_words,
		num_information_label_words);
	    TABLES_ALLOCATE(struct l_word, l_sensitivity_label_tables->l_words,
		num_sensitivity_label_words);
	    TABLES_ALLOCATE(struct l_word, l_clearance_tables->l_words,
		num_clearance_words);
	    TABLES_ALLOCATE(struct l_word, l_channel_tables->l_words,
		num_channel_words);
	    TABLES_ALLOCATE(struct l_word, l_printer_banner_tables->l_words,
		num_printer_banner_words);

	    /*
	     * Now allocate the initial portion of the strings, using the
	     * COMPARTMENTS_ALLOCATE and MARKINGS macros.
	     */

	    COMPARTMENTS_ALLOCATE(l_hi_compartments);
	    MARKINGS_ALLOCATE(l_hi_markings);
	    COMPARTMENTS_ALLOCATE(l_t_compartments);
	    MARKINGS_ALLOCATE(l_t_markings);
	    COMPARTMENTS_ALLOCATE(l_comps_handled);
	    MARKINGS_ALLOCATE(l_marks_handled);
	    COMPARTMENTS_ALLOCATE(l_0_compartments);
	    MARKINGS_ALLOCATE(l_0_markings);
	    COMPARTMENTS_ALLOCATE(l_lo_clearance->l_compartments);
	    COMPARTMENTS_ALLOCATE(l_lo_sensitivity_label->l_compartments);

	    /*
	     * Now that the initial portions of the tables and strings
	     * have been allocated, initialize any variables allocated
	     * above whose value is known at this time, before entering
	     * the conversion loop.
	     */

	    *l_min_classification = min_classification;
	    *l_max_classification = max_classification;
	}

	/*
	 * THE VERSION SECTION.
	 *
	 * The beginning of the file must be the version
	 * specification.  Return an error if not.
	 */

	keyword = next_keyword(version);

	if (keyword < 0) {
	    error(MSGSTR(LABELCOMP_36, "Can't find VERSION specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    return (-1);
	}

	/*
	 * Process the VERSION= keyword depending on which pass.  For
	 * the counting pass, just remember the size of the string.
	 * For the conversion pass, copy the string into space
	 * allocated to hold it, and set l_version to point to the
	 * string.
	 */

	if (counting)
	    size_strings += strlen(dp) + 1;
	else {
	    l_version = strings;
	    while (*strings++ = *dp++)
		;
	}

	/*
	 * THE CLASSIFICATIONS SECTION.
	 *
	 * The next part of the file must be the CLASSIFICATIONS
	 * keyword.  Return an error if not.
	 */

	keyword = next_keyword(classifications);

	if (keyword < 0) {
	    error(MSGSTR(LABELCOMP_37, "Can't find CLASSIFICATIONS specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    return (-1);
	}

	/*
	 * Initialization for counting pass, zero the ptr to the
	 * long_name for the current classification (to be able to
	 * test later that a long_name was provided), and the length
	 * of the largest classification name.
	 */

	if (counting) {
	    long_name = (char *) 0;
	    max_classification_length = 0;
	}

	/*
	 * Loop through each CLASSIFICATIONS keyword (for each pass),
	 * computing the max_classification_length, the min_ and max_
	 * classification values, and converting the encodings into
	 * the CLASSIFICATION arrays l_long_classification,
	 * l_short_classification, l_in_compartments, and
	 * l_in_markings.
	 */

	while (0 <= (keyword = next_keyword(class_keywords))) {
	    if (counting) {
		switch (keyword) {
		case CNAME:
		    L_ALIGN(size_strings);
		    size_strings += COMPARTMENTS_SIZE;
		    size_strings += MARKINGS_SIZE;
		case CSNAME:
		    max_classification_length = L_MAX(max_classification_length,
							strlen(dp));
		    size_strings += strlen(dp) + 1;
		    break;

		case CVALUE:
		    CONVERT_TO_DECIMAL(dp, value);
		    if (*dp) {
			error(MSGSTR(LABELCOMP_38, "Invalid characters in CLASSIFICATION value specification: %s.\n"), dp);
			fclose(f);
			return (-1);
		    }
		    if (value < 0 || value > mand_max_class) {
			error(MSGSTR(LABELCOMP_39, "Invalid CLASSIFICATION value: %d\n"), value);
			fclose(f);
			return (-1);
		    }
		    min_classification = L_MIN(min_classification, value);
		    max_classification = L_MAX(max_classification, value);
		    break;
		}
	    } else {
		if (!long_name && keyword != CNAME) {
		    error(MSGSTR(LABELCOMP_40, "The first keyword after CLASSIFICATIONS must be NAME.\n"));
		    fclose(f);
		    return (-1);
		}

		switch (keyword) {
		case CNAME:
		    if (long_name) {
			if (!short_name) {
			    error(MSGSTR(LABELCOMP_41, "Classification %s does not have an SNAME.\n"),
				long_name);
			    fclose(f);
			    return (-1);
			}
			if (value == -1) {
			    error(MSGSTR(LABELCOMP_42, "Classification %s does not have a VALUE.\n"),
				long_name);
			    fclose(f);
			    return (-1);
			}
			l_long_classification[value] = long_name;
			l_short_classification[value] = short_name;
			l_in_compartments[value] = comps;
			l_in_markings[value] = marks;
		    }
		    short_name = (char *) 0;
		    value = -1;

		    L_ALIGN(strings);
		    COMPARTMENTS_ALLOCATE(comps);
		    MARKINGS_ALLOCATE(marks);

		    long_name = strings;
		    while (*strings++ = *dp++)
			;
		    break;

		case CSNAME:
		    short_name = strings;
		    while (*strings++ = *dp++)
			;
		    break;

		case CVALUE:
		    CONVERT_TO_DECIMAL(dp, value);
		    break;

		case INITIAL_COMPARTMENTS:
		    if (!parse_bits(dp, mand_max_cat, set_compartment,
				    l_t_compartments, comps))
		    {
			error(MSGSTR(LABELCOMP_43, "Invalid INITIAL COMPARTMENTS specification: %s.\n"), dp);
			fclose(f);
			return (-1);
		    }
		    break;

		case INITIAL_MARKINGS:
		    if (!parse_bits(dp, mand_max_mark, set_marking,
				    l_t_markings, marks))
		    {
			error(MSGSTR(LABELCOMP_44, "Invalid INITIAL MARKINGS specification: %s.\n"),
			    dp);
			fclose(f);
			return (-1);
		    }
		    break;
		}
	    }
	}

	/*
	 * Now that all CLASSIFICATIONS keywords have been processed,
	 * determine how to adjust size_tables for CLASSIFICATIONS if
	 * counting pass, or store the last classification if the
	 * conversion pass.
	 */

	if (counting) {
	    num_classifications = max_classification + 1;
	    /* convert to amount of space/entry needed */
	    size_tables += num_classifications *
		(2 * sizeof(char *) + sizeof(MARKINGS *) +
		 sizeof(COMPARTMENTS *) + sizeof(struct l_accreditation_range));
	} else {
	    l_long_classification[value] = long_name;
	    l_short_classification[value] = short_name;
	    l_in_compartments[value] = comps;
	    l_in_markings[value] = marks;
	}

	/*
	 * Process an error if no classifications were encoded.
	 */

	if (max_classification_length == 0) {
	    error(MSGSTR(LABELCOMP_45, "Can't find any CLASSIFICATIONS NAME specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    return (-1);
	}

	/*
	 * THE INFORMATION LABELS SECTION.
	 *
	 * The next part of the file must be the INFORMATION LABELS
	 * keyword.  Return an error if not.
	 */

	if (0 > next_keyword(information_labels)) {
	    error(MSGSTR(LABELCOMP_46, "Can't find INFORMATION LABELS specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    return (-1);
	}

	/*
	 * Call do_words to process the INFORMATION LABEL WORDS.
	 */

	if (!do_words("INFORMATION LABELS", &num_information_label_words,
		      l_information_label_tables, INPUT_OUTPUT)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * Call do_combinations to process the INFORMATION LABEL
	 * REQUIRED and INVALID COMBINATIONS.
	 */

	if (!do_combinations("INFORMATION LABELS", l_information_label_tables,
			     sensitivity_labels)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * THE SENSITIVITY LABELS SECTION.
	 *
	 * At this point we know the SENSITIVITY LABELS keyword has
	 * been found.	Call do_words to handle the WORDS keywords.
	 */

	if (!do_words("SENSITIVITY LABELS", &num_sensitivity_label_words,
		      l_sensitivity_label_tables, INPUT_OUTPUT)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * Call do_combinations to process the SENSITIVITY LABEL
	 * REQUIRED and INVALID COMBINATIONS.
	 */

	if (!do_combinations("SENSITIVITY LABELS", l_sensitivity_label_tables,
			     clearances)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * THE CLEARANCES SECTION.
	 *
	 * At this point we know the CLEARANCES keyword has been
	 * found.  Call do_words to handle the WORDS keywords.
	 */

	if (!do_words("CLEARANCES", &num_clearance_words, l_clearance_tables,
		      INPUT_OUTPUT)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * Call do_combinations to process the CLEARANCES REQUIRED and
	 * INVALID COMBINATIONS.
	 */

	if (!do_combinations("CLEARANCES", l_clearance_tables, channels)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * THE CHANNELS SECTION.
	 *
	 * At this point we know the CHANNELS keyword has been found.
	 * Call do_words to handle the WORDS keywords.
	 */

	if (!do_words("CHANNELS", &num_channel_words, l_channel_tables,
		      OUTPUT_ONLY)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * THE PRINTER BANNERS SECTION.
	 *
	 * The next part of the file must be the PRINTER BANNERS
	 * keyword.  Return an error if not.
	 */

	if (0 > next_keyword(printer_banners)) {
	    error(MSGSTR(LABELCOMP_47, "Can't find PRINTER BANNERS specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    return (-1);
	}

	/*
	 * Call do_words to handle the WORDS keywords.
	 */

	if (!do_words("PRINTER BANNERS", &num_printer_banner_words,
		      l_printer_banner_tables, OUTPUT_ONLY)) {
	    fclose(f);
	    return (-1);
	}

	/*
	 * Now that all the l_tables information is read from the
	 * encodings file, compute the l_max_length for each l_tables,
	 * and allocate some buffers based on these lengths that will
	 * be needed later.
	 */

	if (!counting) {
	    compute_max_length(l_information_label_tables);
	    compute_max_length(l_sensitivity_label_tables);
	    compute_max_length(l_clearance_tables);
	    compute_max_length(l_channel_tables);
	    compute_max_length(l_printer_banner_tables);

	    sl_buffer = calloc(l_sensitivity_label_tables->l_max_length, 1);
	    if (sl_buffer == NULL) {
		error(MSGSTR(LABELCOMP_48, "Can't allocate %d bytes for checking sensitivity labels.\n"),
		    l_sensitivity_label_tables->l_max_length);
		fclose(f);
		return (-1);
	    }

	    cl_buffer = calloc(l_clearance_tables->l_max_length, 1);
	    if (cl_buffer == NULL) {
		error(MSGSTR(LABELCOMP_49, "Can't allocate %d bytes for checking minimum clearance.\n"),
		    l_clearance_tables->l_max_length);
		fclose(f);
		free(sl_buffer);
		return (-1);
	    }
	}

	/*
	 * The next part of the file must be the ACCREDITATION RANGE
	 * keyword.  Return an error if not.
	 */

	if (0 > next_keyword(accreditation_range)) {
	    error(MSGSTR(LABELCOMP_50, "Can't find ACCREDITATION RANGE specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    if (!counting) {
		free(sl_buffer);
		free(cl_buffer);
	    }
	    return (-1);
	}

	/*
	 * L_ALIGN size_strings or strings depending on the pass, to
	 * allow for allocation of any COMPARTMENTS found in the
	 * accreditation range specification.
	 */

	if (counting)
	    L_ALIGN(size_strings);
	else
	    L_ALIGN(strings);

	/*
	 * Now at least one CLASSIFICATION keyword must be present in
	 * a meaningful accreditation range.
	 */

	if (0 > next_keyword(classification)) {
	    error(MSGSTR(LABELCOMP_51, "Can't find ACCREDITATION RANGE CLASSIFICATION specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    if (!counting) {
		free(sl_buffer);
		free(cl_buffer);
	    }
	    return (-1);
	}

	/*
	 * The loop below processes the accreditation range
	 * specification for a given classification.  If the end of
	 * the accreditation range specification is another
	 * CLASSIFICATION specification, then the looping continues.
	 * If the end of the accreditation range specification is the
	 * MINIMUM CLEARANCE specification, then the loop is broken
	 * out.
	 */

	for (;;) {
	    if (!counting) {
		for (i = 0; i <= *l_max_classification; i++)
		    if (l_long_classification[i]) {
			if (0 == strcmp(dp, l_long_classification[i]))
			    break;
			if (0 == strcmp(dp, l_short_classification[i]))
			    break;
		    }
		if (i <= *l_max_classification)
		    ar_class = i;
		else {
		    error(MSGSTR(LABELCOMP_52, "ACCREDITATION RANGE CLASSIFICATION %s not found.\n"),
			dp);
		    fclose(f);
		    free(sl_buffer);
		    free(cl_buffer);
		    return (-1);
		}
	    }

	    /*
	     * Now determine which type of accreditation range
	     * specification was made.
	     */

	    if (0 > (keyword = next_keyword(ar_types))) {
		error(MSGSTR(LABELCOMP_53, "ACCREDITATION RANGE specifier: %s is invalid.\n"),
		    scan_ptr);
		fclose(f);
		if (!counting) {
		    free(sl_buffer);
		    free(cl_buffer);
		}
		return (-1);
	    }
	    if (!counting) {
		l_accreditation_range[ar_class].l_ar_type = keyword + 1;
		if (l_accreditation_range[ar_class].l_ar_type != L_ALL_VALID)
		    l_accreditation_range[ar_class].l_ar_start = strings;
	    }

	    /*
	     * Now read the remaining lines of the file until the MINIMUM
	     * CLEARANCE or another CLASSIFICATION keyword is found.
	     */

	    while (-1 == (keyword = next_keyword(min_clearance))) {

		/*
		 * If counting pass, adjust size_strings to account for each
		 * accreditation range compartments specification.
		 */

		if (counting)
		    size_strings += COMPARTMENTS_SIZE;
		else {

		    /*
		     * We should not have gotten any input here if ALL
		     * VALID was specified.
		     */

		    if (l_accreditation_range[ar_class].l_ar_type == L_ALL_VALID) {
			error(MSGSTR(LABELCOMP_54, "No sensitivity labels allowed after "));
			error(MSGSTR(LABELCOMP_55, "ALL COMPARTMENT COMBINATIONS VALID.\n"));
			fclose(f);
			free(sl_buffer);
			free(cl_buffer);
			return (-1);
		    }

		    /*
		     * Now, call l_parse to parse the label, storing the
		     * result in the accreditation range.  If a parsing
		     * error, print message and return.
		     */

		    dummy_class = ar_class;

		    if (-1 != l_parse(scan_ptr, &dummy_class,
				      (COMPARTMENTS *) strings, l_t_markings,
				      l_sensitivity_label_tables,
				      *l_min_classification, l_0_compartments,
				      *l_max_classification, l_hi_compartments))
		    {
			error(MSGSTR(LABELCOMP_56, "Invalid sensitivity label: %s.\n"), scan_ptr);
			fclose(f);
			free(sl_buffer);
			free(cl_buffer);
			return (-1);
		    }

		    l_convert(sl_buffer, dummy_class, l_short_classification,
			      (COMPARTMENTS *) strings, l_t_markings,
			      l_sensitivity_label_tables, NO_PARSE_TABLE,
			      LONG_WORDS, ALL_ENTRIES);
		    if (0 != strcmp(sl_buffer, scan_ptr)) {
			error(MSGSTR(LABELCOMP_57, "Sensitivity label %s not properly formed.\n"),
			    scan_ptr);
			fclose(f);
			free(sl_buffer);
			free(cl_buffer);
			return (-1);
		    }

		    for (ar = l_accreditation_range[ar_class].l_ar_start;
			    ar != strings; ar += COMPARTMENTS_SIZE)
			if (COMPARTMENTS_EQUAL((COMPARTMENTS *) ar,
					       (COMPARTMENTS *) strings))
			{
			    error(MSGSTR(LABELCOMP_58, "Duplicate sensitivity label %s.\n"),
				scan_ptr);
			    fclose(f);
			    free(sl_buffer);
			    free(cl_buffer);
			    return (-1);
			}

		    strings += COMPARTMENTS_SIZE;
		}
		scan_ptr += strlen(scan_ptr);
	    }

	    /*
	     * At this point we are done with the accreditation range for
	     * this particular classifcation.  If we reached EOF, produce
	     * an error message.  Otherwise set the end pointer for this
	     * accreditation range, then continue in loop if another
	     * CLASSIFICATION was found, or break out if MINIMUM CLEARANCE
	     * was found.
	     */

	    if (keyword == -2) {    /* EOF */
		error(MSGSTR(LABELCOMP_59, "Can't find MINIMUM CLEARANCE specification.\n"));
		error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
		fclose(f);
		if (!counting) {
		    free(sl_buffer);
		    free(cl_buffer);
		}
		return (-1);
	    }

	    if (!counting &&
		    l_accreditation_range[ar_class].l_ar_type != L_ALL_VALID)
	    {
		l_accreditation_range[ar_class].l_ar_end = strings;
	    }

	    if (keyword == MINIMUM_CLEARANCE)
		break;
	}

	/*
	 * At this point we know the MINIMUM CLEARANCE specification
	 * has been found.  If counting, just reserve space for 1
	 * struct l_sensitivity_label.	If the second pass, l_parse
	 * the label and store it, and set l_lo_clearance.
	 */

	if (!counting) {

	    /*
	     * Before parsing the minimum clearance, switch the pointer to
	     * the clearance invalid combination table to point to the
	     * L_END marker for a combination table, so that the
	     * combination constraints are NOT enforced on the minimum
	     * clearance.
	     */

	    struct l_constraints *save_constraints;
	    save_constraints = l_clearance_tables->l_constraints;
	    l_clearance_tables->l_constraints = end_ptr;

	    if (-1 != l_parse(dp, &l_lo_clearance->l_classification,
			      l_lo_clearance->l_compartments, l_t_markings,
			      l_clearance_tables, *l_min_classification,
			      l_0_compartments, *l_max_classification,
			      l_hi_compartments))
	    {
		error(MSGSTR(LABELCOMP_60, "Invalid MINIMUM CLEARANCE: %s.\n"), dp);
		fclose(f);
		free(sl_buffer);
		free(cl_buffer);
		return (-1);
	    }

	    l_convert(cl_buffer, l_lo_clearance->l_classification,
		      l_short_classification, l_lo_clearance->l_compartments,
		      l_t_markings, l_clearance_tables, NO_PARSE_TABLE,
		      LONG_WORDS, ALL_ENTRIES);
	    if (0 != strcmp(cl_buffer, dp)) {
		error(MSGSTR(LABELCOMP_61, "MINIMUM CLEARANCE %s not properly formed.\n"), dp);
		fclose(f);
		free(sl_buffer);
		free(cl_buffer);
		return (-1);
	    }

	    /*
	     * Restore the clearance combination constraints table.
	     */

	    l_clearance_tables->l_constraints = save_constraints;
	}

	/*
	 * Next the MINIMUM SENSITIVITY LABEL keyword must follow.
	 * Return an error if not.
	 */

	if (0 > next_keyword(min_sensitivity_label)) {
	    error(MSGSTR(LABELCOMP_62, "Can't find MINIMUM SENSITIVITY LABEL specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    if (!counting) {
		free(sl_buffer);
		free(cl_buffer);
	    }
	    return (-1);
	}

	/*
	 * At this point we know the MINIMUM SENSITIVITY LABEL
	 * specification has been found.  If counting, just reserve
	 * space for 1 struct l_sensitivity_label.  If the second
	 * pass, l_parse the label and store it, and set
	 * l_lo_sensitivity_label.
	 */

	if (!counting) {
	    if (-1 != l_parse(dp, &l_lo_sensitivity_label->l_classification,
			      l_lo_sensitivity_label->l_compartments,
			      l_t_markings, l_sensitivity_label_tables,
			      *l_min_classification, l_0_compartments,
			      *l_max_classification, l_hi_compartments))
	    {
		error(MSGSTR(LABELCOMP_63, "Invalid MINIMUM SENSITIVITY LABEL: %s.\n"), dp);
		fclose(f);
		free(sl_buffer);
		free(cl_buffer);
		return (-1);
	    }

	    l_convert(sl_buffer, l_lo_sensitivity_label->l_classification,
		      l_short_classification,
		      l_lo_sensitivity_label->l_compartments, l_t_markings,
		      l_sensitivity_label_tables, NO_PARSE_TABLE, LONG_WORDS,
		      ALL_ENTRIES);
	    if (0 != strcmp(sl_buffer, dp)) {
		error(MSGSTR(LABELCOMP_64, "MINIMUM SENSITIVITY LABEL %s not properly formed.\n"), dp);
		fclose(f);
		free(sl_buffer);
		free(cl_buffer);
		return (-1);
	    }
	    if (l_lo_clearance->l_classification < l_lo_sensitivity_label->l_classification
		    || !COMPARTMENTS_DOMINATE(l_lo_clearance->l_compartments,
				l_lo_sensitivity_label->l_compartments))
	    {
		error(MSGSTR(LABELCOMP_65, "MINIMUM SENSITIVITY LABEL must be dominated by minimum clearance.\n"));
		fclose(f);
		free(sl_buffer);
		free(cl_buffer);
		return (-1);
	    }
	}

	/*
	 * Next the MINIMUM PROTECT AS CLASSIFICATION keyword must
	 * follow.  Return an error if not.
	 */

	if (0 > next_keyword(min_protect_as_classification)) {
	    error(MSGSTR(LABELCOMP_66, "Can't find MINIMUM PROTECT AS CLASSIFICATION specification.\n"));
	    error(MSGSTR(LABELCOMP_5, "Found instead: %s\n"), scan_ptr);
	    fclose(f);
	    if (!counting) {
		free(sl_buffer);
		free(cl_buffer);
	    }
	    return (-1);
	}

	/*
	 * At this point we know the MINIMUM PROTECT AS CLASSIFICATION
	 * specification has been found.  If counting, just reserve
	 * space for 1 CLASSIFICATION.	If the second pass, l_parse
	 * the label and process an error if it does not parse or
	 * includes compartments or markings.  Otherwise, store the
	 * classification parsed.
	 */

	if (!counting) {
	    for (i = 0; i <= *l_max_classification; i++)
		if (l_long_classification[i]) {
		    if (0 == strcmp(dp, l_long_classification[i]))
			break;
		    if (0 == strcmp(dp, l_short_classification[i]))
			break;
		}

	    if (i <= *l_max_classification) {
		if (i > l_lo_clearance->l_classification) {
		    error(MSGSTR(LABELCOMP_67, "MINIMUM PROTECT AS CLASSIFICATION %s exceeds\n"), dp);
		    error(MSGSTR(LABELCOMP_68, "classification in MINIMUM CLEARANCE.\n"));
		    fclose(f);
		    free(sl_buffer);
		    free(cl_buffer);
		    return (-1);
		} else
		    *l_classification_protect_as = i;
	    } else {
		error(MSGSTR(LABELCOMP_69, "Invalid MINIMUM PROTECT AS CLASSIFICATION: %s.\n"), dp);
		fclose(f);
		free(sl_buffer);
		free(cl_buffer);
		return (-1);
	    }
	}

	/*
	 * At this point we should be at end of file.  If not, print
	 * an error message and return.
	 */

	if (EOF != fgetc(f)) {
	    error(MSGSTR(LABELCOMP_70, "End of file not found after "));
	    error(MSGSTR(LABELCOMP_71, "MINIMUM PROTECT AS CLASSIFICATION specification.\n"));
	    fclose(f);
	    free(sl_buffer);
	    free(cl_buffer);
	    return (-1);
	}

	if (counting) {

	    counting = FALSE;

	    /*
	     * Now reset the seek ptr to the start of the encodings file,
	     * so that the second (conversion) pass can re-read the file.
	     */

	    fseek(f, 0L, 0);
	    buffer[0] = '\0';
	    scan_ptr = buffer;
	} else
	    break;
    }

    free(cl_buffer);
    free(sl_buffer);
    fclose(f);

    return (0);
}


/*
 * l_dbstore - write the parsed encodings to the specified file descriptor
 */

static
l_dbstore(fd)
    int fd;
{
    struct l_dbf_header h;
    register long	base = (long) l_min_classification;
    register int	i;
    int			status = 0;

#define Adjust(ptr,incr)    *(long *)&(ptr) += (long)(incr)
#define AdjustWords(table,incr) {\
    register struct l_word  *wp = table->l_words;\
    for (i = table->l_num_entries; --i >= 0; ) {\
	Adjust(wp[i].l_w_output_name, incr);\
	Adjust(wp[i].l_w_long_name, incr);\
	if (wp[i].l_w_short_name)\
	    Adjust(wp[i].l_w_short_name, incr);\
	Adjust(wp[i].l_w_cm_mask, incr);\
	Adjust(wp[i].l_w_cm_value, incr);\
	Adjust(wp[i].l_w_mk_mask, incr);\
	Adjust(wp[i].l_w_mk_value, incr);\
    }}
#define AdjustConstraints(table,incr) {\
    register struct l_constraints   *cp;\
    for (cp = table->l_constraints; cp->l_c_type != L_END; ++cp) {\
	Adjust(cp->l_c_first_list, incr);\
	Adjust(cp->l_c_second_list, incr);\
	Adjust(cp->l_c_end_second_list, incr);\
    }}

    /*
     * Adjust pointers embedded in tables so that when
     * written to the file they appear as byte offsets
     */

    base = -base;

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

    AdjustWords(l_information_label_tables, base);
    Adjust(l_information_label_tables->l_words, base);
    Adjust(l_information_label_tables->l_required_combinations, base);
    AdjustConstraints(l_information_label_tables, base);
    Adjust(l_information_label_tables->l_constraints, base);

    AdjustWords(l_sensitivity_label_tables, base);
    Adjust(l_sensitivity_label_tables->l_words, base);
    Adjust(l_sensitivity_label_tables->l_required_combinations, base);
    AdjustConstraints(l_sensitivity_label_tables, base);
    Adjust(l_sensitivity_label_tables->l_constraints, base);

    AdjustWords(l_clearance_tables, base);
    Adjust(l_clearance_tables->l_words, base);
    Adjust(l_clearance_tables->l_required_combinations, base);
    AdjustConstraints(l_clearance_tables, base);
    Adjust(l_clearance_tables->l_constraints, base);

    AdjustWords(l_channel_tables, base);
    Adjust(l_channel_tables->l_words, base);

    AdjustWords(l_printer_banner_tables, base);
    Adjust(l_printer_banner_tables->l_words, base);

    /*
     * Set up the file header and write everything out
     */
    h.total_space = size_tables + size_strings;
    h.table_space = size_tables;

    if (write(fd, &h, sizeof h) != sizeof h) {
	status = -1;
	perror(MSGSTR(LABELCOMP_72, "header write failed"));
	fprintf(stderr, MSGSTR(LABELCOMP_73, "addr %x, length %d\n"), &h, sizeof h);
    }
    else if (write(fd, l_min_classification, h.total_space) != h.total_space) {
	status = -1;
	perror(MSGSTR(LABELCOMP_74, "data write failed"));
	fprintf(stderr, MSGSTR(LABELCOMP_73, "addr %x, length %d\n"), base, h.total_space);
    }


    /*
     * Now restore all the pointers to their original values
     */

    base = -base;

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

    return status;

#undef Adjust
#undef AdjustWords
#undef AdjustConstraints

}
