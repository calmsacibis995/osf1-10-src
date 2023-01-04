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
static char	*sccsid = "@(#)$RCSfile: options.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:48:28 $";
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
 *   options.c
 *   
 *   Contents:
 *
 *   int parse_args(argcp, argvp, opt_without_val, opt_with_val,
 *		required_args)
 *		int *argcp;
 *		char ***argvp;
 *		char *opt_without_val;
 *		char *opt_with_val;
 *		unsigned int required_args;
 *	Parses the command line, checking for one-character options specified
 *	in two strings (one for those without separate argument, one for those
 *	with separate argument) and checking also that a number of required
 *	arguments is supplied.
 *	Either NULL or "" can be used to specify that no options are allowed.
 *	Returns OK if the command line is coherent with the specified options;
 *	otherwise returns NOT_OK.
 *
 *   int used_opt(key)
 *		char key;
 *	Returns TRUE or FALSE, according to the presence of the option 
 *	specified with "key" in the command line previously parsed.
 *
 *   char *value_of_opt(key)
 *		char key;
 *	Returns the command line string given as argument to the option
 *	specified by "key". NULL is returned if the option was not typed
 *	within the command line.
 *
 *   char *next_arg()
 *	After the command line has been parsed, the requested and extra
 *      arguments can be accessed with this function. It returns NULL
 *	when there are nomore arguments left.
 *
 *   char **left_arg(numptr)
 *		int *numptr;
 *	Returns what has not been used yet of the arguments list; useful
 *	for lists of arguments with the same meaning (files, etc.); this
 *	inhibits further calls to next_arg(). Number of args still available
 *	is copied into *numptr.
 *
 *   int print_arg_error()
 *	Issues an error message on stderr, corresponding to the error code
 *	set by parse_args().
 *
 *   int print_usage(usage_msg)
 *		char *usage_msg
 *	Formats and prints the user-supplied usage message.
 *
 *   int usage_error(bad_input, err_msg)
 *		char *bad_input, *err_msg;
 *	Allows the user to drive the error reporting issued by
 *	print_arg_error().
 *
 *   int bad_char_arg_value(flagptr, valptr, flag, flagmeaning, goodvalues)
 *		char *flagptr, *valptr, flag, *flagmeaning, *goodvalues;
 *	Checks that the flag specified by "flag", if used, has a value in
 *	the string "goodvalues"; if so, or if the flag has not been used,
 *	returns FALSE (=> good value), storing a boolean value via "flagptr"
 *	and (if present) the single-char value via valptr; otherwise, returns
 *	TRUE and prepares a error message (using "flagmeaning") to be reported
 *	by print_arg_error().
 *
 *   int bad_int_arg_value(flagptr, valptr, flag, flagmeaning)
 *		char *flagptr, flag, *flagmeaning;
 *		int *valptr;
 *	Checks that the flag specified by "flag", if used, is an integer
 *	number; if so, or if the flag has not been used,
 *	returns FALSE (=> good value), storing a boolean value via "flagptr"
 *	and the int value (if present) via valptr; otherwise, returns TRUE and
 *	prepares a error message (using "flagmeaning") to be reported by
 *	print_arg_error().
 *
 *   char *program_name()
 *	Returns the basename of the program name as supplied in argv[0].
 *
 *   Example:
 *	The user issued: "this_cmd -a -d -f file -cb reqarg1 extraarg2".
 *	   int retcode;
 *	   char *filename;
 *	   char *currentarg;
 *
 *	   retcode = parse_args(&argc, &argv, "abcd", "f", 1);
 *	   if (retcode != OK) {
 *	       print_usage(USAGE);
 *	       print_arg_error();
 *	       exit(FATAL_ERROR);
 *	   }
 *	   if (used_opt('a')) aflag = TRUE;
 *	   ...
 *	   if (used_opt('f')) filename = value_of_opt('f');
 *	   while ((currentarg = next_arg()) != NULL)
 *	      ...
 *	Thie while loop will be executed twice, once for "reqarg1", and once
 *	for extraarg2.
 */

#include "lvmcmds.h"


/* Maximum number of options allowed */
#define MAXOPTIONS	30
#define	LF_TAB		"\n\t"

/* Values for legal_opt() */
#define WITH_VALUE	TRUE
#define WITHOUT_VALUE	(!WITH_VALUE)

/* When an argument looks like an option, add a '-' in front of it */
#define special_case(word)	((word)[0] == '-' && (word)[1] == '-')

/* Type of descriptor of option */
typedef struct {
   char key;		/* how to distinguish this option */
   char val_required;	/* TRUE if a value is required for this */
   char opt_used;	/* TRUE if the user used this opt, FALSE otherwise */
   char *usr_value;	/* what the user has typed in for it */
} opt_descr_t;

/* Local functions */
static int legal_opt(char key, int value_required);
static opt_descr_t *search_opt(register char opt_char);
static clean_opt_list();
static char *next_word(register char **argv);
static int key_to_value(char flag, char **valueptr);
static int save_name(char *name);

/* Table of legal options */
static opt_descr_t opt_list[MAXOPTIONS];

/* Count of legal options */
static unsigned int opt_cnt = 0;

/* Save a local copy of what we return to the user */
static char **argv_copy;
static int argc_copy;

/* Save the name of the program */
static char *prog_basename;

/* Error memo and error messages */
static char *opt_errmsg = NULL;
static char *bad_input;



int
parse_args(
   int *argcp,
   char ***argvp,
   char *opt_without_val,
   char *opt_with_val,
   unsigned int required_args
)
{
   register int i;
   register char *cur_word;
   register char c;
   register opt_descr_t *opt_ptr;
   char *usr_value;
   int ret_code;
   int argc;
   char **argv;

   /* Insert debug option in the table */
   set_debug_opt();

   /* Insert legal options in the table */
   if (opt_without_val != NULL)
      /* Loop on the "boolean" flags */
      while ((c = *opt_without_val++) != '\0')
	 legal_opt(c, WITHOUT_VALUE);
	 
   if (opt_with_val != NULL)
      /* Loop on the "programmable" flags */
      while ((c = *opt_with_val++) != '\0')
	 legal_opt(c, WITH_VALUE);
	 
   /* Once an error has been done, all functions won't work */
   if (opt_errmsg != NULL)
      return(NOT_OK);

   /* Set defaults */
   ret_code = OK;

   /* Pretend to be a main() */
   argc = *argcp;
   argv = *argvp;

   /* Save name of program */
   save_name(argv[0]);

   /* Skip name of command */
   ++argv, --argc;

   /*
    *   Loop through command line words: exit from loop if there are
    *   no more words, or the options are finished
    */

   for ( ; (cur_word = *argv) != NULL; argv++, argc--) {

      if (cur_word[0] != '-')
	 break;

      /*
       *   Deal immediately with the special case of "--" (to mean
       *   that "-" is a true argoment, rather than an option)
       */

      if (special_case(cur_word))
	 break;

      /* Skip '-' */
      ++cur_word;

      /* Only '-' supplied? */
      if (*cur_word == '\0') {
	 opt_errmsg = MSG_NO_CHAR;
	 bad_input = NULL;
         ret_code = NOT_OK;
      }

      /* Search for the single character options that make up this word */
      while (*cur_word != '\0' && ret_code == OK) {

         opt_ptr = search_opt(*cur_word);

         /* If there's no such option, or it has been used, it's an error */
         if (opt_ptr == NULL || opt_ptr->opt_used) {

	    /* Store enough info for error reporting */
	    opt_errmsg = (opt_ptr == NULL) ? MSG_ILLEGAL_OPT :
					     MSG_OPT_USED_TWICE;
	    bad_input = cur_word;

            ret_code = NOT_OK;
	 }
         else {

            opt_ptr->opt_used = TRUE;

            /* Might need a value for that argument */
            if (opt_ptr->val_required) {

	       /* If a value is required, the option must be alone */
	       if (cur_word[-1] != '-' || cur_word[1] != '\0') {

	          /* Store enough info for error reporting */
	          opt_errmsg = MSG_NOT_ALONE;
	          bad_input = cur_word;

	          ret_code = NOT_OK;
	       }
	       else {

	          /* Get value from next word. Check for errors */
	          usr_value = next_word(argv);
	          if (usr_value == NULL) {

	             /* Store enough info for error reporting */
	             opt_errmsg = MSG_VALUE_REQ;
	             bad_input = cur_word;

	             /* At least, one error. It's not worth continuing */
	             ret_code = NOT_OK;
		  }
	          else {

		     /* Skip next word: was the value */
	             ++argv, --argc;
		     opt_ptr->usr_value = usr_value;
		  }
	       }
	    }
	 }
	 cur_word++;
      }

      /* After scanning the word, check for some error */
      if (ret_code == NOT_OK)
         break;
   }

   /* Check for compulsory arguments (the number, at least) */
   if (argc < required_args) {

      /* Store enough info for error reporting */
      opt_errmsg = MSG_MORE_ARGS;
      bad_input = NULL;

      ret_code = NOT_OK;
   }

   /* Let the caller deal with compulsory arguments */
   if (ret_code == OK) {

      /* Let the user concern not about throwing away leading '-' from "--" */
      if (*argv != NULL && special_case(*argv))
         ++*argv;

      /* Save what's left of arguments: there are no more options */
      *argcp = argc;
      *argvp = argv;
   }
   else {

      /* If there was some error, discard only cmd name from argv/argc */
      ++*argvp;
      --*argcp;

      /* This means that NO option is accepted */
      clean_opt_list();

      /* opt_errmsg has been set, so we we will reject all future requests */
   }

   /* Store a local copy of what we return */
   argv_copy = *argvp;
   argc_copy = *argcp;

   return(ret_code);
}



int
used_opt(
   char key
)
{
   register opt_descr_t *opt_ptr;

   /* Once an error has been done, all functions won't work */
   if (opt_errmsg != NULL)
      return(FALSE);

   /* Search for an option that matches the given key */
   opt_ptr = search_opt(key);

   /* If no such option was defined as legal, return FALSE */
   if (opt_ptr == NULL) {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(FALSE);
   }

   return(opt_ptr->opt_used);
}



char *
value_of_opt(
   char key
)
{
   register opt_descr_t *opt_ptr;

   /* Once an error has been done, all functions won't work */
   if (opt_errmsg != NULL)
      return(NULL);

   /* Search for an option that matches the given key */
   opt_ptr = search_opt(key);

   /*
    *   If no such option was defined as legal, or it was
    *   not used, return NULL
    */
   if (opt_ptr == NULL || !opt_ptr->opt_used) {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(NULL);
   }

   /* Might as well be NULL... */
   return(opt_ptr->usr_value);
}



char *
next_arg()
{
   char *arg;

   arg = NULL;

   /* Very easy: we fool the user with our local copy */
   if (argc_copy > 0) {
      arg = *argv_copy++;
      argc_copy--;
   }
   
   return(arg);
}



char **
left_arg(
   int *numptr
)
{

   char **list;
   int num;

   /* Save values to be returned */
   list = argv_copy;
   num = argc_copy;

   /* This is the last time the user gets something from us */
   argv_copy = NULL;
   argc_copy = 0;

   /* The user wants to access what's left of the arguments. Let him do it */
   *numptr = num;
   return(list);
}



int
print_arg_error()
{
   /* Very simple */
   if (bad_input != NULL)
      fprintf(stderr, "\"%s\": ", bad_input);
   fprintf(stderr, opt_errmsg);
   putc('\n', stderr);

   return(OK);
}



int
print_usage(
   register char *usage_msg
)
{
   register char c;
   register int brack;
   register int met_options;
   register int end_of_options;
   register int alternative;

   /* Split the usage message on several lines */
   brack = 0;
   met_options = FALSE;
   end_of_options = FALSE;

   while ((c = *usage_msg++) != '\0') {

      /* This might change at any time */
      alternative = FALSE;

      /*
       *   Print spaces as they are; after end of options, print
       *   everything on one line only
       */

      if (!isspace(c) && !end_of_options) {

         switch (c) {

	    /* Optional items or constrained choices */
	    case '{':
	    case '[':
		  met_options = TRUE;
	          if (brack == 0) {
		     putc('\n', stderr);
		     putc('\t', stderr);
	          }
	          brack++;
	       break;

	    case '}':
	    case ']':
	          brack--;
	       break;

	    /* Alternative items of constrained choices */
	    case '|':
		  if (brack == 1) 
		     alternative = TRUE;
	       break;

	    default:
		  /* If opt's have already been printed, start the last line */
		  if (brack == 0 && met_options) {
		     end_of_options = TRUE;
	             fputs(LF_TAB, stderr);
		  }
         }
      }

      putc(c, stderr);

      /* Might need a nice indentation */
      if (alternative) 
	 fputs(LF_TAB, stderr);
   }

   return(OK);
}



int
usage_error(
   char *bad_inp,
   char *err_msg
)
{
   /* Very simple */
   bad_input = bad_inp;
   opt_errmsg = err_msg;
   return(OK);
}



int
bad_char_arg_value(
   char *flagptr,
   char *valptr,
   char flag,
   char *flagmeaning,
   char *goodvalues
)
{
   char *value_supplied;
   static char err_msg[80];

   /* Check for correct usage */
   if (flagptr == NULL || valptr == NULL ||
	 goodvalues == NULL || goodvalues[0] == '\0') {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(TRUE);
   }

   /* Set defaults: *valptr must not be overwritten */
   *flagptr = FALSE;

   /* Convert the key to the string (or NULL if not used) */
   if (key_to_value(flag, &value_supplied) != OK)
      return(TRUE);

   /* Not used? */
   if (value_supplied == NULL)
      return(FALSE);

   /* Finally, check that the user has been polite */
   if (value_supplied[1] != '\0' || !in_string(value_supplied[0], goodvalues)) {
      sprintf(err_msg, MSG_BAD_VALUE_SUPPLIED, goodvalues);
      usage_error(flagmeaning, err_msg);
      return(TRUE);
   }

   /* Against every expectation, the user has been correct */
   *flagptr = TRUE;
   *valptr = value_supplied[0];
   return(FALSE);
}



int
bad_int_arg_value(
   char *flagptr,
   int *valptr,
   char flag,
   char *flagmeaning
)
{
   char *value_supplied;
   int int_value;

   /* Check for correct usage */
   if (flagptr == NULL || valptr == NULL) {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(TRUE);
   }

   /* Set defaults: *valptr must not be overwritten */
   *flagptr = FALSE;

   /* Convert the key to the string (or NULL if not used) */
   if (key_to_value(flag, &value_supplied) != OK)
      return(TRUE);

   /* Not used? */
   if (value_supplied == NULL)
      return(FALSE);

   /* Finally, check that the user has been polite */
   if (sscanf(value_supplied, "%d", &int_value) != 1) {
      usage_error(flagmeaning, MSG_MUST_BE_NUMBER);
      return(TRUE);
   }

   /* Against every expectation, the user has been correct */
   *flagptr = TRUE;
   *valptr = int_value;
   return(FALSE);
}



char *
program_name()
{
   /* Trivial */
   return(prog_basename);
}



static int
legal_opt(
   char key,
   int value_required
)
{
   register opt_descr_t *opt_ptr;

   /* Once an error has been done, all functions won't work */
   if (opt_errmsg != NULL)
      return(NOT_OK);

   /* Check for space; see if this option has been already defined */
   if (opt_cnt == MAXOPTIONS || search_opt(key) != NULL) {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(NOT_OK);
   }

   /* Check for correct usage */
   if ((key == '\0' || key == '-') ||
	  (value_required != WITH_VALUE && value_required != WITHOUT_VALUE)) {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(NOT_OK);
   }

   /* Store this option in the list */
   opt_ptr = &opt_list[opt_cnt++];

   /* Initialize new entry */
   opt_ptr->key = key;
   opt_ptr->val_required = value_required;
   opt_ptr->opt_used = FALSE;
   opt_ptr->usr_value = NULL;

   return(OK);
}



static opt_descr_t *
search_opt(
   register char opt_char
)
{
   register opt_descr_t *opt_ptr;
   register int i;

   /* Scan table, searching for that specific character */
   for (opt_ptr = opt_list, i = 0; i < opt_cnt; i++, opt_ptr++) {
      if (opt_ptr->key == opt_char)
	 return(opt_ptr);
   }

   /* Not found */
   return(NULL);
}



static
clean_opt_list()
{
   register opt_descr_t *opt_ptr;
   register int i;

   /* Scan table, resetting usage and value */
   for (opt_ptr = opt_list, i = 0; i < opt_cnt; i++, opt_ptr++) {
      opt_ptr->opt_used = FALSE;
      opt_ptr->usr_value = NULL;
   }
}



static char *
next_word(
   register char **argv
)
{
   register char *word;

   /* See if next word can be a good argument */
   word = argv[1];
   if (word != NULL) {
      
      /* Might be. See if it's not a list of options */
      if (word[0] == '-') {
	 
	 /* There might be some options left. Error? */
         if (special_case(word)) 
	    word++;
	 else
	    word = NULL;
      }
   }

   return(word);
}



static int
key_to_value(
   char flag,
   char **valueptr
)
{
   int been_used;
   char *value_supplied;

   /* Set defaults */
   *valueptr = NULL;

   /* See if option has been used: if not so, no value has been supplied */
   if (!used_opt(flag) && opt_errmsg == NULL)
      return(OK);

   /* Get the value */
   value_supplied = value_of_opt(flag);  

   /* The following should be impossible... */
   if (value_supplied == NULL) {

      /* Reject all future requests */
      opt_errmsg = MSG_PROG_ERROR;
      return(NOT_OK);
   }

   /* Store the value */
   *valueptr = value_supplied;
   return(OK);
}



static int
save_name(
   char *name
)
{
   register char *cp;
   register int len;

   /* Search for a '/', from the end to the beginning */
   len = strlen(name);
   for (cp = &name[len - 1]; cp >= name; cp--)
      if (*cp == '/')
	 break;

   /* Store the basename */
   prog_basename = (cp < name) ? name : cp + 1;
}
