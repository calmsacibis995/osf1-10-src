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
static char	*sccsid = "@(#)$RCSfile: ostd_labels.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:39 $";
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


/* This is  a  ripoff from the std_labels.c from lib/libsecurity.
   We use this file to save modifying too much code in the screens XSL.c
   routine. We only load this in if we have the SEC_MAC_OB, otherwise
   the regular code from the lib/libsecuriy library is picked up  */

#include <sys/secdefines.h>

#if SEC_BASE
#if SEC_MAC_OB
	/* Only use this if mac_ob becuase Encodings file libsecurity.a
         * defines everything in here */
#include <sys/types.h>
#include <sys/security.h>
#include <mandatory.h>
#include "XMain.h"
#define MARKWORDS CATWORDS

static char *l_ID = "CMW Labels Release 1.0; 5/30/89: std_labels.c";

/*
 * The subroutines contained herein use table-driven specifications of
 * label semantics to perform conversion of information labels, sensitivity 
 * labels, and clearances between human-readable and internal bit encoding
 * formats, and to produce other labeling information needed for labeling
 * printed output.
 *
 * This file contains both external subroutines and internal subroutines used by
 * the external subroutines.  All external (global) names used by these routines
 * begin with "l_" to hopefully distinguish them from other external names
 * with which they may be compiled.  Although long (greater than 6 character)
 * names are used throughout, all external names are unique in their first 6
 * characters.
 *
 * A companion subroutine to those in this file is l_init, which appears in the
 * file l_init.c.  l_init reads a human-readable representation of the label encoding
 * tables from a file and converts them into the internal format needed by the other
 * subroutines.
 *
 * The external subroutines contained herein are l_parse, l_convert,
 * l_b_parse, l_b_convert, and l_in_accreditation_range.
 *
 * l_parse takes a human-readable representation of a single label (information,
 * sensitivity, or clearance) and converts it into the internal format.
 *
 * l_convert takes the internal representation of a single label and converts it
 * into its canonicalized human-readable form.
 *
 * l_b_parse uses l_parse to convert a banner than consists of a combination of
 * information and sensitivity labels in the standard human-readable format of:
 *
 *      INFORMATION LABEL [SENSITIVITY LABEL]
 *
 * into the internal representation of the labels.
 *
 * l_b_convert uses l_convert to convert the internal representation of an
 * information and sensitivity label into the human-readable banner shown above.
 *
 * l_in_accreditation_range determines whether or not a sensitivity label is in
 * the system's accreditation range.
 *
 * A more complete description of the usage of each subroutine appears before
 * the subroutine below.  The supporting internal subroutines appear below
 * before the external ones that use them.
 *
 * All of these routines require that the encodings set by l_init be
 * initialized before they can operate.  Therefore, each of these routines
 * calls the external subroutine l_encodings_initialized, whose job it is to
 * ensure that the variables have been initialized.  If l_encodings_initialized
 * returns FALSE, then these routines return without functioning, with an
 * apporpriate error return.
 *
 * All of these subroutines are designed to operate on compartment and marking
 * bits strings that can be defined to be any length.  The definitions of how
 * compartments and markings are stored and operated on appear in the file
 * std_labels.h.  It is possible to change the number of compartment or marking
 * bits that these subroutines support by changing these definitions and 
 * recompiling these subroutines.  Furthermore, it is possible to change the
 * definitions in std_labels.h such that the size of compartments are markings
 * are taken at execution time from global variables.  This is possible because
 * ALL operations on compartments and markings are done via these definitions,
 * and because these subroutines deal only with pointers to these values.
 *
 * std_labels.h also contains the declarations of a number of external variables
 * that represent the conversion tables and system parameters read from the
 * encodings file by l_init.  These tables allow the same subroutines below
 * to work on either information, sensitivity, or clearance labels depending
 * on which l_tables are passed to the subroutines.
 *
 * These routines pass the internal form of classifications, compartments, and
 * markings as separate arguments rather than assuming the are combined in any
 * particular structure.  Furthermore, compartments and markings are always passed
 * with pointers (by reference) rather than by value.
 *
 * Note that these subroutines contain a couple of features to support the
 * construction of a "multiple choice" graphical interface for changing labels.
 * These features are noted below in the description of each subroutine.
 */

#include "ostd_labels.h"

char *Calloc();
int l_encodings_initialized();  /* external subroutine that determines whether
                                   encodings have been initialized */

/*
 * The internal subroutine make_parse_table fills in a parse table
 * based on the passed classification, compartments, markings, and label
 * tables.  A parse table is a character array with each character
 * corresponding to an entry in the l_words table.  The character at index
 * i of the parse table is TRUE iff the word represented by the ith entry
 * in the l_words table is represented by the classification, compartments,
 * and markings.  The caller must assure that the passed parse table contains
 * l_tables->l_num_entries entries (and therefore correlates in size with the
 * passed l_words table in l_tables).  The flags argument specifies which l_word
 * entries are to be considered when scanning the l_word table.  If flags is
 * ALL_ENTRIES, all entries are considered.  Otherwise, only those entries whose
 * l_w_flags have all bits in flags on are considered.
 */

static void
make_parse_table (parse_table, class, comps_ptr, marks_ptr, l_tables, flags)
register char *parse_table;         /* the parse table to be filled in */
CLASSIFICATION class;               /* the label classification */
COMPARTMENTS *comps_ptr;            /* the label compartments */
MARKINGS *marks_ptr;                /* the label markings */
register struct l_tables *l_tables; /* the encoding tables to use */
short flags;                        /* l_w_flags of l_word entries to use */
{
    register int i;                 /* loop index for searching l_words */

/*
 * Initialize set of compartments and markings we have already handled by producing
 * parse table entries that cover them.
 */
 
    COMPARTMENT_MASK_ZERO (l_comps_handled);
    MARKING_MASK_ZERO (l_marks_handled);

/*
 * Loop through each word in the word table to determine whether this word is
 * indicated by the passed compartments and markings.  If so, put the word
 * in the parse table only if all the compartments indicated by the word have
 * not been already handled or not all the markings indicated have been
 * already handled.  Entries whose l_w_flags components do not match the passed
 * flags are ignored completely.
 */

    for (i=l_tables->l_first_main_entry;i<l_tables->l_num_entries;i++)
    {
        if ((l_tables->l_words[i].l_w_flags & flags) != flags) continue;
        
        if (COMPARTMENTS_IN (comps_ptr,
                             l_tables->l_words[i].l_w_cm_mask,
                             l_tables->l_words[i].l_w_cm_value))
            if (MARKINGS_IN (marks_ptr,
                             l_tables->l_words[i].l_w_mk_mask,
                             l_tables->l_words[i].l_w_mk_value))
                if (!COMPARTMENT_MASK_IN (l_tables->l_words[i].l_w_cm_mask, l_comps_handled)
                 || !MARKING_MASK_IN (l_tables->l_words[i].l_w_mk_mask, l_marks_handled))

/* 
 * We now know this word is indicated by the compartments and markings passed.
 * However, the word should only be output if it is appropriate at the label's
 * minimum output classification.  The best example of a word that would
 * have a minimum output classification specified is a release marking:
 * UNCLASSIFIED data is inherently releasable to all countries, but the
 * release markings are--by convention--not output with unclassified
 * labels, and would therefore have an output_min_class greater than
 * UNCLASSIFIED.
 */
 
                    if (class >= l_tables->l_words[i].l_w_output_min_class)
                    {

/*
 * Since this word is to be handled, mark its index TRUE in the parse table, and
 * record that we have handled its compartments and markings.
 */
 
                        parse_table[i] = TRUE;
                        COMPARTMENT_MASK_COMBINE (l_comps_handled,
                                                  l_tables->l_words[i].l_w_cm_mask);
                        MARKING_MASK_COMBINE (l_marks_handled,
                                              l_tables->l_words[i].l_w_mk_mask);
                        continue;   /* in loop */
                    }
                    
        parse_table[i] = FALSE; /* if any if above not satisfied */
    }
}
 
/*
 * The subroutine turnoff_word, called by l_parse and turnon_word (below),
 * turns off the indicated word in the passed  parse_table, unless the word
 * was required by some other word that is turned on in the parse_table.
 * TRUE is returned if the word was turned off, else FALSE is returned.  Also,
 * if the word is turned off, the words_max_class, which is the maximum class
 * allowed by the words that are on, is adjusted to account for the removal
 * of the word; therefore, it may be raised if the word removed was the reason
 * for its being lower than the max_class.
 */

static int
turnoff_word (l_tables, parse_table, index, words_max_class_ptr, max_class)
struct l_tables *l_tables;  /* the encoding tables */
char *parse_table;          /* the parse table to change */
register int index;         /* the parse table index to turn off */
CLASSIFICATION *words_max_class_ptr;    /* ptr to max class specified by words on */
CLASSIFICATION max_class;   /* the maximum classification whose existence to acknowledge */
{
    register struct l_word_pair *wp;
    register int i;
    
    if (parse_table[index])     /* if this word is on... */
    {
        for (wp=l_tables->l_required_combinations; wp->l_word1 != L_END; wp++)
            if (index == wp->l_word2        /* if word to turn off was required... */
             && parse_table[wp->l_word1])   /* and the word that required it is on */
                return (FALSE);             /* then don't turn off this word */

        parse_table[index] = FALSE;
        
/*
 * If the word just turned off had a max_class equal to the words_max_class, then
 * the words_max_class must be recomputed, because it might be raised as a result
 * of turning off this word.
 */

        if (l_tables->l_words[index].l_w_max_class == *words_max_class_ptr)
        {
            *words_max_class_ptr = max_class;   /* initial value if no words on */
            for (i=l_tables->l_first_main_entry; i<l_tables->l_num_entries; i++)
                if (parse_table[i])
                    *words_max_class_ptr = L_MIN (*words_max_class_ptr,
                                                  l_tables->l_words[i].l_w_max_class);
        }
    }

    return (TRUE);
}

/*
 * The subroutine turnon_word, called by l_parse (below) and itself (recursively),
 * determines whether the indicated word can be turned on in the passed parse
 * table, and if so, turns it on.  
 *
 * If the indicated word IS turned on, any other words required by this word
 * are also turned on, and the classification is raised if required for the
 * turned on word.  TRUE is returned if the word was turned on. FALSE is returned
 * if the word could not be turned on.
 */
 
static int
turnon_word (l_tables, parse_table, index, class,
             words_max_class_ptr, max_class, max_comps_ptr)
struct l_tables *l_tables;      /* the encoding tables */
char *parse_table;              /* the parse table to modify */
register int index;             /* the parse table index to turn on */
CLASSIFICATION class;           /* the current classification for this word */
CLASSIFICATION *words_max_class_ptr;    /* ptr to max class specified by words on */
CLASSIFICATION max_class;   /* the maximum classification whose existence to acknowledge */
COMPARTMENTS *max_comps_ptr;    /* the maximum allowable compartments */
{
    register struct l_word_pair *wp;
    register int i;
    struct l_constraints *cp;
    short *lp;
    short *invalid_list;
    short *end_invalid_list;
    short dont_turn_on;

/*
 * First check whether the word can be turned on given its maximum and minimum
 * classifications.  It cannot be turned on if its max_class is below the current
 * class.  It also cannot be turned on if its min_class is above the words_max_class.
 */

    if (class > l_tables->l_words[index].l_w_max_class
     || *words_max_class_ptr < l_tables->l_words[index].l_w_min_class)
        return (FALSE);
    
/*
 * Next check the list of constraints to see whether any constraint prevents this
 * word from being turned on.  Scan the list of constraints, processing each entry
 * depending on its type: NOT_WITH or ONLY_WITH.
 */
 
    for (cp = l_tables->l_constraints; cp->l_c_type != L_END; ++cp)
    {

/*
 * Processing for a constraint of type ONLY_WITH.  For this type, any words in the
 * first list can appear ONLY with words from the second list.
 */
 
        if (cp->l_c_type==ONLY_WITH)
        {
        
/*
 * First check whether the word to turn on is in the first list.  If so, the
 * second list contains the ONLY other words that can be on, so update the
 * parse_table with the only valid words to be on, then scan the parse_table
 * to see if any other words are on.  If so, return FALSE.
 */
 
#define L_OK 4

            for (lp = cp->l_c_first_list; lp != cp->l_c_second_list; lp++)
             if (index==*lp)
            {
                for (lp = cp->l_c_second_list; lp < cp->l_c_end_second_list; lp++)
                    parse_table[*lp] |= L_OK;   /* mark word OK to be on */
                dont_turn_on = FALSE;
                for (i=l_tables->l_first_main_entry; i<l_tables->l_num_entries; i++)
                {
                    if (parse_table[i]&L_OK)
                        parse_table[i] &= ~L_OK;                        
                    else if (parse_table[i])    /* on but not L_OK */
                        dont_turn_on = TRUE;
                }
                if (dont_turn_on) return (FALSE);
                break;
            }

/*
 * Now check whether any word in the first list is on.  If so, then the word to
 * turn on must be in the second list or return FALSE.
 */
 
            for (lp = cp->l_c_first_list; lp != cp->l_c_second_list; lp++)
             if (parse_table[*lp])
            {
                for (lp = cp->l_c_second_list; lp < cp->l_c_end_second_list; lp++)
                    if (index==*lp) break;  /* all is well */
                if (lp==cp->l_c_end_second_list)    /* if word to turn on not found */
                    return (FALSE);
                break;
            }
        }

/*
 * If the constraint type is NOT_WITH, the two lists specify words that cannot
 * be combined (none of the first list can be combined with any of the second).
 * Thus, the word we are trying to turn on cannot be turned on if it is part of
 * such an invalid combination, the other half of which: 1) is already turned on,
 * or 2) which will be forced on by the max_comps (i.e. an inverse compartment).
 */

        else        /* type is NOT_WITH */
        {
            invalid_list = 0;
            for (lp = cp->l_c_first_list; lp != cp->l_c_second_list; lp++)
             if (index==*lp)
            {
                invalid_list = cp->l_c_second_list;
                end_invalid_list = cp->l_c_end_second_list;
                break;
            }
    
            if (!invalid_list) for (lp = cp->l_c_second_list;
             lp != cp->l_c_end_second_list;
             lp++) if (index==*lp)
            {
                invalid_list = cp->l_c_first_list;
                end_invalid_list = cp->l_c_second_list;
                break;
            }
                
            if (invalid_list) for (lp=invalid_list; lp!=end_invalid_list; lp++)
            {
                if (parse_table[*lp])
                    return (FALSE);     /* don't turn on word */
                
                if (!COMPARTMENT_MASK_EQUAL (l_tables->l_words[*lp].l_w_cm_mask,
                                             l_0_compartments)) /* if this is a compartment word */
                    if (COMPARTMENTS_EQUAL (l_tables->l_words[*lp].l_w_cm_value,
                                            l_0_compartments))  /* for an inverse compartment */
                        if (COMPARTMENTS_IN (max_comps_ptr, 
                                             l_tables->l_words[*lp].l_w_cm_mask,
                                             l_tables->l_words[*lp].l_w_cm_value))
                                                /* and indicated by the max */
                            return (FALSE); /* don't turn on this word */
            }
        }
    }
    
/*
 * Next check each word already on in label to see if its compartments and
 * markings dominate those of the word we have been asked to turn on.  If
 * such a word is found, just return without turning on the word.    
 */
 
    for (i=l_tables->l_first_main_entry; i<l_tables->l_num_entries; i++)
     if (i!=index && parse_table[i])
    {
        if (COMPARTMENT_MASK_DOMINATE (l_tables->l_words[i].l_w_cm_mask,
                                       l_tables->l_words[index].l_w_cm_mask))
            if (COMPARTMENTS_DOMINATE (l_tables->l_words[i].l_w_cm_value,
                                       l_tables->l_words[index].l_w_cm_value))
                if (MARKING_MASK_DOMINATE (l_tables->l_words[i].l_w_mk_mask,
                                           l_tables->l_words[index].l_w_mk_mask))
                    if (MARKINGS_DOMINATE (l_tables->l_words[i].l_w_mk_value,
                                           l_tables->l_words[index].l_w_mk_value))
                        return (FALSE);

/*
 * Else if this word we are turning on dominates another already on, turn off
 * the other word.  If the other word cannot be turned off, then return FALSE,
 * indicating that this word cannot be turned on.
 */
 
        if (COMPARTMENT_MASK_DOMINATE (l_tables->l_words[index].l_w_cm_mask,
                                       l_tables->l_words[i].l_w_cm_mask))
            if (COMPARTMENTS_DOMINATE (l_tables->l_words[index].l_w_cm_value,
                                       l_tables->l_words[i].l_w_cm_value))
                if (MARKING_MASK_DOMINATE (l_tables->l_words[index].l_w_mk_mask,
                                           l_tables->l_words[i].l_w_mk_mask))
                    if (MARKINGS_DOMINATE (l_tables->l_words[index].l_w_mk_value,
                                           l_tables->l_words[i].l_w_mk_value))
                        if (!turnoff_word (l_tables, parse_table, i,
                                           words_max_class_ptr, max_class))
                            return (FALSE);
    }          
                               
/*
 * Now try to turn on any words required by this word.  If this word requires 
 * another and the other word is not visible at the maximum class and comps, then
 * this word cannot be turned on.  If any required word visible at the maximum
 * class and comps cannot be turned on, then return FALSE.  In turning on the
 * required words, remember in the parse table whether they were FORCED on or
 * already on.
 */

#define FORCED 2        /* flag in parse table sez word was FORCED on by another word */

    for (wp=l_tables->l_required_combinations; wp->l_word1 != L_END; wp++)
    {
        if (index==wp->l_word1) /* if this word requires another */
        {
            if (!parse_table[wp->l_word2])  /* if needed word not already on */
            {
                if (turnon_word (l_tables, parse_table, wp->l_word2, class,
                                 words_max_class_ptr, max_class, max_comps_ptr)) /* if word can be turned on */
                    parse_table[wp->l_word2] |= FORCED; /* mark it as FORCED */
                    

/*
 * If some required word cannot be turned on, then any other required words that were
 * FORCED on before this word was attempted must be turned back off.  Then return FALSE.
 */
 
                else
                {
                    while ((--wp) >= l_tables->l_required_combinations)
                        if (index==wp->l_word1 && parse_table[wp->l_word2] & FORCED)
                        {
                            turnoff_word (l_tables, parse_table, wp->l_word2,
                                          words_max_class_ptr, max_class);
                            parse_table[wp->l_word2] &= ~FORCED; /* clean up parse table */
                        }
                    return (FALSE);
                }
            }
        }
    }
    
/*
 * Now go back and remove any FORCED flags left on in the parse table.
 */
 
    for (wp=l_tables->l_required_combinations; wp->l_word1 != L_END; wp++)
        if (index==wp->l_word1) /* if this word requires another */
            parse_table[wp->l_word2] = TRUE;    /* remove FORCED if present */

            
/*
 * We now know that its legal to turn on the word, so turn it on in the
 * parse_table, and adjust the maximum classification allowed by for this
 * word (lower the words_max_class if this word has a lower max class than
 * the other words).
 */
    
    parse_table[index] = TRUE;
    *words_max_class_ptr = L_MIN (*words_max_class_ptr,
                                  l_tables->l_words[index].l_w_max_class);
    return (TRUE);
}

/*
 * The l_parse subroutine parses the passed character representation of
 * a label (either information, sensitivity, or clearance), using the passed
 * l_tables to indicate valid words for the label.  The resultant internal encodings are 
 * placed in the passed classification, compartments, and markings.  The syntax
 * of the input is as follows:
 *
 *      [+][CLASSIFICATION] [[+|-][WORD]]...
 *
 * where brackets denote optional entries, ... denotes zero or more of the
 * proceeding bracketed entry can be specified with blanks preceding it.
 * Blanks, tabs,  and slashes are interchangeable, with multiples allowed.
 * CLASSIFICATIONS and WORDS themselves can contain blanks if so encoded in the
 * encoding tables.
 *
 * If the passed string starts with + or - characters, then the string will be
 * interpreted as changes to the existing label. If the passed string starts
 * with a classification followed by a + or -, then the new classification
 * is used, but the rest of the old label is retained and modified as
 * specified in the passed string.  If the passed string is empty, then
 * l_parse returns immediately without changing the label.
 *
 * l_parse performs a great deal of error correction, and returns a -1 if
 * everything parsed OK; otherwise, the index into the input string where
 * parsing stopped is returned.
 * 
 * The maximum classification and compartments whose existence should be acknowledged
 * are also passed, to insure that no words in the l_words table above these
 * maxima are revealed.  Any such words are completed IGNORED.
 *
 * The minimum classification and compartments specifiable are also passed.  Any
 * valid label below these minima is automatically converted to these minima.
 *
 * The passed classification, compartments, and markings are assume to contain
 * a "previous value" of the label in case label modification is specified by the
 * input (with a + or -).  However, if the classification is NO_LABEL, then there is
 * assume to be no previous label, and any attempt to modify the label will be
 * treated instead as a specification of a new label.
 *
 * The allowed syntax of the input string allows l_parse to conveniently support
 * a "multiple choice" graphical interface in the following manner.  Such an
 * interface would typically display words that can appear in a label, along 
 * with an indication of whether the word is present in the current label.
 * l_parse can be called with "+word" to turn on a given word, or "-word" to
 * turn off a given word.
 */
 
int
l_parse (input, class_ptr, comps_ptr, marks_ptr, l_tables,
             min_class, min_comps_ptr, max_class, max_comps_ptr)
char *input;                        /* the input string to parse */
CLASSIFICATION *class_ptr;          /* the classification to use/return */
COMPARTMENTS *comps_ptr;            /* the compartments to use/return */
MARKINGS *marks_ptr;                /* the markings to use/return */
register struct l_tables *l_tables; /* the encodings tables to use */
CLASSIFICATION min_class;           /* min classification specifiable */
COMPARTMENTS *min_comps_ptr;        /* min compartments specifiable */  
CLASSIFICATION max_class;           /* max classification to acknowledge */
COMPARTMENTS *max_comps_ptr;        /* max compartments to acknowledge */   
{
    register int i;             /* array index */
    register char *s;           /* ptr into string to be parsed */
    register struct l_word_pair *wp;    /* ptr to word combinatiopn pair */
    int word_length;            /* length of a word in table being matched */
    int len_matched;            /* length of input matched each time */
    int index_matched;          /* index in word table of word matched against input */
    CLASSIFICATION class;       /* classification parsed */
    int prefix = L_NONE;            /* current prefix being parsed */
    int suffix = L_NONE;            /* current suffix being parsed */
    int add = TRUE;             /* are we adding or subtracting ? */
    int prefix_followed;        /* have we found something to go after a prefix found? */
    char *saved_s;              /* ptr to last prefix or word that needs suffix matched */
    char *parse_table;          /* ptr to parse table allocated by make parse table */
    char *norm_input;           /* ptr to normalized input string */
    short *norm_map;            /* ptr to normalizes input string mapping */
    int have_blank;             /* flag for normalization routine */
    CLASSIFICATION words_max_class; /* max classification allowed by words on */
        
    if (!l_encodings_initialized()) return (0); /* error return if encodings not in place */
    
/*
 * Copy the input string to norm_input, normalizing it by:
 *
 *      1) Forcing all alphabetic characters to upper case,
 *      2) Changing all slashes (/) and tabs to blanks, and
 *      3) Replacing multiple blanks with one blank.
 *
 * This normalization allows for uniform comparison of input against the
 * stored tables.
 *
 * Also, since the normalized string could be shorter than the original string,
 * maintain a mapping in norm_map between the strings.  For example, if the
 * ith character in norm_input corresponds to the i+3rd character in the
 * input string, then norm_map[i] = i+3.  The norm_map is used for returning
 * the index of erroneous input.
 */
    
    {
        register char *string;
        register short *nm;
        
        norm_input = Calloc (strlen(input)+1, 1);
        norm_map = (short *) Calloc (strlen(input)+1, sizeof (short));
        
        s = norm_input;
        nm = norm_map;
        
        have_blank = TRUE;  /* causes leading blanks to be ignored */
        
        for (string=input; *string; string++)   /* for each character in input string */
        {
            if (!have_blank
             && (*string==' ' || *string=='/' || *string=='\11' || *string == ','))
            {
                have_blank = TRUE;
                *s++ = ' ';
                *nm++ = (short) (string-input);
                continue;
            }
            else if (*string!=' ' && *string!='/' && *string!='\11' && *string!=',')
            {
                have_blank = FALSE;
                if (*string >= 'a' && *string <= 'z') *s++ = *string & ~('a'-'A');
                        /* force upper case */
                else
                    *s++ = *string;
                *nm++ = (short) (string-input);
            }
        }
        *s = '\0';
        s = norm_input;
    }

/*
 * If there are no non-blank characters left in the input string, just return
 * without doing anything.
 */

    if (*s == '\0')     /* if no input */
    {
        free (norm_map);
        free (norm_input);
        return (-1);        /* make no changes, report no errors */
    }
            
/*
 * Start out assuming that we will retain the original classification.  Also,
 * initialize words_max_class (the maximum classification allowed by words that
 * are on) to be the max_class passed, on the assumption (for now) that the user is NOT
 * changing an old label.
 */
 
    class = *class_ptr;
    words_max_class = max_class;

/*
 * If the first character of the string is a +, then this indicates that the
 * current label, as represented by the passed class, comps, and marks, is
 * to be modified rather than being replaced.  Therefore, if the first
 * character is a +, just skip it for now and proceed to look for a classification.
 */
 
    while (*s == '+') s++;
    
/*
 * once a - is entered, a classification is invalid.  Therefore, try to
 * parse a classification only if there is no -.  If the classification
 * cannot be parsed, assume no classification was entered, retain the
 * original classification, and continue parsing as a word.
 */
  
    if (*s != '-')
    {
        for (i=0; i <= max_class; i++)  /* for each classification */
        {
            if (l_long_classification[i])
            {
                word_length = strlen (l_short_classification[i]);
                if (0==strincmp (s, l_short_classification[i], word_length)
                 && (s[word_length]==' ' || s[word_length]=='\0'))
                {
                    s +=word_length;
                    break;
                }

                word_length = strlen (l_long_classification[i]);
                if (0==strincmp (s, l_long_classification[i], word_length)
                 && (s[word_length]==' ' || s[word_length]=='\0'))
                {
                    s +=word_length;
                    break;
                }
            }
        }

        if (i <= max_class) /* if class found */
            class = i;              /* save classification */
    }

/*
 * Raise the classification to the minimum allowed.
 */

    class = L_MAX (class, min_class);

/*
 * Allocate a parse_table of all FALSE entries.
 */
 
    parse_table = Calloc (l_tables->l_num_entries, 1);

/*
 * At this point, if a classification was found, s points after it.
 * Otherwise, s points to first word or a - before the first word.
 * If s now points to a + or -, or if the first character entered
 * (norm_input[0]) was a +, then set the parse table based on the
 * passed classification, compartments, and markings, because the user
 * has requested modifying the current label with the + or - (unless
 * the passed classification is NO_LABEL).  After the parse_table is
 * filled in, the words_max_class must be recomputed to be the lowest
 * l_w_max_class specified by the words on in the parse table.
 */
 
    if (*s == ' ') s++;     /* skip any blank */
    if (*class_ptr != NO_LABEL  /* if label already had a value */
     && (*s=='+' || *s=='-' || norm_input[0] == '+'))
    {
        make_parse_table (parse_table, *class_ptr, comps_ptr, marks_ptr, l_tables,
                          ALL_ENTRIES);
        for (i=l_tables->l_first_main_entry; i < l_tables->l_num_entries; i++)
            if (parse_table[i]) {
                words_max_class = L_MIN (words_max_class,
                                         l_tables->l_words[i].l_w_max_class);
		}
    }

/*
 * This is the start of the main loop to check each 
 * remaining part of the string against the word table.
 */
 
    while (*s != '\0')      /* while there is more left to parse... */
    {

/*
 * Check for + or -.  If found, set boolean add
 * appropriately and skip over + or -.
 */
 
        if (*s == ' ') s++;     /* skip any blank */
        if (*s=='\0') break;
        
        if (*s=='+' || *s=='-')
        {
            add = (*s=='+');    /* set add appropriately */
            s++;                /* skip over + or - */
            continue;           /* back to start of main loop */
        }

/*
 * Now, try to match the next part of the string against each word in the word
 * table that is visible given the maximum classification and compartments.  The longest
 * match found in the table will be used.  Therefore, if both "EYES" and "EYES ONLY"
 * appear in the word table in any order, the input string "EYES ONLY" will be
 * matched against "EYES ONLY", not "EYES".  
 */
 
        len_matched = 0;        /* nothing matched yet */
        for (i=0; i<l_tables->l_num_entries; i++)   /* for each word in table */
            if (WORD_VISIBLE (i, max_class, max_comps_ptr))
            {
/*
 * If this word appears to be visible, but requires another word that is not
 * visible, then treat this word as invisible also (and therefore ignore it
 * by continuing in the for loop).
 */

                for (wp=l_tables->l_required_combinations; wp->l_word1 != L_END; wp++)
                    if (i==wp->l_word1  /* if this word requires another... */
                     && !WORD_VISIBLE (wp->l_word2, max_class, max_comps_ptr))
                        break;
                if (wp->l_word1 != L_END) continue;

/*
 * Continue in loop (ignoring this word in table) if we are not parsing after
 * a prefix and this word requires a prefix.
 */
 
                if (prefix==L_NONE && l_tables->l_words[i].l_w_prefix >= 0) continue;
                
/*
 * Continue in loop (ignoring this word in table) if we ARE parsing after
 * a prefix and this word does not require this prefix.
 */

                if (prefix>=0 && prefix != l_tables->l_words[i].l_w_prefix) continue;

/*
 * Continue in loop (ignoring this word in table) if we are not parsing after
 * a word that requires a suffix and this word IS a suffix.
 */
 
                if (suffix==L_NONE && l_tables->l_words[i].l_w_suffix == L_IS_SUFFIX) continue;
                
/*
 * Continue in loop (ignoring this word in table) if we ARE parsing after
 * a word that requires a suffix and this word is not a suffix or another word
 * that requires the same suffix.
 */

                if (suffix>=0 && (suffix != l_tables->l_words[i].l_w_suffix
                                  && suffix != i)) continue;

/*
 * If this word is not to be ignored, and if the length of this word is greater than that 
 * of any word matched above this one in the table, then compare the string being parsed
 * to this word.  If it matches, save the new (greater) length matched and the index of
 * the word matched, and continue looking for a longer match by continuing in the loop.
 * If no match against the long name is found, try a match against the short name also.
 */
 
                word_length = strlen (l_tables->l_words[i].l_w_long_name);
                if (word_length > len_matched
                 && 0==strincmp (s, l_tables->l_words[i].l_w_long_name,word_length)
                 && (s[word_length]==' ' || s[word_length]=='\0'))
                {
                    len_matched = word_length;
                    index_matched = i;
                    continue;
                }
                
#if SEC_ENCODINGS
                if (l_tables->l_words[i].l_w_short_name)
                {
                    word_length = strlen (l_tables->l_words[i].l_w_short_name);
                    if (word_length > len_matched
                     && 0==strincmp (s, l_tables->l_words[i].l_w_short_name,word_length)
                     && (s[word_length]==' ' || s[word_length]=='\0'))
                    {
                        len_matched = word_length;
                        index_matched = i;
                        continue;
                    }
                }
#endif /* SEC_ENCODINGS */
            }

/*
 * Find out if string matches word in table.
 */
 
        if (len_matched > 0)    /* if found */
        {

/*
 * If we are parsing after having found a prefix, then this entry REQUIRES
 * this prefix, so set prefix_followed to indicate we have "satisfied" the
 * prefix and do normal processing.
 */
 
            if (prefix >= 0) prefix_followed = TRUE;

/*
 * If the word matched IS a prefix, we must record that we have encountered a
 * prefix and continue in the main loop.
 */
 
            else if (l_tables->l_words[index_matched].l_w_prefix==L_IS_PREFIX)
            {
                prefix = index_matched;     /* save prefix we found */
                prefix_followed = FALSE;    /* record that nothing follows it yet */
                saved_s = s;    /* save ptr to prefix word matched */
                s += len_matched;           /* s ptr to rest to parse */
                continue;   /* back to main loop; nothing more to do for this word */
            }

/*
 * Now, if the word REQUIRES a prefix, and we didn't have one,
 * we must process the error.
 */
 
            else if (l_tables->l_words[index_matched].l_w_prefix >= 0)  /* if proper prefix
                                                               didn't precede */            
                break;      /* out of while parse loop */
                
/*
 * If we have previously encountered a word that requires a suffix,
 * see if we have found it now.  If so, record that we no longer need a
 * suffix and continue at top of main loop.  If this is not the suffix,
 * break out of the loop to process an error.
 */
            
            if (suffix >= 0)    /* if we need a suffix */
            {
                if (index_matched == suffix)    /* if we found the suffix we await */
                {
                    suffix = L_NONE;
                    s += len_matched;           /* s ptr to rest to parse */
                    continue;       /* we've handled this word */
                }
            }
            
/*
 * If the word matched IS a suffix, then produce an error message because we
 * are not expecting a suffix at this point.
 */
 
            else if (l_tables->l_words[index_matched].l_w_suffix==L_IS_SUFFIX)
                break;      /* out of while parse loop */
            
/*
 * If this word REQUIRES a suffix, we should record that fact and do
 * normal processing (below).
 */
 
            else if (l_tables->l_words[index_matched].l_w_suffix >= 0)  /* if suffix required */
            {
                saved_s = s;    /* save ptr to word that needs suffix */
                suffix = l_tables->l_words[index_matched].l_w_suffix;   /* save suffix we need */
            }

            s += len_matched;           /* s ptr to rest to parse */
            
/*
 * Now that we have handled the special cases for prefixes and suffixes,
 * we can handle the word as an add or a subtract.  If an add, the 
 * appropriate entry in the parse table (index i) is set to TRUE to indicate
 * that the word at index i should be represented in the label.  If a
 * subtract, the appropriate entry in the parse table is set to FALSE.
 */
 
            if (add) 
                turnon_word (l_tables, parse_table, index_matched, class,
                             &words_max_class, max_class, max_comps_ptr);
            else
                turnoff_word (l_tables, parse_table, index_matched,
                              &words_max_class, max_class);
        }

/*
 * If control falls after the else below, then the string could not be matched
 * against entry in the word table that was not ignored.  This either means
 * that the string does not match any legitimate entry in the table, or that
 * we were looking for a word after a prefix, but the string does not match
 * any entry that can go after the same prefix.  Therefore, if the prefix
 * we had parsed has already been followed by some word (prefix_followed),
 * then indicate that we are no longer processing after a prefix and try to
 * look up the word again.  If the prefix was not followed, then the string
 * cannot be matched, so break out of the parse loop to process the error.
 */
 
        else    /* word not found in table */
        {
            if (prefix_followed)
            {
                prefix_followed = FALSE;
                prefix = L_NONE;
            }
            else
                break;  /* out of while parse loop */
        }
     }

/*
 * Now, if not all input was parsed, or a suffix is missing, or a prefix 
 * entered was not followed, process a syntax error.
 */

    if (suffix >= 0)    /* if a needed suffix was missing */
        s = saved_s;    /* backup s to input that needs the suffix */
    else if (prefix >= 0 && !prefix_followed)   /* if prefix not followed */
        s = saved_s;        /* backup s to prefix that was not followed */

    if (*s) /* if not all input parsed */
    {
        i = norm_map[(int) (s-norm_input)]; /* compute index into original input
                                               of syntax error */
        free (parse_table);
        free (norm_map);
        free (norm_input);
        return (i);
    }

/*
 * Now make sure the classification is high enough for all of the words on
 * in parse_table.
 */

    for (i=l_tables->l_first_main_entry; i < l_tables->l_num_entries; i++)
        if (parse_table[i])
            class = L_MAX (class, l_tables->l_words[i].l_w_min_class);

/*
 * At this point, if the class is higher than words_max_class, this is because
 * the user raised it too high above (no word could have raised it because words
 * are not turned on if they are above it).  Therefore, class can be lowered
 * to the words_max_class.
 */
    
    class = L_MIN (class, words_max_class);

/*
 * Now add any more words to the parse table that are indicated by the max_comps.
 * By calling turnon_word with any such inverse compartments, any other words 
 * they require will also be turned on.
 */
 
    for (i=l_tables->l_first_main_entry; i<l_tables->l_num_entries; i++)
    {
        if (!COMPARTMENT_MASK_EQUAL (l_tables->l_words[i].l_w_cm_mask,
                                     l_0_compartments)) /* word is compartment */
            if (COMPARTMENTS_IN (max_comps_ptr,
                                 l_tables->l_words[i].l_w_cm_mask,
                                 l_tables->l_words[i].l_w_cm_value))
                if (COMPARTMENTS_EQUAL (l_0_compartments,
                                        l_tables->l_words[i].l_w_cm_value))
                    if (class >= l_tables->l_words[i].l_w_output_min_class)
                        turnon_word (l_tables, parse_table, i, class,
                                     &words_max_class, max_class, max_comps_ptr);
    }

/*
 * Now the parse_table is correctly set, so set the passed classification,
 * compartments, and markings properly.
 *
 * First initialize the compartments and markings with their proper values if no
 * entries are indicated in the parse table.  There there can be several such
 * initial compartments and markings, depending the classification of the data.
 * The initial compartments and markings tables contains this information.
 */
 
 
    COMPARTMENTS_COPY (comps_ptr, l_in_compartments[class]);
    MARKINGS_COPY (marks_ptr, l_in_markings[class]);

/*
 * Now scan the parse table and set the appropriate bits in the compartments
 * and markings for each TRUE entry in the parse table.
 */
        
    for (i=l_tables->l_first_main_entry; i < l_tables->l_num_entries; i++)
        if (parse_table[i])
        {
            COMPARTMENTS_SET (comps_ptr,
                              l_tables->l_words[i].l_w_cm_mask,
                              l_tables->l_words[i].l_w_cm_value);
            MARKINGS_SET (marks_ptr,
                          l_tables->l_words[i].l_w_mk_mask,
                          l_tables->l_words[i].l_w_mk_value);
        }

/*
 * Now combine the minimum compartments with the compartments to make sure
 * we didn't go below the minimum.
 */

    COMPARTMENTS_COMBINE (comps_ptr, min_comps_ptr);

/*
 * Now return the classification.
 */
 
    *class_ptr = class;

/*
 * Free allocated areas and return with no error.
 */
 
    free (parse_table);
    free (norm_map);
    free (norm_input);
    return (-1);
}

/*
 * The l_convert subroutine converts the passed classification, compartments,
 * and markings into a character string representation thereof, which it
 * puts in the passed string.  The passed string is assumed to at least as
 * long as specified by the l_max_length in the passed l_tables, which specifies
 * the maximum amount of space needed to hold a label converted from the l_tables
 * encodings.
 *
 * If the parse_table argument is passed as NO_PARSE_TABLE, l_convert will allocate,
 * use, and free its own parse table.  However, if the caller wants access
 * to the parse table after l_convert returns, the caller can pass a ptr
 * to a parse table that l_convert will use instead.
 *
 * The parse table is a character array the same size as the passed l_words
 * table, containing TRUE in each character whose corresponding word is
 * represented in the label, and FALSE otherwise.  The parse table would be
 * useful to a caller implementing a "multiple choice" user interface for label
 * specification, by specifying which words in the multiple choice are present in
 * a given label.
 *
 * The l_classification argument is a ptr to a classification name table (either
 * long or short names), or NO_CLASSIFICATION if no classification is to be output.
 *
 * If the use_short_names arguments is passed as TRUE, then the short names for
 * words is output, rather than the output name.
 *
 * The flags argument specifies which l_word entries are to be considered when
 * scanning the l_word table.  If flags is ALL_ENTRIES, all entries are considered.
 * Otherwise, only those entries whose l_w_flags have all bits on as flags are
 * considered.
 *
 * This subroutine can be used for information, sensitivity, and clearance labels,
 * using different l_tables, and passing dummy markings for a sensitivity
 * label or clearance.  It can also be used for producing various labels for printer
 * banners.  Using the l_tables l_channel_tables produces output that represents
 * to appropriate "HANDLE VIA" caveat for printer banner pages, if passed
 * a sensitivity label.  Using the l_tables l_printer_banner produces a string
 * that represents "other" special printer banner output specific to the sensitivity
 * label passed.  Using the l_tables l_information_label_tables with flags of
 * ACCESS_RELATED produces those access related markings needed for part of the
 * printer banner page.
 */

void
l_convert (string, class, l_classification, comps_ptr, marks_ptr, l_tables,
                caller_parse_table, use_short_names, flags)
register char *string;              /* place to put the output */
CLASSIFICATION class;               /* the label classification */
char *l_classification[];           /* the human-readable classification table */
COMPARTMENTS *comps_ptr;            /* the label compartments */
MARKINGS *marks_ptr;                /* the label markings */
register struct l_tables *l_tables; /* the l_tables to use */
char *caller_parse_table;           /* place to return the parse table */
short use_short_names;              /* flag sez use short names for output */
short flags;                        /* l_w_flags of l_word entries to use */
{
    register int i;                 /* loop index for searching parse table */
    char *parse_table;              /* ptr to parse table for this label */
    int prefix = L_NONE;            /* indicates output of words after a prefix */
    int suffix = L_NONE;            /* indicates suffix must be output */
    int inserted_comma = 0;	    /* True if inserted a comma yet */

    if (!l_encodings_initialized()) return;     /* return if encodings not in place */

/*
 * First put classification in output string.
 */

    if (l_classification != NO_CLASSIFICATION) strcpy (string, l_classification[class]);
    else string[0] = '\0';

/*
 * Next, create and fill in a parse table for this label.
 */

    if (caller_parse_table != NO_PARSE_TABLE) 
        parse_table = caller_parse_table;
    else
        parse_table = Calloc (l_tables->l_num_entries, 1);

    make_parse_table (parse_table, class, comps_ptr, marks_ptr, l_tables, flags);

/*
 * Loop through each entry in the parse table to determine whether this word is
 * to be output.
 */

#if SEC_MAC_OB
	/* Format of Orange Book is  class /cats, cats/ */
    strcat (string, " /");
#endif /* SEC_MAC_OB */

    for (i=l_tables->l_first_main_entry;i<l_tables->l_num_entries;i++) 
	if (parse_table[i]) {
            
#if SEC_ENCODINGS
/*
 * Given that this word is to be output, first determine whether a / must be
 * output as a separation character.  A slash is needed if this is the
 * non-first word output after a prefix or before a suffix.
 */
 
            if (suffix >= 0 && suffix == l_tables->l_words[i].l_w_suffix
                    /* if suffix for this word is same as the previous */
             || prefix >= 0 && prefix == l_tables->l_words[i].l_w_prefix)
                    /* if prefix for this word is same as the previous */
                strcat (string, "/");

/*
 * If this word is not a continuation of words before a suffix or after a 
 * prefix and if the previous word required a suffix, then output that 
 * suffix now.
 */
 
            else
            {
                if (suffix >= 0)
                {
                    string+=strlen(string); /* output the suffix */
                    sprintf (string, " %s",
                             (use_short_names &&
                             l_tables->l_words[suffix].l_w_short_name) ?
                             l_tables->l_words[suffix].l_w_short_name :
                             l_tables->l_words[suffix].l_w_output_name);
                }
 
/*
 * Now, assume we have no prefix and then check whether
 * this word requires one.  If so, remember what prefix it requires and output
 * the prefix.  Otherwise, output just the normal blank separator.
 */
 
                prefix = L_NONE;        /* end of prefix we had output (if any) */
                if (l_tables->l_words[i].l_w_prefix >= 0)       /* if prefix required */
                {
                    prefix = l_tables->l_words[i].l_w_prefix;   /* remember prefix */
                    string+=strlen(string);
                    sprintf (string, " %s ",
                             (use_short_names &&
                             l_tables->l_words[prefix].l_w_short_name) ?
                             l_tables->l_words[prefix].l_w_short_name :
                             l_tables->l_words[prefix].l_w_output_name);

                }
                else
                    strcat (string, " ");   /* put on normal separator */
            }
            
/*
 * Now, if this word requires that a suffix be printed after it, remember in index
 * of the suffix in the word table for output near the top of this loop, or after loop.
 */

            suffix = l_tables->l_words[i].l_w_suffix;   /* will be L_NONE or index of
                                                           suffix in word table */
            
#endif /* SEC_ENCODINGS */
/*
 * Now we are finally ready to output the word matched.
 */
 
#if SEC_ENCODINGS
            strcat (string,
                    (use_short_names &&
                    l_tables->l_words[i].l_w_short_name) ?
                    l_tables->l_words[i].l_w_short_name :
                    l_tables->l_words[i].l_w_output_name);
#else
	/* Don't insert comma for first classification */
	    if (inserted_comma)
		strcat (string, ", ");
	    else 
		inserted_comma = 1;

            strcat (string,
                    l_tables->l_words[i].l_w_output_name);
#endif /* SEC_ENCODINGS */

    }

/*
 * Now that all words have been output, output the trailing suffix if one is needed.
 */

#if SEC_ENCODINGS
    if (suffix >= 0)
    {
        string+=strlen(string); /* output the suffix */
        sprintf (string, " %s",
                 (use_short_names &&
                 l_tables->l_words[suffix].l_w_short_name) ?
                 l_tables->l_words[suffix].l_w_short_name :
                 l_tables->l_words[suffix].l_w_output_name);

    }
#endif /* SEC_ENCODINGS */

/*
 * Finally, free the parse table if allocated.
 */
 
    if (caller_parse_table == NO_PARSE_TABLE) free (parse_table);
#if SEC_MAC_OB
	/* Insert a trailing slash for Orange Book */
	strcat (string, "/");
#endif /* SEC_MAC_OB */
}

/*
 * The external subroutine l_b_parse uses l_parse (above) to parse an input
 * string of the form:
 *
 *      INFORMATION LABEL [SENSITIVITY LABEL]
 *
 * into the internal representation of the information label and sensitivity label
 * specified.  If a parsing error occurs, the index into the passed string where
 * parsing stopped is returned.  A -1 is returned if everything parsed OK.
 * Since the legality of an information is determined by its associated sensitivity
 * label (because the IL classification and compartments cannot be above the
 * SL classification and compartments), the sensitivity label is parsed first,
 * and its values are used in parsing the information label.
 *
 * The passed maximum classification and compartments are used when parsing the
 * sensitivity label, so that no information classified above these maxima are
 * revealed.
 *
 * When parsing the sensitivity label, its minimum value is taken to be the high
 * water mark of the information level value and the l_lo_sensitivity_label value.
 *
 * If no [ is present in the input string, the passed sensitivity label is left
 * unchanged.  If a [ is present, the closing ] is optional.
 *
 * If the last argument is TRUE, the sensitivity label can be changed, and is
 * therefore parsed.  Otherwise, it is completely ignored if present.
 *
 * This subroutine implements an important convention for specifying changes to
 * both and IL and a SL: that the IL cannot be raised above the SL without first
 * changing the SL.  Therefore, the SL portion is parsed first so that a
 * corresponding rise in the IL is not disallowed because the SL hasn't been
 * raised yet.
 *
 * The global temporary compartments and markings (l_t_compartments and l_t_markings)
 * are changed if the allow_sl_changing argument is TRUE.
 */

	/* Don't use for orange book B1 */
#ifdef NOT_USED
int
l_b_parse (input, iclass_ptr, icomps_ptr, imarks_ptr,
               sclass_ptr, scomps_ptr, max_class, max_comps_ptr,
               allow_sl_changing)
char *input;                    /* the input string to parse */
CLASSIFICATION *iclass_ptr;     /* the information label classification */
COMPARTMENTS *icomps_ptr;       /* the information label compartments */
MARKINGS *imarks_ptr;           /* the information label markings */
CLASSIFICATION *sclass_ptr;     /* the sensitivity label classification */
COMPARTMENTS *scomps_ptr;       /* the sensitivity label compartments */
CLASSIFICATION max_class;       /* the maximum classification to reveal */
COMPARTMENTS *max_comps_ptr;    /* the maximum compartments to reveal */
int allow_sl_changing;          /* flag sez SL can be changed */
{
    register int i;
    register char *sen_label;
    int error;
    CLASSIFICATION sl_min_class;
    
    if (!l_encodings_initialized()) return (0); /* error return if encodings not in place */

    for (i=0; input[i]; i++) /* scan input */
        if (input[i] == '[')    /* if SL found */
        {
            input[i] = '\0';   /* terminate IL */

            if (allow_sl_changing)
            {
                sl_min_class = L_MAX (*iclass_ptr,
                                      l_lo_sensitivity_label->l_classification);
                                    
                COMPARTMENTS_COPY (l_t_compartments, l_lo_sensitivity_label->l_compartments);
                if (*iclass_ptr != NO_LABEL)    /* make sure SL > IL, if IL present */
                    COMPARTMENTS_COMBINE (l_t_compartments, icomps_ptr);
                    
                                      
                sen_label = &input[i+1];
                for (i=0; sen_label[i]; i++)    /* scan SL for closing ] */
                    if (sen_label[i] == ']')
                    {
                        sen_label[i] = '\0';    /* remove closing ] */
                        break;
                    }
     
                error = l_parse (sen_label, sclass_ptr, scomps_ptr, l_t_markings,
                                 l_sensitivity_label_tables,
                                 sl_min_class,
                                 l_t_compartments,
                                 max_class, max_comps_ptr);
                if (error != -1)    /* if a parsing error occurred */
                    return (error + (sen_label - input)); /* return proper error index */
            }
        }

    return (l_parse (input, iclass_ptr, icomps_ptr, imarks_ptr,
                     l_information_label_tables,
                     *l_min_classification, l_0_compartments,
                     *sclass_ptr, scomps_ptr));
}
    
/*
 * The external subroutine l_b_convert uses l_convert (above) to convert
 * the passed information label and sensitivity label internal representations
 * into a string of the form:
 *
 *      INFORMATION LABEL [SENSITIVITY LABEL]
 *
 * The passed string must be of size l_information_label_tables->l_max_length +
 * l_sensitivity_label_tables->l_max_length + 3.
 *
 * If the caller desires, the information label and sensitivity label parse tables
 * used for conversion can be returned in the passed parse tables.  The parse tables
 * are passed as above for l_convert.
 */

void
l_b_convert (string, iclass, icomps_ptr, imarks_ptr, iparse_table,
                  sclass, scomps_ptr, sparse_table, use_short_names)
char * string;              /* the place to return the output string */
CLASSIFICATION iclass;      /* the information label classification */      
COMPARTMENTS *icomps_ptr;   /* the information label compartments */
MARKINGS *imarks_ptr;       /* the information label markings */
char *iparse_table;         /* the information label parse table */
CLASSIFICATION sclass;      /* the sensitivity label classification */
COMPARTMENTS *scomps_ptr;   /* the sensitivity label compartments */
char *sparse_table;         /* the sensitivity label parse table */
int use_short_names;        /* flag sez use short names for output */
{
    register char *sp;      /* fast ptr into output string */
    
    if (!l_encodings_initialized()) return;     /* error return if encodings not in place */

    sp = string;
    
    l_convert (sp, iclass, l_long_classification, icomps_ptr, imarks_ptr,
               l_information_label_tables, iparse_table, use_short_names, ALL_ENTRIES);
    
    sp += strlen (sp);      /* sp now ptr after information label in string */
    
    strcpy (sp, " [");      /* add starting sensitivity label delimiter */
    
    sp += 2;                /* sp now ptr to place for sensitivity label */

    l_convert (sp, sclass, l_short_classification, scomps_ptr, l_t_markings,
               l_sensitivity_label_tables, sparse_table, use_short_names, ALL_ENTRIES);
    
    strcat (sp, "]");       /* add closing delimiter */
}
#endif /* NOT_USED */

/*
 * The external subroutine l_in_accreditation_range returns TRUE iff the
 * passed classification and compartments are in the accreditation range
 * of the system.
 */

int
l_in_accreditation_range (class, comps_ptr)
CLASSIFICATION class;               /* sensitivity label classification */
register COMPARTMENTS *comps_ptr;   /* sensitivity label compartments */
{
    register char *ar;  /* fast ptr to accreditation range compartments */
    
    if (!l_encodings_initialized()) return (FALSE); /* error return if encodings not in place */

    switch (l_accreditation_range[class].l_ar_type)
    {
        case L_ALL_VALID_EXCEPT:
        
            for (ar=l_accreditation_range[class].l_ar_start;
                 ar!=l_accreditation_range[class].l_ar_end;
                 ar += COMPARTMENTS_SIZE)
                if (COMPARTMENTS_EQUAL (comps_ptr, (COMPARTMENTS *) ar))
                    break;

            if (ar!=l_accreditation_range[class].l_ar_end)
                return (FALSE); /* no SL in AR */
            else
                return (TRUE);
            
        case L_ALL_VALID:
        
                return (TRUE);

        case L_ONLY_VALID:
        
            for (ar=l_accreditation_range[class].l_ar_start;
                 ar!=l_accreditation_range[class].l_ar_end;
                 ar += COMPARTMENTS_SIZE)
             if (COMPARTMENTS_EQUAL (comps_ptr, (COMPARTMENTS *) ar))
                break;
            if (ar!=l_accreditation_range[class].l_ar_end)
                return (TRUE);  /* found SL in AR */
            else
                return (FALSE);
            
        case L_NONE_VALID:
        
                return (FALSE);
    }
}
#endif /* SEC_MAC_OB */
#endif /* SEC_BASE */
