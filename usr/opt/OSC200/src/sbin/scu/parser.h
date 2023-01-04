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
 * @(#)$RCSfile: parser.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/04/14 14:46:54 $
 */
/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any	other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived	from  software	received  from	the	*
 *   University    of	California,   Berkeley,   and	from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is	subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or	reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 * File:	parser.h
 * Author:	Robin T. Miller
 * Date:	August 9, 1991
 *
 * Description:
 *	Common include file used for parser functions.
 *
 * Modification History:
 *
 */
#include <sys/types.h>
#include "tokens.h"

/*
 * Define Parser Return Codes:
 */
#define PC$_SUCCESS	0		/* Successfully parsed args.	*/
#define PC$_FAILURE	-1		/* Failure, invalid arguments.	*/
#define PC$_WARNING	1		/* Warning, optional arguments.	*/
#define PC$_CONTINUE	PC$_WARNING	/* Continue parsing keywords.	*/
#define PC$_TRUE	1		/* Boolean TRUE value.		*/
#define PC$_FALSE	0		/* Boolean FALSE value.		*/

#define COPT_ARRAY	2		/* Command option array size.	*/
#define KOPT_ARRAY	8		/* Keyword option array size.	*/
#define NOPTBITS	(sizeof(long) * NBBY)

/*
 * Define Macros for Setting & Testing Options.
 */
#define SET_COPT(p,n)	\
		p->pc_copts[(n)/NOPTBITS] |= (1L << ((n) % NOPTBITS))
#define CLR_COPTS(p)							\
	{	int i = 0;						\
		if ( (C_LAST_TOKEN/NOPTBITS) >= COPT_ARRAY) {		\
		    (*Pc->pc_fprintf)					\
	("Command option array (%d) is too small for last token (%d).",	\
					COPT_ARRAY, C_LAST_TOKEN);	\
		    exit (PC$_FAILURE);					\
		}							\
		while (i < COPT_ARRAY) {				\
			p->pc_copts[i++] = 0;				\
		}							\
	}
#define ISCLR_COPT(p,n)	\
		((p->pc_copts[(n)/NOPTBITS] & (1L << ((n) % NOPTBITS))) == 0L)
#define ISSET_COPT(p,n)	\
		(p->pc_copts[(n)/NOPTBITS] & (1L << ((n) % NOPTBITS)))

#define SET_KOPT(c,n)	\
		c->c_kopts[(n)/NOPTBITS] |= (1L << ((n) % NOPTBITS))
#define ANY_KOPTS(c)	\
		(c->c_kopts[0] || c->c_kopts[1] || c->c_kopts[2] ||	\
		 c->c_kopts[3] || c->c_kopts[4] || c->c_kopts[5] ||	\
		 c->c_kopts[6] || c->c_kopts[7])
#define CLR_KOPTS(p)							\
	{	int i = 0;						\
		if ( (K_LAST_TOKEN/NOPTBITS) >= KOPT_ARRAY) {		\
		    (*Pc->pc_fprintf)					\
	("Keyword option array (%d) is too small for last token (%d).",	\
					KOPT_ARRAY, K_LAST_TOKEN);	\
		    exit (PC$_FAILURE);					\
		}							\
		while (i < KOPT_ARRAY) {				\
			p->c_kopts[i++] = 0;				\
		}							\
	}
#define ISCLR_KOPT(c,n)	\
		((c->c_kopts[(n)/NOPTBITS] & (1L << ((n) % NOPTBITS))) == 0L)
#define ISSET_KOPT(c,n)	\
		(c->c_kopts[(n)/NOPTBITS] & (1L << ((n) % NOPTBITS)))

/*
 * Since there was so much commonality between the command and keyword
 * entrys, I decided to make a common parser entry.
 */
#define cmd_entry	parser_entry	/* Define the command entry.	*/
#define key_entry	parser_entry	/* Define the keyword entry.	*/

/*
 * Define Control Parsing Tokens:
 */
enum control_tokens {
	PC$_END_OF_TABLE,		/* End of parser table.		*/
	PC$_LAMBDA,			/* Always match (optional).	*/
	PC$_SHARED_TABLE,		/* Parse shared keyword table.	*/
	PC$_END_CONTROL			/* Last token for range check.	*/
};

/*
 * Define Parser Control Flags:
 */
#define PC_EXACT		0x01	/* Exact match of table entrys.	*/
#define PC_INTERACTIVE		0x02	/* Program in interactive mode.	*/

#define PC_DEFAULT_STR_SIZE	64	/* The default prompt size.	*/
#define PC_INPUT_BUFFER_SIZE	256	/* The input buffer size.	*/

/*
 * Define Parser Control Structure:
 */
typedef struct parser_control {
	int	pc_flags;		/* Parser control flags.	*/
	caddr_t	pc_name;		/* Program name for errors.	*/
	int	pc_argc;		/* Argument count.		*/
	char	**pc_argv;		/* Argument pointer array.	*/
	u_long	pc_copts[COPT_ARRAY];	/* Command options flag array.	*/
	int	pc_keyc;		/* Working keyword count.	*/
	struct cmd_entry *pc_cp;	/* The command table pointer.	*/
	struct cmd_entry *pc_ce;	/* The current command entry.	*/
	int	(*pc_func)();		/* Parser control function.	*/
	char	(*pc_gets)();		/* Function to get a string.	*/
	char	*pc_inpbufptr;		/* The input buffer pointer.	*/
	int	pc_inpbufsiz;		/* The input buffer size.	*/
	int	(*pc_printf)();		/* Function to perform printf.	*/
	int	(*pc_fprintf)();	/* Function to perform fprintf.	*/
	int	(*pc_special)();	/* Function for special parsing	*/
} PARSER_CONTROL;

/*
 * Define the Date Month Values:
 */
enum months { MONTH_NONE,
	MONTH_JAN, MONTH_FEB, MONTH_MAR, MONTH_APR, MONTH_MAY, MONTH_JUN,
	MONTH_JUL, MONTH_AUG, MONTH_SEP, MONTH_OCT, MONTH_NOV, MONTH_DEC,
	NUMBER_OF_MONTHS
};

/*
 * Define the Month Table Entry format:
 */
typedef struct month_entry {
	enum months month;		/* Internal month designator.	*/
	char	*month_name;		/* The short month name.	*/
	char	*month_full;		/* The full month name.		*/
} MONTH_ENTRY;

/*
 * Define Internal Date Structure:
 */
typedef struct parser_date {
	time_t	time;			/* The internal time value.	*/
	int	day;			/* The day of the month.	*/
	enum months month;		/* The month of the year.	*/
	int	year;			/* The year of the century.	*/
} PARSER_DATE;

/*
 * Define Parser Types:
 */
enum parser_types {
	TYPE_NONE,			/* No additional parameters.	*/
	TYPE_ACTION,			/* Performs an action.		*/
	TYPE_STRING,			/* String variable.		*/
	TYPE_DATE,			/* Date string.			*/
	TYPE_VALUE,			/* Numeric value.		*/
	TYPE_LOGICAL,			/* Logical value (0 or 1).	*/
	TYPE_DECIMAL,			/* Decimal number.		*/
	TYPE_OCTAL,			/* Octal number.		*/
	TYPE_HEX,			/* Hexidecimal number.		*/
	TYPE_SET_FLAG,			/* Set flag to k_value.		*/
	TYPE_SET_BITS,			/* Set bits in a flag.		*/
	TYPE_CLEAR_BITS,		/* Clear bits in a flag.	*/
	TYPE_BOOLEAN,			/* Set boolean flag (0 / 1).	*/
	NUMBER_OF_TYPES
};

/*
 * Define Parser Entry Control Flags:
 */
#define PF_ARGS_OPT		0x01	/* Argument(s) are optional.	*/
#define PF_KEYS_OPT		0x02	/* Keyword(s) are optional.	*/
#define PF_RANGE		0x04	/* Check value is within range.	*/
#define PF_DEFAULT		0x08	/* Do command default action.	*/
#define PF_EXACT		0x10	/* Exact match of table entrys.	*/
#define PF_CONTROL		0x20	/* Use parser control function.	*/
#define PF_DYNAMIC		0x40	/* Dynamically allocate space.	*/
#define PF_SPECIAL		0x80	/* Use special parser function.	*/

/*
 * Parser Control Function Flags:
 */
#define PCF_PARSE_ARGS		0x01	/* Parse additional arguments.	*/
#define PCF_QUEUE_HEAD		0x02	/* Position entry @ head of Q.	*/

/*
 * Define Parser Range Structure:
 */
typedef struct parser_range {
	u_long	pr_min;			/* Range: Minimum value.	*/
	u_long	pr_max;			/* Range: Maximum value.	*/
} PARSER_RANGE;

/*
 * Define Parser Entrys:
 */
typedef struct parser_entry {
	caddr_t	p_name;			/* The ASCII command name.	*/
	caddr_t	p_alias;		/* The command alias (if any).	*/
	enum parser_tokens p_token;	/* The internal command token.	*/
	enum parser_types p_type;	/* Type of argument to parse.	*/
	u_short	p_flags;		/* Parser entry control flags.	*/
	u_short	p_cflags;		/* The parser control flags.	*/
	caddr_t p_default;		/* The command default (if any)	*/
	struct	parser_range p_range;	/* Range minimum/maximum values	*/
	int	(*p_func)();		/* Command function (optional).	*/
	u_long	p_cmd;			/* The command code (optional).	*/
 	u_long	p_spec;			/* The command specific value.	*/
	caddr_t	p_argp;			/* Optional argument pointer.	*/
	caddr_t	p_msgp;			/* Optional message pointer.	*/
	caddr_t	p_strp;			/* Optional string pointer.	*/
	struct	key_entry *p_keyp;	/* Keyword table pointer.	*/
	int	p_keyc;			/* The number of keywords.	*/
	u_long	p_kopts[KOPT_ARRAY];	/* Keyword options flag array.	*/
} PARSER_ENTRY;

#define p_min	p_range.pr_min
#define p_max	p_range.pr_max

/*
 * Define the Command Entry Fields:
 */
#define c_name		p_name
#define c_alias		p_alias
#define c_token		p_token
#define c_type		p_type
#define c_flags		p_flags
#define c_cflags	p_cflags
#define c_default	p_default
#define c_min		p_min
#define c_max		p_max
#define c_func		p_func
#define c_cmd		p_cmd
#define c_spec		p_spec
#define c_argp		p_argp
#define c_msgp		p_msgp
#define c_strp		p_strp
#define c_keyp		p_keyp
#define c_keyc		p_keyc
#define c_kopts		p_kopts

/*
 * Define the Keyword Entry Fields:
 */
#define k_name		p_name
#define k_alias		p_alias
#define k_token		p_token
#define k_type		p_type
#define k_flags		p_flags
#define k_cflags	p_cflags
#define k_default	p_default
#define k_min		p_min
#define k_max		p_max
#define k_func		p_func
#define k_cmd		p_cmd
#define k_spec		p_spec
#define k_argp		p_argp
#define k_msgp		p_msgp
#define k_strp		p_strp
#define k_keyp		p_keyp
#define k_keyc		p_keyc
#define k_kopts		p_kopts

/*
 * Declare the Parser Support Funcitons:
 */
extern struct month_entry MonthTable[];
extern struct parser_control *Pc;

#ifdef PROTOTYPE

extern int ParseCommands (struct parser_control *pc);
extern int ParseDate (char *str, struct parser_date *date);
extern struct cmd_entry *FindCmdEntry (enum parser_tokens cmd_token);
extern struct key_entry *FindKeyEntry (struct cmd_entry *ce,
				      	enum parser_tokens cmd_token,
					enum parser_tokens key_token);
#else /* !PROTOTYPE */

extern int ParseCommands();		/* Initial parser entry point.	*/
extern int ParseDate();
extern struct cmd_entry *FindCmdEntry();
extern struct key_entry *FindKeyEntry();

#endif /* PROTOTYPE */
