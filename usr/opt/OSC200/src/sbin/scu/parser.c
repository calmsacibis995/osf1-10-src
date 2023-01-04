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
static char *rcsid = "@(#)$RCSfile: parser.c,v $ $Revision: 1.1.3.6 $ (DEC) $Date: 1992/07/31 16:45:40 $";
#endif
/*
 * File:	parse.c - Table Parser.
 * Author:	Robin T. Miller
 * Date:	November 21, 1990
 *
 * Description:
 *	This file contains the functions to implement a table parser.
 *
 * Modification History:
 *
 */
#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <time.h>

#ifdef OSF
#define SECS_PER_DAY (60 * 60 * 24)
#define TM_YEAR_BASE 1900
#else /* OSF */
#include <tzfile.h>
#endif /* OSF */

#include "parser.h"

/*
 * Externel/Forward Declarations:
 */
#ifdef PROTOTYPE

extern char *malloc(unsigned size);
static char *GetKeyName(struct key_entry *ke);
int ParseCommands (struct parser_control *pc);
static int ParseKeywords (struct parser_control *pc, struct key_entry *kp);
static int ParseKeyTable (struct parser_control *pc,
			  struct cmd_entry *ce, struct key_entry *ke);
static int ParseControl (struct parser_control *pc, struct cmd_entry *ce,
			 struct key_entry *ke, caddr_t argp);
static int ParseBoolean (char *str, u_long *argp);
int ParseDate (char *str, struct parser_date *date);
static int ParseType (struct parser_control *pc, struct parser_entry *pe);
int PromptUser (struct parser_control *pc, enum parser_types type,
		int flags, char *prompt, char *deflt, char *argp,
		char *optp, char prchar);
static int MoreKeys (struct parser_control *pc, struct cmd_entry *ce,
		     struct key_entry *ke, int keyc);
static int NeedBoolean (struct parser_control *pc,
			struct cmd_entry *ce, struct key_entry *ke);
/* static int NeedFlag (struct parser_control *pc, struct cmd_entry *ce); */
static int NeedKeyc (struct parser_control *pc,
		     struct cmd_entry *ce, struct key_entry *ke);
static int NeedString (struct parser_control *pc,
		       struct cmd_entry *ce, struct key_entry *ke);
static int NeedDate (struct parser_control *pc,
		     struct cmd_entry *ce, struct key_entry *ke);
static int NeedValue (struct parser_control *pc,
		      struct cmd_entry *ce, struct key_entry *ke);
static int CheckRange (struct parser_control *pc,
		       struct cmd_entry *ce, struct key_entry *ke);
struct cmd_entry *FindCmdEntry (enum parser_tokens cmd_token);
struct key_entry *FindKeyEntry (struct cmd_entry *ce,
				enum parser_tokens cmd_token,
				enum parser_tokens key_token);

#else /* !PROTOTYPE */

#ifdef OSF
extern void *malloc();
#else /* OSF */
extern char *malloc();
#endif /* OSF */

static char *GetKeyName();

#endif /* PROTOTYPE */

/*
 * Local Declarations:
 */
struct parser_control *Pc;		/* Pointer to parser control.	*/

/*
 * Define the Months Table:
 */
struct month_entry MonthTable[NUMBER_OF_MONTHS] = {
    {	MONTH_NONE,	"XXX",		"None"		},
    {	MONTH_JAN,	"Jan",		"January"	},
    {	MONTH_FEB,	"Feb",		"February"	},
    {	MONTH_MAR,	"Mar",		"March"		},
    {	MONTH_APR,	"Apr",		"April"		},
    {	MONTH_MAY,	"May",		"May"		},
    {	MONTH_JUN,	"Jun",		"June"		},
    {	MONTH_JUL,	"Jul",		"July"		},
    {	MONTH_AUG,	"Aug",		"August"	},
    {   MONTH_SEP,	"Sep",		"September"	},
    {	MONTH_OCT,	"Oct",		"October"	},
    {	MONTH_NOV,	"Nov",		"November"	},
    {	MONTH_DEC,	"Dec",		"December"	}
};

/************************************************************************
 *									*
 * ParseCommands() - Parse A Command List.				*
 *									*
 * Inputs:	pc = Pointer to the parse control block.		*
 *									*
 * Return Value:							*
 *		Returns 0 / 1 / -1 = SUCCESS / CONTINUE / FAILURE.	*
 *									*
 ************************************************************************/
int
ParseCommands (pc)
register struct parser_control *pc;
{
	register char *argp;
	register struct cmd_entry *ce;
	register int status;

	Pc = pc;
	CLR_COPTS (pc);
	if (pc->pc_ce) {
	    CLR_KOPTS (pc->pc_ce);
	}
	while ( pc->pc_argc ) {

	    pc->pc_argc--;		/* Adjust the argument count.	*/
	    argp = *pc->pc_argv++;	/* Pointer to next argument.	*/

	    /*
	     * Search the command table, looking for a match.
	     */
	    for (ce = pc->pc_cp; ce->c_name; ce++) {
		if ( (pc->pc_flags & PC_EXACT) || (ce->c_flags & PF_EXACT) ) {
		    if (strcmp (argp, ce->c_name) == 0) {
			break;
		    } else if (ce->c_alias) {
			if (strcmp (argp, ce->c_alias) == 0) {
			    break;
			}
		    }
		} else {
		    if (strncmp (argp, ce->c_name, strlen(argp)) == 0) {
			break;
		    } else if (ce->c_alias) {
			if (strncmp (argp, ce->c_alias, strlen(argp)) == 0) {
			    break;
			}
		    }
		}
	    }

	    /*
	     * See if the command was found.
	     */
	    if (ce->c_name == (caddr_t) 0) {
		(*pc->pc_fprintf) ("Command '%s' was not recognized.", argp);
		return (PC$_FAILURE);
	    }

	    pc->pc_ce = ce;
	    SET_COPT (pc, (int) ce->c_token);
	    CLR_KOPTS (ce);

	    /*
	     * Parse the type field (if any).
	     */
	    if (ce->c_type != TYPE_NONE) {
		if ( (status = ParseType (pc, ce)) == PC$_FAILURE) {
		    return (status);
		}
	    }

	    if (pc->pc_keyc = ce->c_keyc) {
		if (pc->pc_argc) {
		    if ( (status = ParseKeywords (pc, ce->c_keyp)) != PC$_SUCCESS) {
			return (status);
		    }
		} else if (!(ce->c_flags & PF_KEYS_OPT)) {
		    return (MoreKeys (pc, ce, NULL, 0));
		}
	    }

	    /*
	     * The parser control function allows external control after
	     * the command is parsed successfully, but before any command
	     * function is invoked to permit additional command checking.
	     */
	    if (ce->c_flags & PF_CONTROL) {
		if ( (status = (*pc->pc_func)(ce, NULL)) == PC$_CONTINUE) {
		    continue;
		} else if (status == PC$_FAILURE) {
		    return (status);
		}
	    }

	    if (ce->c_func != (int (*)()) 0) {
		/*
		 * If the entry is marked as command default, then only
		 * invoke this function if no keywords were specified.
		 * This logic allows keyword action routines to override
		 * default command processing.
		 */
		if ( (ce->c_flags & PF_DEFAULT) && ANY_KOPTS(ce) ) {
		    continue;
		}
		if ( (status = (*ce->c_func)(ce, NULL)) != PC$_SUCCESS) {
		    return (status);
		}
	    }

	} /* End of 'while (pc->pc_argc) {' */

	return (PC$_SUCCESS);
}

/************************************************************************
 *									*
 * ParseKeywords() - Parse Command Keywords.				*
 *									*
 * Inputs:	pc = Pointer to parse control structure.		*
 *		kp = Pointer to keyword table.				*
 *									*
 * Return Value:							*
 *		Returns 0 / 1 / -1 = SUCCESS / CONTINUE / FAILURE.	*
 *									*
 ************************************************************************/
static int
ParseKeywords (pc, kp)
register struct parser_control *pc;
struct key_entry *kp;
{
	register struct cmd_entry *ce = pc->pc_ce;
	register struct key_entry *ke;
	register char *argp;
	register int status;

	/*
	 * Decode the Keywords:
	 */
 	while (pc->pc_argc && pc->pc_keyc) {

	    pc->pc_argc--;		/* Adjust the argument count.	*/
	    argp = *pc->pc_argv++;	/* Parse the next argument.	*/

	    /*
	     * Decode the keywords:
	     */
	    for (ke = kp; ke->k_name; ke++) {
		/*
		 * Parse control tokens so keywords are not necessary.
		 */
		if ( (long) ke->k_name < (long) PC$_END_CONTROL ) {
		    status = (ParseControl (pc, ce, ke, argp));
		    switch (status) {
			case PC$_CONTINUE:
			    if (ke->k_keyp) {
				goto doparse;	/* Parse sub-table... */
			    }
			    /*FALLTHROUGH*/
			default:
			    return (status);
		    }
		}
		if ( (pc->pc_flags & PC_EXACT) || (ke->k_flags & PF_EXACT) ) {
		    if (strcmp (argp, ke->k_name) == 0) {
			break;
		    } else if (ke->k_alias) {
			if (strcmp (argp, ke->k_alias) == 0) {
			    break;
			}
		    }
		} else {
		    if (strncmp (argp, ke->k_name, strlen(argp)) == 0) {
			break;
		    } else if (ke->k_alias) {
			if (strncmp (argp, ke->k_alias, strlen(argp)) == 0) {
			    break;
			}
		    }
		}
	    }

	    /*
	     * See if the keyword was found.
	     */
	    if (ke->k_name == (caddr_t) 0) {
		(*pc->pc_fprintf) ("Command '%s', keyword '%s' is invalid.",
							ce->c_name, argp);
		return (PC$_FAILURE);
	    }

	    pc->pc_keyc--;		/* Adjust the keyword count.	*/

	    /*
	     * Parse the keyword type (if any).
	     */
	    if (ke->k_type != TYPE_NONE) {
		if ( (status = ParseType (pc, ke)) == PC$_FAILURE) {
		    return (status);
		}
	    }

	    SET_KOPT (ce, (int) ke->k_token);

	    /*
	     * Parse multiple keyword tables.
	     */
doparse:    if ( (status = ParseKeyTable (pc, ce, ke)) != PC$_SUCCESS) {
		return (status);
	    }

	    /*
	     * The parser control function allows external control after
	     * the keyword is parsed successfully, but before any keyword
	     * function is invoked to permit additional command checking.
	     */
	    if (ke->k_flags & PF_CONTROL) {
		if ( (status = (*pc->pc_func)(ce, ke)) == PC$_CONTINUE) {
		    continue;
		} else if (status == PC$_FAILURE) {
		    return (status);
		}
	    }

	    if (ke->k_func != (int (*)()) 0) {
		if ( (status = (*ke->k_func)(ce, ke)) != PC$_SUCCESS) {
		    return (status);
		}
	    }

	} /* End 'while (pc->pc_argc && pc->pc_keyc) {' */

	return (PC$_SUCCESS);
}

/************************************************************************
 *									*
 * ParseKeyTable() - Parse The Next Keyword Table.			*
 *									*
 * Inputs:	pc = The parse control structure.			*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns 0 / 1 / -1 = SUCCESS / CONTINUE / FAILURE.	*
 *									*
 ************************************************************************/
static int
ParseKeyTable (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	int status = PC$_SUCCESS;

	if (pc->pc_argc && ke->k_keyp) {
	    register int p_keyc = pc->pc_keyc;

	    /*
	     * Sanity check to ensure keytable count is setup.
	     */
	    if ((pc->pc_keyc = ke->k_keyc) == 0) {
		return (NeedKeyc (pc, ce, ke));
	    }

	    /*
	     * Parse keywords in this table.
	     */
	    if ( (status = ParseKeywords (pc, ke->k_keyp)) == PC$_FAILURE) {
		return (status);
	    }

	    /*
	     * See if more keywords are expected or are optional.
	     *
	    if ( pc->pc_keyc && (status != PC$_CONTINUE) &&
		((ke->k_flags & PF_KEYS_OPT) == 0) ) {
		return (MoreKeys (pc, ce, ke, pc->pc_keyc));
	    }

	    /*
	     * For optional keyword tables, then map status to success.
	     * This allows additional entrys in the current keyword table
	     * to get parsed.  The exception to this are control entrys,
	     * which only get mapped to success if a keyword was parsed.
	     */
	    if (status == PC$_CONTINUE) {
		if (pc->pc_keyc != ke->k_keyc) {
		    status = PC$_SUCCESS;
		} else if ( (long) ke->k_name > (long) PC$_END_CONTROL ) {
		    status = PC$_SUCCESS;
		}
	    }

	    /*
	     * Adjust previous keyword count based on keywords parsed.
	     */
	    pc->pc_keyc = p_keyc - (ke->k_keyc - pc->pc_keyc);
	    if (pc->pc_keyc < 0) pc->pc_keyc = 0;
	} else if (ke->k_keyc && !(ke->k_flags & PF_KEYS_OPT)) {
	    return (MoreKeys (pc, ce, ke, ke->k_keyc));
	}
	return (status);
}

/************************************************************************
 *									*
 * ParseControl() - Parse The Control Token Field.			*
 *									*
 * Inputs:	pc = The parse control structure.			*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		argp = The argument pointer.				*
 *									*
 * Return Value:							*
 *		Returns 0 / 1 / -1 = SUCCESS / CONTINUE / FAILURE.	*
 *									*
 ************************************************************************/
/*ARGSUSED*/
static int
ParseControl (pc, ce, ke, argp)
register struct parser_control *pc;
struct cmd_entry *ce;
register struct key_entry *ke;
register caddr_t argp;
{
	int status;

	switch ((long) ke->k_name) {

	    case PC$_END_OF_TABLE:
		status = PC$_FAILURE;	/* End of parser table.		*/
		break;

	    case PC$_LAMBDA:
	    case PC$_SHARED_TABLE:
		pc->pc_argc++;		/* Adjust the argument count.	*/
		pc->pc_argv--;		/* Adjust the argument pointer.	*/
		status = PC$_CONTINUE;
		break;

	    default:
		(*pc->pc_fprintf) (
		    "Invalid control token (%d), parsing keyword '%s'.",
				    		GetKeyName(ke), argp);
		abort();
		/*NOTREACHED*/
	}
	return (status);
}

/************************************************************************
 *									*
 * ParseBoolean() - Parse Boolean String Parameter.			*
 *									*
 * Inputs:	str = Pointer to the boolean string.			*
 *		argp = Pointer to boolean variable.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
ParseBoolean (str, argp)
register char *str;
u_long *argp;
{
	int status = PC$_SUCCESS;

	if ( (strncasecmp (str, "on", strlen(str)) == 0) ||
	     (strncasecmp (str, "true", strlen(str)) == 0) ||
	     (strncasecmp (str, "1", strlen(str)) == 0) ) {
	    *argp = PC$_TRUE;
	} else if ( (strncasecmp (str, "off", strlen(str)) == 0) ||
		    (strncasecmp (str, "false", strlen(str)) == 0) ||
		    (strncasecmp (str, "0", strlen(str)) == 0) ) {
	    *argp = PC$_FALSE;
	} else {
	    (*Pc->pc_fprintf) (
	"String '%s' is invalid for a boolean string parameter...", str);
	    (*Pc->pc_fprintf) (
	"Valid parameters are: '{ on | true | 1 }' or '{ off | false | 0 }'");
	    status = PC$_FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * ParseDate() - Parse The Date String to an Internal Format.		*
 *									*
 * Inputs:	str = Pointer to the date string.			*
 *		date = Pointer to internal date buffer.			*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
ParseDate (str, date)
register char *str;
register struct parser_date *date;
{
	register struct month_entry *me;
	char month[3+1];
	register int i;
	struct tm tm_info;
	register struct tm *tmp = &tm_info;
	int count;

	(void) bzero ((char *) date, sizeof(*date));
	(void) time (&date->time);
	/*
	 * Allow the special keywords "today" or "yesterday" for dates.
	 */
	if ( (strncasecmp (str, "today", strlen(str)) == 0) ||
	     (strncasecmp (str, "yesterday", strlen(str)) == 0) ) {
	    tmp = localtime (&date->time);
	    tmp->tm_hour = tmp->tm_min = tmp->tm_sec = 0;
	    if ( (date->time = mktime (tmp)) == -1) {
		(*Pc->pc_fprintf) ("Failed to setup today's date.");
		return (PC$_FAILURE);
	    }
	    if (strncasecmp (str, "yesterday", strlen(str)) == 0) {
		date->time -= SECS_PER_DAY;
	    }
	    return (PC$_SUCCESS);
	}

	/*
	 * Check for empty date fields.
	 */
	if (strncmp (str, "   ", 3) == 0) {
	    return (PC$_SUCCESS);
	}

	/*
	 * Decode date string in form of: 'dd-mmm-yyyy'
	 */
	count = sscanf (str, "%2d%*1c%3s%*1c%4d",
						&date->day,
						month,
						&date->year);
	if (count != 3) {
	    (*Pc->pc_fprintf) (
		"Failed to convert date string of '%s'", str);
	    return (PC$_FAILURE);
	}

	/*
	 * Do case insensitive compare ('finq' inserts lowercase).
	 */
	for (i = 1; i < (int) NUMBER_OF_MONTHS; i++) {
	    me = &MonthTable[i];
	    if (strncasecmp (month, me->month_name, 3) == 0) {
		break;
	    }
	}

	if (i == (int) NUMBER_OF_MONTHS) {
	    (*Pc->pc_fprintf) (
		"Failed to locate valid month in string '%s'", month);
	    return (PC$_FAILURE);
	};

	date->month = me->month;
	if (date->year < TM_YEAR_BASE) {
	    date->year += TM_YEAR_BASE;
	}

	/*
	 * Convert the date into the internal time format.
	 */
	(void) bzero ((char *) tmp, sizeof(*tmp));
	tmp->tm_mday = date->day;
	tmp->tm_mon = (int) date->month - 1;
	tmp->tm_year = (int) date->year - TM_YEAR_BASE;
	if ( (date->time = mktime (tmp)) == -1) {
	    (*Pc->pc_fprintf) (
		"Failed to convert date to internal format using '%d/%d/%d'",
					date->month, date->day, date->year);
	    return (PC$_FAILURE);
	}
	return (PC$_SUCCESS);
}

/************************************************************************
 *									*
 * ParseType() - Parse The Type Field (Command/Keyword Arguments).	*
 *									*
 * Inputs:	pc = Pointer to parse control structure.		*
 *		pe = Pointer to parser entry (command or keyword).	*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
static int
ParseType (pc, pe)
register struct parser_control *pc;
register struct parser_entry *pe;
{
	register struct cmd_entry *ce = pc->pc_ce;
	register int status = PC$_SUCCESS;
	int base;

	switch (pe->p_type) {

	    case TYPE_NONE:
	    case TYPE_ACTION:
		break;

	    case TYPE_BOOLEAN:
		if (pc->pc_argc) {
		    status = ParseBoolean (*pc->pc_argv, (u_long) pe->p_argp);
		    pc->pc_argc--;
		    pc->pc_argv++;
		} else {
		    if (!(pe->p_flags & PF_ARGS_OPT)) {
			return (NeedBoolean (pc, ce, pe));
		    }
		}
		break;

	    case TYPE_CLEAR_BITS:
		*(u_long *)pe->p_argp &= ~(u_long) pe->p_default;
		break;

	    case TYPE_SET_FLAG:
		*(u_long *)pe->p_argp = (u_long) pe->p_default;
		break;

	    case TYPE_SET_BITS:
		*(u_long *)pe->p_argp |= (u_long) pe->p_default;
		break;

	    case TYPE_DATE:
		if (pc->pc_argc) {
		    status = ParseDate (*pc->pc_argv, pe->p_argp);
		    pc->pc_argc--;
		    pc->pc_argv++;
		} else {
		    if (!(pe->p_flags & PF_ARGS_OPT)) {
			return (NeedDate (pc, ce, pe));
		    }
		}
		break;

	    case TYPE_STRING:
		if (pe->p_flags & PF_DYNAMIC) {
		    char *str = (char *) *(long *)pe->p_argp;
		    if (str != NULL) {
			(void) free (str);
			*(long *)pe->p_argp = NULL;
		    }
		}
		if (pc->pc_argc) {
		    char *str = *pc->pc_argv;
		    if (pe->p_flags & PF_DYNAMIC) {
			char *bufptr = malloc (strlen(str) + 1);
			(void) strcpy (bufptr, str);
			*(long *)pe->p_argp = (long) bufptr;
		    } else {
			*(long *)pe->p_argp = (long) str;
		    }
		    pc->pc_argc--;
		    pc->pc_argv++;
		} else {
		    if (!(pe->p_flags & PF_ARGS_OPT)) {
			return (NeedString (pc, ce, pe));
		    } else {
			*(long *)pe->p_argp = NULL;
		    }
		}
		break;

	    case TYPE_LOGICAL:
		base = 2; goto doconv;
	    case TYPE_DECIMAL:
		base = 10; goto doconv;
	    case TYPE_OCTAL:
		base = 8; goto doconv;
	    case TYPE_HEX:
		base = 16; goto doconv;
	    case TYPE_VALUE:
		base = 0;
doconv:
		if (pc->pc_argc &&
		    ( ( isdigit (*pc->pc_argv[0]) ) ||
		      ( pe->p_flags & PF_SPECIAL) ) ) {
		    char *eptr;
		    if (pe->p_flags & PF_SPECIAL) {
			*(u_long *)pe->p_argp = 
			    (*pc->pc_special) (*pc->pc_argv, &eptr, base);
		    } else {
			*(u_long *)pe->p_argp =
				strtoul (*pc->pc_argv, &eptr, base);
		    }
		    if (*eptr != '\0') {
			(*pc->pc_fprintf) (
			"Invalid character detected in number: '%c'", *eptr);
			return (PC$_FAILURE);
		    }
		    pc->pc_argc--;
		    pc->pc_argv++;
		    if (pe->p_flags & PF_RANGE) {
			if (status = CheckRange (pc, ce, pe)) {
			    return (status);
			}
		    }
		} else {
		    if (!(pe->p_flags & PF_ARGS_OPT)) {
			return (NeedValue (pc, ce, pe));
		    }
		}
		break;

	    default:
		if (ce == pe) {		/* Parsing command argument.	*/
		    (*pc->pc_fprintf) (
			"Command '%s' was not parsed properly.", ce->c_name);
		} else {
		    (*pc->pc_fprintf) (
			"Command '%s', keyword '%s' was not parsed properly.",
						ce->c_name, pe->p_name);
		}
		return (PC$_FAILURE);
	}
	return (status);
}

/************************************************************************
 *									*
 * PromptUser() - Prompt User for Specified Data Type.			*
 *									*
 * Inputs:	pc = Pointer to parse control structure.		*
 *		type = The input data type.				*
 *		prompt = Pointer to prompt string.			*
 *		prchar = The prompt character.				*
 *		deflt = The default parameter.				*
 *		argp = The argument pointer.				*
 *		flags = Parser control flags.				*
 *		optp = Optional parameter.				*
 *									*
 * Return Value:							*
 *		Returns 0 / -1 = SUCCESS / FAILURE.			*
 *									*
 ************************************************************************/
int
PromptUser (pc, type, prompt, prchar, deflt, argp, flags, optp)
register struct parser_control *pc;
enum parser_types type;
int flags;
char *prompt, *deflt, *argp, *optp;
char prchar;
{
	struct parser_range *range;
	char input_buffer[PC_INPUT_BUFFER_SIZE];
	register char *inptr = input_buffer;
	char default_str[PC_DEFAULT_STR_SIZE];
	register char *defptr = default_str;
	register char *fmtptr = (char *) 0;
	int base;
	size_t inlen;
	u_long value = 0;
	int status = PC$_SUCCESS;

	(void) bzero (inptr, sizeof(input_buffer));
	(void) bzero (defptr, sizeof(default_str));

	if (flags & PF_RANGE) {
	    range = (struct parser_range *) optp;
	}

	/*
	 * Prepare the default string.
	 */
	switch (type) {

	    case TYPE_SET_FLAG:
	    case TYPE_SET_BITS:
	    case TYPE_CLEAR_BITS:
		if (deflt) {
		    (void) sprintf (defptr, "[D:%#lx]", *(u_long *) deflt);
		}
		break;

	    case TYPE_DATE:
	    case TYPE_BOOLEAN:
	    case TYPE_STRING:
		if (deflt) {
		    (void) sprintf (defptr, "[D:\"%s\"]", deflt);
		}
		break;

	    case TYPE_DECIMAL:
	    case TYPE_VALUE:
		if (deflt) {
		    if (flags & PF_RANGE) {
			fmtptr = "[R:%lu-%lu D:%lu]";
		    } else {
			fmtptr = "[D:%lu]";
		    }
		} else if (flags & PF_RANGE) {
		    fmtptr = "[R:%lu-%lu]";
		}
		goto dofmt;

	    case TYPE_OCTAL:
		if (deflt) {
		    if (flags & PF_RANGE) {
			fmtptr = "[R:%#lo-%#lo D:%#lo]";
		    } else {
			fmtptr = "[D:%#lo]";
		    }
		} else if (flags & PF_RANGE) {
		    fmtptr = "[R:%#lo-%#lo]";
		}
		goto dofmt;

	    case TYPE_HEX:
		if (deflt) {
		    if (flags & PF_RANGE) {
			fmtptr = "[R:%#lx-%#lx D:%#lx]";
		    } else {
			fmtptr = "[D:%#lx]";
		    }
		} else if (flags & PF_RANGE) {
		    fmtptr = "[R:%#lx-%#lx]";
		}
dofmt:
		if (deflt) {
		    if (flags & PF_RANGE) {
			(void) sprintf (defptr, fmtptr,
					range->pr_min, range->pr_max,
					*(u_long *) deflt);
		    } else {
			(void) sprintf (defptr, fmtptr, *(u_long *) deflt);
		    }
		} else if (flags & PF_RANGE) {
		    (void) sprintf (defptr, fmtptr,
					range->pr_min, range->pr_max);
		}
		break;

	    default:
		break;
	}

	/*
	 * Prompt the user.
	 */
reprompt:
	(*pc->pc_printf)("%s %s%c ", prompt, defptr, prchar);
	if ((*pc->pc_gets)(inptr, sizeof(input_buffer)) == NULL) {
	    if (pc->pc_flags & PC_INTERACTIVE) {
		(*pc->pc_printf)("\n");
	    }
	    return (PC$_FAILURE);
	}
	if ( ((inlen = strlen (inptr)) == 0) &&
	     (deflt == (char *) 0) ) {
		goto reprompt;
	}

	/*
	 * Decode the parameter entered.
	 */
	switch (type) {

	    case TYPE_SET_FLAG:
		if (inlen) {
		    *(u_long *) argp = strtoul (inptr, (char **)NULL, 0);
		} else {
		    *(u_long *) argp = *(u_long *) deflt;
		}
		break;

	    case TYPE_SET_BITS:
		if (inlen) {
		    *(u_long *) argp |= strtoul (inptr, (char **)NULL, 0);
		} else {
		    *(u_long *) argp |= *(u_long *) deflt;
		}
		break;

	    case TYPE_CLEAR_BITS:
		if (inlen) {
		    *(u_long *) argp &= ~strtoul (inptr, (char **)NULL, 0);
		} else {
		    *(u_long *) argp &= ~*(u_long *) deflt;
		}
		break;

	    case TYPE_BOOLEAN: {
		status = ParseBoolean ((inlen) ? inptr : deflt, argp);
		if (status == PC$_FAILURE) {
		    goto reprompt;
		}
		break;
	    }
	    case TYPE_DATE: {
		status = ParseDate ((inlen) ? inptr : deflt, argp);
		if (status == PC$_FAILURE) {
		    goto reprompt;
		}
		break;
	    }
	    case TYPE_STRING: {
		if (inlen) {
		    char *bufptr = malloc (strlen(inptr) + 1);
		    strcpy (bufptr, inptr);
		    *(long *)argp = (long) bufptr;
		} else {
		    *(char **)argp = deflt;
		}
		break;
	    }
	    case TYPE_LOGICAL:
		base = 2; goto doconv;
	    case TYPE_DECIMAL:
		base = 10; goto doconv;
	    case TYPE_OCTAL:
		base = 8; goto doconv;
	    case TYPE_HEX:
		base = 16; goto doconv;
	    case TYPE_VALUE:
		base = 0;
doconv:
		if (inlen) {
		    value = strtoul (inptr, (char **)NULL, base);
		    if ( (flags & PF_RANGE) &&
			 ( (value < range->pr_min) ||
			   (value > range->pr_max) ) ) {
			(*pc->pc_fprintf) ("Value is not within range.");
			goto reprompt;
		    }
		    *(u_long *) argp = value;
		} else {
		    *(u_long *) argp = *(u_long *) deflt;
		}
		break;

	    default:
		status = PC$_FAILURE;
		break;
	}
	return (status);
}

/************************************************************************
 *									*
 * MoreKeys()	Report More Keywords Needed Parsing.			*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *		keyc = The remaining keyword count.			*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
MoreKeys (pc, ce, ke, keyc)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
int keyc;
{
	if (ke == (struct key_entry *) 0) {
	    (*pc->pc_fprintf) (
		"Command '%s' requires at least %d keyword%s.",
						ce->c_name, ce->c_keyc,
						(ce->c_keyc == 1) ? "" : "s");
	    if (ce->c_keyc - pc->pc_keyc) {
		(*pc->pc_fprintf) ("Only %d keywords were parsed correctly.",
						(ce->c_keyc - pc->pc_keyc));
	    }
	} else {
	    (*pc->pc_fprintf) (
		"Command '%s', keyword '%s' requires %d more keyword%s.",
					ce->c_name, GetKeyName(ke), keyc,
					(keyc == 1) ? "" : "s");
	}
	return (PC$_FAILURE);
}

/************************************************************************
 *									*
 * NeedBoolean() - Report Boolean String Parameter Needed.		*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
NeedBoolean (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	if (ce == ke) {
	    (*pc->pc_fprintf) (
	"Command '%s' requires a boolean string parameter.", ce->c_name);
	} else {
	    (*pc->pc_fprintf) (
	"Command '%s', keyword '%s' requires a boolean string parameter.",
						ce->c_name, GetKeyName(ke));
	}
	return (PC$_FAILURE);
}

#ifdef notdef
/************************************************************************
 *									*
 * NeedFlag()	Report Flag Needed for Command.				*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
NeedFlag (pc, ce)
register struct parser_control *pc;
register struct cmd_entry *ce;
{
	(*pc->pc_fprintf) ("Command '%s' requires a flag parameter.",
							ce->c_name);
	return (PC$_FAILURE);
}
#endif notdef

/************************************************************************
 *									*
 * NeedKeyc()	Report Key Count Needed for Parsing.			*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
NeedKeyc (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	(*pc->pc_fprintf) (
		"Command '%s', keyword '%s' needs a keyword count.",
						ce->c_name, GetKeyName(ke));
	return (PC$_FAILURE);
}

/************************************************************************
 *									*
 * NeedString()	Report String Needed for Command.			*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
NeedString (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	if (ce == ke) {
	    (*pc->pc_fprintf) (
		"Command '%s' requires a string parameter.", ce->c_name);
	} else {
	    (*pc->pc_fprintf) (
		"Command '%s', keyword '%s' requires a string parameter.",
						ce->c_name, GetKeyName(ke));
	}
	return (PC$_FAILURE);
}

/************************************************************************
 *									*
 * NeedDate()	Report Date String Needed for Command.			*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
NeedDate (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	if (ce == ke) {
	    (*pc->pc_fprintf) (
	"Command '%s' requires a date string (dd-mmm-yyyy).", ce->c_name);
	} else {
	    (*pc->pc_fprintf) (
	"Command '%s', keyword '%s' requires a date string (dd-mmm-yyyy).",
						ce->c_name, GetKeyName(ke));
	}
	return (PC$_FAILURE);
}

/************************************************************************
 *									*
 * NeedValue()	Report Value Needed for Command.			*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value: Returns FAILURE status code.				*
 *									*
 ************************************************************************/
static int
NeedValue (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	if (ce == ke) {
	    (*pc->pc_fprintf) (
		"Command '%s' requires a numeric value.", ce->c_name);
	} else {
	    (*pc->pc_fprintf) (
	    "Command '%s', keyword '%s' requires a numeric value.",
						ce->c_name, GetKeyName(ke));
	}
	return (PC$_FAILURE);
}

/************************************************************************
 *									*
 * CheckRange()	Check Value For Within Range Fields.			*
 *									*
 * Inputs:	pc = The parse control block.				*
 *		ce = The command table entry.				*
 *		ke = The keyword table entry.				*
 *									*
 * Return Value: Returns SUCCESS/FAILURE = In Range/Out of Range.	*
 *									*
 ************************************************************************/
static int
CheckRange (pc, ce, ke)
register struct parser_control *pc;
register struct cmd_entry *ce;
register struct key_entry *ke;
{
	register int value;
	int status = PC$_SUCCESS;

	value = (int) *(long *)ke->k_argp;
	if ( (value < ke->k_min) || (value > ke->k_max) ) {
	    (*pc->pc_fprintf) (
			"%s '%s' requires range between (%d - %d).",
			(ce == ke) ? "Command" : "Keyword",
			GetKeyName(ke), ke->k_min, ke->k_max);
	    status = PC$_FAILURE;
	}
	return (status);
}

/************************************************************************
 *									*
 * GetKeyName() - Get Key Name to Display on Parse Error Messages.	*
 *									*
 * Description:								*
 *	This function ensures a valid key name string is available for	*
 * displaying on parsing error messages.  This is necessary, since	*
 * parse control tokens which may appear in the keyword name field is	*
 * not a valid string pointer.						*
 *									*
 * Inputs:	ke = The keyword table entry.				*
 *									*
 * Return Value:							*
 *		Returns pointer to key name string to display.		*
 *									*
 ************************************************************************/
static char *
GetKeyName (ke)
register struct key_entry *ke;
{
	char *ke_name;

	if ( (long) ke->k_name < (long) PC$_END_CONTROL ) {
	    if ( (ke_name = ke->k_msgp) == NULL) {
		ke_name = "?";
	    }
	} else {
	    ke_name = ke->k_name;
	}
	return (ke_name);
}

/************************************************************************
 *									*
 * FindCmdEntry() - Find Command Table Entry.				*
 *									*
 * Inputs:	cmd_token = The command token.				*
 *									*
 * Return Value:							*
 *		Returns the command table entry or 0 if not found.	*
 *									*
 ************************************************************************/
struct cmd_entry *
FindCmdEntry (cmd_token)
register enum parser_tokens cmd_token;
{
	register struct cmd_entry *ce;

	/*
	 * Search the command table, looking for a match.
	 */
	for (ce = Pc->pc_cp; ce->c_name; ce++) {
		if (ce->c_token == cmd_token) {
			return (ce);
		}
	}
	return ((struct cmd_entry *) 0);
}

/************************************************************************
 *									*
 * FindKeyEntry() - Find Keyword Table Entry.				*
 *									*
 * Inputs:	ce = The command entry (NULL if none).			*
 *		cmd_token = The command token.				*
 *		key_token = The keyword token.				*
 *									*
 * Return Value:							*
 *		Returns the keyword table entry or 0 if not found.	*
 *									*
 ************************************************************************/
struct key_entry *
FindKeyEntry (ce, cmd_token, key_token)
register struct cmd_entry *ce;
register enum parser_tokens cmd_token;
register enum parser_tokens key_token;
{
	register struct key_entry *ke;

	if (ce == (struct cmd_entry *) 0) {
	    if ((ce = FindCmdEntry (cmd_token)) == (struct cmd_entry *) 0) {
		return ((struct key_entry *) 0);
	    }
	}

	/*
	 * Search the keyword table, matching on the token.
	 */
	for (ke = ce->c_keyp; ke->k_name; ke++) {
	    if (ke->k_token == key_token) {
		return (ke);
	    } else if (ke->k_keyp) {
		register struct key_entry *kp;
		/*
		 * Check for recursive call to same keyword table.
		 */
		if (ce->c_keyp == ke->k_keyp) continue;
		/*
		 * This recursive call is possible since the command
		 * and keyword entries are now the same structure.
		 */
		if ( (kp = FindKeyEntry (ke, 0, key_token)) != NULL) {
		    return (kp);
		}
	    }
	}
	return ((struct key_entry *) 0);
}
