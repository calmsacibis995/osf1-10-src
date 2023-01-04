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
 * @(#)$RCSfile: cma_util.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:53:55 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  FILENAME:
 *
 *	CMA_UTIL.C
 *
 *  ABSTRACT:
 *
 *	This module implements functions needed by CMA. Much of this code has
 *	been reworked from the VAX CRTL.
 *
 *  AUTHORS:
 *
 *	Paul Curtin
 *
 *  CREATION DATE:
 *
 *	27 July 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Paul Curtin	24 August 1990
 *		Added internal buffer to cma__getenv, no longer  call
 *		cma__alloc_mem.
 *	002	Webb Scales	30 August 1990
 *		- Modified _gets to return 0 for empty lines, since get_input
 *		    doesn't detect EOF at the terminal.
 *		- Added a prompt string to _gets. - Added/corrected VMS
 *		status checks.
 *	003	Paul Curtin	7 September 1990
 *		Fixed conditional platform code, check _cma__vax_vms.
 *	004	Dave Butenhof	10 September 1990
 *		Fix cma__gets on ULTRIX.
 *	005	Paul Curtin	10 September 1990
 *		Fix atoi reference of atol ==> cma__atol
 *	006	Paul Curtin	14 September 1990
 *		Added cma__ftime function.
 *	007	Bob Conti	6 October 1990
 *		Added cma__putint_5, cma__putint_10, cma__puthex_8,  and
 *		cma__set_eol_routine.  Had to change the PUT routines to use
 *		sprintf (i.e. write to the buffer) for the latter routine to
 *		be meaningful on all platforms.
 *	008	Dave Butenhof	25 October 1990
 *		Fix sprintfs to write at proper offset in buffer, rather than
 *		overlaying text already written.
 *	009	Dave Butenhof	27 February 1991
 *		Work around some compilation errors on RS/6000 platform.
 *	010	Dave Butenhof	7 March 1991
 *		Remove cma__strtok function, which is no longer used.
 *	011	Dave Butenhof	27 March 1991
 *		Add trace package.
 *	012	Dave Butenhof	24 April 1991
 *		Remove TRC prefix from trace messages.
 *	013	Dave Butenhof	29 May 1991
 *		Add fflush(stdout) call to cma__abort().
 *	014	DECthreads team	    22 July 1991
 *		Removed use of fflush on VAX/VMS
 *	015	Paul Curtin	20 August 1991
 *		Conditionalized out the include of stdio.h on VMS.
 *	016	Paul Curtin	20 November 1991
 *		Alpha port: Added a number of platform specific items.
 *	017	Dave Butenhof	26 November 1991
 *		Alpha ASSEMBLER's $RAB and $FAB macros don't work, so move
 *		the structures into C. This is VMS-only!
 *	018	Dave Butenhof	27 November 1991
 *		Fix some bugs in 017's setup of RAB and FAB.
 *	019	Dave Butenhof	16 December 1991
 *		Remove some unnecessary RAB/FAB options.
 *	020	Dave Butenhof	18 December 1991
 *		Change interface to cma__getenv() -- it's incorrect, since it
 *		returns address of stack buffer; but while I'm fixing it, I
 *		might as well clean it up and make it reentrant.
 *	021	Dave Butenhof	20 December 1991
 *		Change FAB file sharing of trace file.
 *	022	Dave Butenhof	27 January 1992
 *		Add stdlib.h to get proto for getenv. Also simplify some
 *		conditionals from "vax vms or alpha vms" to "vms".
 *	023	Dave Butenhof	29 January 1992
 *		Simplify some option tests, modify RMS setup.
 *	024	Dave Butenhof	30 January 1992
 *		Add option processing for trace file.
 *	025	Dave Butenhof	06 February 1992
 *		Fix bug in UNIX version of cma__getenv()
 *	026	Dave Butenhof	18 February 1992
 *		Fix an inconsistency in cma__puteol()
 *	027	Dave Butenhof	10 March 1992
 *		Now that we're using gettimeofday() instead of ftime() on
 *		UNIX, change cma__ftime() to cma__gettimeval() [which is like
 *		gettimeofday() except it doesn't return the timezone
 *		information that we don't need anyway].
 *	028	Dave Butenhof	12 March 1992
 *		Add 'wrapper' trace class support.
 *	029	Dave Butenhof	23 March 1992
 *		Separate file open logic from init_trace into a new
 *		cma__int_fopen() that can be used for bugcheck output, too.
 *		Also add cma__putformat to do formatted write using debug
 *		output indirection.
 *	030	Dave Butenhof	30 July 1992
 *		DCE ot 4877: casting byte-aligned char * to (int) for
 *		arithmetic doesn't work on Cray (high bits are byte offset in
 *		addressable word). Should just let ANSI C do pointer
 *		arithmetic directly.
 *	031	Dave Butenhof	31 July 1992
 *		Add cma__strtol function.
 *	032	Dave Butenhof	14 September 1992
 *		Return length of formatted strings.
 *	033	Dave Butenhof	16 October 1992
 *		Get rid of %0*lx -- 0-filling on 64-bits is ugly.
 *	034	Dave Butenhof	19 January 1993
 *		Return NULL for failing cma__int_fopen instead of raising the
 *		RMS error.
 *	035	Dave Butenhof	11 May 1993
 *		Use new TIS disable hook on cma__trace to avoid deadlocks.
 *	036	Dave Butenhof	27 May 1993
 *		Add manager thread trace class.
 *	037	Brian Keane	1 July 1993
 *		Minor touch-ups to eliminate warnings with DEC C on OpenVMS AXP.
 */


/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_util.h>
#include <cma_defs.h>
#include <cma_stack_int.h>
#include <cma_mutex.h>
#include <cma_errors.h>
#if _CMA_OS_ == _CMA__VMS
# include <cma_rms.h>
# include <descrip.h>
# include <ssdef.h>
# include <lib$routines.h>
#else
# include <stdio.h>
# include <stdlib.h>
#endif

#ifdef _CMA_PROTO_
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#ifndef NDEBUG
# include <cma_tis_sup.h>
#endif

/*
 * GLOBAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL DATA
 */
#define NULLC 0
#define EQUAL 0
#define ESCAPE 27               /* escape character */
#define MAX_LOGICAL_NAME 256

#if (_CMA_OS_ == _CMA__VMS)
 static $DESCRIPTOR(lnm_file_dev, "LNM$FILE_DEV");
#endif

static cma__t_eol_routine   cma___g_alt_eol_rtn;    /* Alternate eol routine */
static cma_t_boolean	cma___g_disable_tis = cma_c_false;

#ifndef NDEBUG
  static cma__t_file	cma___g_trace_file = (cma__t_file)cma_c_null_ptr;
  static unsigned int	cma___g_trace_bits;	/* trace enable bits */

# define cma___c_trc_classes	20
  static char	*cma___g_trc_classes[cma___c_trc_classes] = {
    "assembly",				/* Assembly code trace */
    "attributes",			/* Attributes changes */
    "bugcheck",				/* More info before bugcheck */
    "cache",				/* Object cache changes */
    "debug",				/* Debug operations */
    "dispatcher",			/* Dispatcher operation */
    "io",				/* I/O wrappers */
    "initialization",			/* Initialization */
    "null",				/* Null thread */
    "object",				/* Object creation/deletion */
    "scheduling",			/* Scheduling changes */
    "stack",				/* Trace stacks */
    "synchronization",			/* Synchronization functions */
    "timer",				/* Timer operations */
    "temporary",			/* Temporary debug traces */
    "vm",				/* Virtual memory */
    "vp",				/* Virtual Processors */
    "wrapper",				/* Assorted wrappers */
    "zombies",				/* Zombie magic */
    "manager"				/* Manager thread */
    };
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__abort - Execute an illegal instruction that terminates process.
 *
 *  FORMAL PARAMETERS:
 *
 *	None.
 *
 *  IMPLICIT INPUTS:
 *
 *
 *  IMPLICIT OUTPUTS:
 *
 *
 *  FUNCTION VALUE:
 *
 *
 *  SIDE EFFECTS:
 *
 *	Raises the signal SIGABRT.
 */
extern void 
cma__abort
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    sys$exit(SS$_ABORT);
#else
    (void)fflush (stdout);

    abort();
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__atol - Converts ascii string to a long integer.  The string
 *	may be preceeded by an arbitrary number of tabs or spaces and
 *	an optional sign.  The first non_digit terminates the string.
 *
 *  FORMAL PARAMETERS:
 *
 *	string	pointer to a digit character
 *
 *  IMPLICIT INPUTS:
 *
 *
 *  IMPLICIT OUTPUTS:
 *
 *
 *  FUNCTION VALUE:
 *
 *	Integer value of string.
 *
 *  SIDE EFFECTS:
 *
 */
extern cma_t_integer
cma__atol
#ifdef _CMA_PROTO_
	(
	char	    *string)
#else	/* no prototypes */
	(string)
	char *string;
#endif	/* prototype */
    {
    cma_t_integer result;
    cma_t_integer sign;

    result=0; sign=0;

    for (;;string++) {
	    switch (*string) {
	    case ' ':
	    case '\t':
	    case '\n':
	    case '\v':
	    case '\f':
	    case '\r':
		    continue;
	    case '-':
		    sign++;
	    case '+':
		    string++;
		    }
	    break;
	    }

    while (*string >= '0' && *string <= '9')
	    result=result*10 + *string++ - '0';

    return (sign? -result : result);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	CMA__ATOI - this function is the same as ATOL on VMS.
 *
 *  FORMAL PARAMETERS:
 *
 *	string	pointer to a digit character string
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	The integer value of string.
 *
 *  SIDE EFFECTS:
 *
 */
extern cma_t_integer 
cma__atoi
#ifdef _CMA_PROTO_
	(
	char	    *string)
#else	/* no prototypes */
	(string)
	char	    *string;
#endif	/* prototype */
    {
    return (cma__atol(string));
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__int_fopen:		Create a DECthreads file
 *
 *  FORMAL PARAMETERS:
 *
 *	file			file name
 *	mode			file mode ("w")
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Address of file descriptor
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern cma__t_file
cma__int_fopen
#ifdef _CMA_PROTO_
	(char *file, char *mode)
#else
	(file, mode)
	char	*file;
	char	*mode;
#endif
    {
    int			filelen = cma__strlen (file);
#if _CMA_OS_ == _CMA__VMS
    unsigned long	status;
    cma__t_file		filestruct;


    /*
     * FIX-ME:
     *
     * Right now, the VMS code ignores the "mode" of the file, always opening
     * with PUT and SHRGET access. It wouldn't be hard to handle "r" and "a"
     * modes, but it doesn't seem worth the effort now.
     */
    filestruct = (cma__t_file)cma__alloc_zero (sizeof (cma__t_vmsfile));
    filestruct->fab.fab$b_bid = FAB$C_BID;
    filestruct->fab.fab$b_bln = FAB$C_BLN;
    filestruct->fab.fab$l_fop = FAB$M_SQO;
    filestruct->fab.fab$b_fac = FAB$M_PUT;
    filestruct->fab.fab$b_shr = FAB$M_SHRGET;
    filestruct->fab.fab$b_org = FAB$C_SEQ;
    filestruct->fab.fab$b_rat = FAB$M_CR;
    filestruct->fab.fab$b_rfm = FAB$C_STMLF;

    if (filelen == 6 && (cma__strncmp (file, "stdout", 6) == 0
	    || cma__strncmp (file, "STDOUT", 6) == 0)) {
	filestruct->fab.fab$l_fna = "sys$output:";
	filestruct->fab.fab$b_fns = 11;
	status = sys$open (&filestruct->fab);
	}
    else if (filelen == 6 && (cma__strncmp (file, "stderr", 6) == 0
	    || cma__strncmp (file, "STDERR", 6) == 0)) {
	filestruct->fab.fab$l_fna = "sys$error:";
	filestruct->fab.fab$b_fns = 10;
	status = sys$open (&filestruct->fab);
	}
    else {
	filestruct->fab.fab$l_fop |= FAB$M_MXV;
	filestruct->fab.fab$l_fna = file;
	filestruct->fab.fab$b_fns = filelen;
	status = sys$create (&filestruct->fab);
	}

    if (!(status & 1)) {
	cma__free_mem (filestruct);
	return (cma__t_file)0;
	}

    filestruct->rab.rab$b_bid = RAB$C_BID;
    filestruct->rab.rab$b_bln = RAB$C_BLN;
    filestruct->rab.rab$b_rac = RAB$C_SEQ;
    filestruct->rab.rab$l_fab = &filestruct->fab;
    status = sys$connect(&filestruct->rab);

    if (!(status & 1)) {
	cma__free_mem (filestruct);
	return (cma__t_file)0;
	}

    return filestruct;
#else
    /*
     * Handle "stdout" & "stderr" as special cases... point to the proper
     * pre-defined stream.
     */
    if (filelen == 6 && cma__strncmp (file, "stdout", 6) == 0)
	return (cma__t_file)stdout;
    else if (filelen == 6 && cma__strncmp (file, "stderr", 6) == 0)
	return (cma__t_file)stderr;
    else {
	return (cma__t_file)fopen (file, mode);
	}
#endif
    }

#ifndef NDEBUG
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__init_trace:	Initialize trace file
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__init_trace
#ifdef _CMA_PROTO_
	(char	*trace_env)
#else	/* no prototypes */
	(trace_env)
	char	*trace_env;
#endif	/* prototype */
    {
    int			trclen, oplen, optcnt, key;
    cma_t_boolean	negopt, firstopt;
    char		*opt;


    cma___g_trace_bits = 0;		/* Clear all trace bits */
    firstopt = cma_c_true;		/* We're on first option */

    opt = trace_env;

    /*
     * Find the end of the trace file name
     */
    while (*opt != ',' && *opt != ' ' && *opt != '\t' && *opt != '\0')
	opt++;

    trclen = (int)(opt - trace_env);
    optcnt = 0;

    /*
     * Now process the trace options
     */
    while (*opt != '\0') {
	/*
	 * Find the start of the next option
	 */
	while (*opt == ',' || *opt == ' ' || *opt == '\t')
	    opt++;

	oplen = 0;

	/*
	 * A '-' at beginning of option means to negate it. If this is the
	 * first option, then start by negating the entire trace mask (in
	 * other words, we want to enable all EXCEPT certain options).
	 */
	if (*opt == '-') {

	    if (firstopt) cma___g_trace_bits = ~0;

	    opt++;			/* Skip the '-' */

	    while (*opt == ' ' || *opt == '\t')	/* Skip whitespace */
		opt++;

	    negopt = cma_c_true;
	    }
	else
	    negopt = cma_c_false;

	firstopt = cma_c_false;		/* Not first anymore */

	while (opt[oplen] != ',' && opt[oplen] != ' ' && opt[oplen] != '\0') {

	    if (opt[oplen] >= 'A' && opt[oplen] <= 'Z')
		opt[oplen] = (opt[oplen] + ('a' - 'A'));

	    oplen++;
	    }

	for (key = 0; key < cma___c_trc_classes; key++) {

	    if (cma__strncmp (opt, cma___g_trc_classes[key], oplen) == 0) {

		if (negopt)
		    cma___g_trace_bits &= ~(1 << key);
		else
		    cma___g_trace_bits |= (1 << key);

		optcnt++;
		break;
		}

	    }	    

	opt += oplen;
	}

    /*
     * If no options were specified, then default to ALL
     */
    if (optcnt == 0)
	cma___g_trace_bits = ~0;

    trace_env[trclen] = '\0';
    cma___g_trace_file = cma__int_fopen (trace_env, "w");

    if (cma___g_trace_file == (cma__t_file)0) {
	cma___g_trace_file = cma__int_fopen ("stderr", "w");
	cma__trace_print (
		cma__c_trc_init,
		"(init_trace) opening trace file stderr: %s not writable",
		trace_env);
	}
    else {
	cma__trace_print (
		cma__c_trc_init,
		"(init_trace) opening trace file %s",
		trace_env);
	}

    }
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__trace_print - Print trace statements to a log file.
 *
 *  FORMAL PARAMETERS:
 *
 *	const char *string
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Returns length of string.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__trace_print
#ifdef _CMA_PROTO_
	(
	int	type,
	char	*reason,
	...)
#else	/* no prototypes */
	(va_alist)
	va_dcl
#endif	/* prototype */
    {
    va_list	ap;
    char	buffer[512];
#ifdef _CMA_PROTO_
    va_start (ap, reason);
#else
    int		type;
    char	*reason;

    va_start (ap);
    type = va_arg (ap, int);
    reason = va_arg (ap, char*);
#endif

    /*
     * If trace file hasn't been opened, return.
     */
    if (cma___g_trace_file != (cma__t_file)cma_c_null_ptr) {

	/*
	 * If none of the facilities that it's trying to trace are enabled,
	 * return.
	 */
	if ((cma___g_trace_bits & type) != 0) {
	    cma_t_boolean	saved_bit = cma__g_tis_disable;

	    if (cma___g_disable_tis)
		cma__g_tis_disable = cma_c_true;

	    cma__int_vsprintf (
		    buffer,
		    reason,
		    ap);
	    cma__int_fprintf (cma___g_trace_file, "%s\n", buffer);
#if _CMA_OS_ != _CMA__VMS
	    fflush (cma___g_trace_file);
#endif

	    if (cma___g_disable_tis)
		cma__g_tis_disable = saved_bit;

	    }

	}

    va_end (ap);
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__getenv - this routine searches the c environmental database 
 *	and the VMS logical tables for a string of the format name=value 
 *	where "name" is equivalent to input argument.  A pointer to the 
 *	string "value" is returned if found.
 *
 *	This routine is VMS specific!
 *
 *  FORMAL PARAMETERS:
 *
 *	name	pointer to string "name".
 *	buffer	pointer to character buffer for translation
 *	size	size of buffer
 *
 *  IMPLICIT INPUTS:
 *
 *	It is assume that the environ pointer has been set up before 
 *	control can be passed here.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None.
 *
 *  FUNCTION VALUE:
 *
 *	If string is found, then a pointer to string "value" is returned,
 *	otherwise, a NULL pointer is returned.
 *
 *  SIDE EFFECTS:
 *
 */
extern char *
cma__getenv
#ifdef _CMA_PROTO_ 
	(
	char    *name,
	char	*buffer,
	int	size)
#else	/* no prototypes */
	(name, buffer, size)
	char	*name;
	char	*buffer;
	int	size;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    char			*ptr;	/* pointer to translated name */
    struct dsc$descriptor	logical_name;	/* logical name descriptor */
    short			return_length = 0;	/* for $TRNLNM */

    struct {
	unsigned short	buffer_length;
	unsigned short	item_code;
	char		*buffer_address;
	short		*return_length_address;
	int		end;
	} itmlst;

    int	SYS$TRNLNM();			/* translate logical name */

    if (buffer == NULLC)
	return cma_c_null_ptr;

    itmlst.buffer_length = size;
    itmlst.item_code = 2;
    itmlst.buffer_address = buffer;
    itmlst.return_length_address = &return_length;
    itmlst.end = 0;

    logical_name.dsc$w_length = cma__strlen(name);
    logical_name.dsc$a_pointer = name;
    logical_name.dsc$b_dtype = DSC$K_DTYPE_T;
    logical_name.dsc$b_class = DSC$K_CLASS_S;

    if (SYS$TRNLNM (0, &lnm_file_dev, &logical_name, 0, &itmlst) == SS$_NORMAL)
	{
	buffer[return_length] = '\000';
	ptr = buffer;

	if (ptr[0] == ESCAPE)
	    ptr += 4;

	return ptr;
	}

    return (char *)cma_c_null_ptr;
#else
    char	*ptr;
    int		i;

    ptr = getenv (name);

    if ((long)ptr <= 0)
	return (char *)cma_c_null_ptr;

    for (i = 0; ptr[i] != '\000' && i < size; i++)
	buffer[i] = ptr[i];

    buffer[i] = '\000';
    return buffer;
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__gets - reads a line from standard input.
 *
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		str - pointer to char (preallocated - char 
 *		      buffer[cma__c_buffer_size]).
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	A pointer to the line that was read; or, null indicating error.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern char *
cma__gets
#ifdef _CMA_PROTO_
	(
	char	*prompt,
	char	*string)	    /* Address of char[cma__c_buffer_size] */
#else	/* no prototypes */
	(prompt, string)
	char	*prompt;
	char	*string;	    /* Address of char[cma__c_buffer_size] */
#endif	/* prototype */
    {
    cma_t_integer   prompt_dsc[2];
    cma_t_integer   string_dsc[2];
    cma__t_short    ret_length;
    cma_t_integer   status;


#if _CMA_OS_ == _CMA__VMS
    prompt_dsc[0] = cma__strlen (prompt);
    prompt_dsc[1] = (cma_t_integer)prompt;
    string_dsc[0] = cma__c_buffer_size;
    string_dsc[1] = (cma_t_integer)string;
    status = lib$get_input (string_dsc, prompt_dsc, &ret_length);
    if (!(status&1))	return 0;

    /*
     * Since lib$get_input doesn't detect EOF on the terminal, treat an
     * empty line the same as an EOF.
     */
    if (ret_length == 0)    return 0;

    cma__memcpy (string, (char *)string_dsc[1], ret_length);
    string[ret_length] = '\0';
    return string;
#else
    printf ("%s", prompt);
    fflush (stdout);
    return (cma_t_address)gets (string);
#endif

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__memcpy - copies the string specified by the second argument, if
 *	present, to the area whose address is given in the first argument.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		s1 - pointer to first object (area to be copied to).
 *		s2 - pointer to second object (area to be copied from).
 *		size - number of bytes to be copied.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	pointer to s1
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern char *
cma__memcpy
#ifdef _CMA_PROTO_
	(
	char		*s1,
	char		*s2,
	cma_t_integer	size)
#else	/* no prototypes */
	(s1, s2, size)
	char		*s1;
	char		*s2;
	cma_t_integer	size;
#endif	/* prototype */
    {
    cma_t_integer	count;

    count = 0;

    while (count != size) {
	*(s1+count) = *(s2+count);
	count++;
	}

    return s1;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__memset - sets the specified number of bytes to the given 
 *	value in the area whose address is given in the first argument.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		s	- an array pointer.
 *		value	- value to be placed in s.
 *		size	- number of bytes to be placed in s.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Functions returns pointer to s.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern char *
cma__memset
#ifdef _CMA_PROTO_
	(
	char		*s,
	cma_t_integer	value,
	cma_t_integer	size)
#else	/* no prototypes */
	(s, value, size)
	char		*s;
	cma_t_integer	value;
	cma_t_integer	size;
#endif	/* prototype */
    {
    char		*ptr = s;
    cma_t_integer	i;


    for (i = 0; i < size; i++, ptr++)
	*ptr = value;

    return s;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__putformat - takes a pointer to a null terminated string and 
 *	formats the string to a buffer to be put to standard output when
 *	cma__puteol is called.
 *
 *  FORMAL PARAMETERS:
 *
 *	buffer	- buffer for output (appended to end of null-term. string).
 *	format	- pointer to null terminated string.
 *	args	- list of sprintf arguments.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of formatted string
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__putformat
#ifdef _CMA_PROTO_
	(
	char		*buffer,
	char		*format,
	...)
#else	/* no prototypes */
	(va_alist)
	va_dcl
#endif	/* prototype */
    {
    cma_t_integer	buf_length;
    va_list		ap;
#ifdef _CMA_PROTO_
    va_start (ap, format);
#else
    char	*buffer;
    char	*format;


    va_start (ap);
    buffer = va_arg (ap, char*);
    format = va_arg (ap, char*);
#endif

    buf_length = cma__strlen (buffer);
    cma__int_vsprintf (&buffer[buf_length], format, ap);
    va_end (ap);

    return (cma__strlen (buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__putstring - takes a pointer to a null terminated string and 
 *	puts the string to a buffer to be put to standard output when
 *	cma__puteol is called.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		string	- pointer to null terminated string.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of string
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__putstring
#ifdef _CMA_PROTO_
	(
	char		*buffer,
	char		*string)
#else	/* no prototypes */
	(buffer, string)
	char		*buffer;
	char		*string;
#endif	/* prototype */
    {
    cma_t_integer	buf_length;

    buf_length = cma__strlen(buffer);
    cma__int_sprintf(&buffer[buf_length], "%s", string);
    return (cma__strlen(buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__putint - takes an integer by value and places it to
 *	a buffer which is put to standard output upon a call cma__
 *	puteol.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		number	- integer by value.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of formatted integer
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__putint
#ifdef _CMA_PROTO_
	(
	char		*buffer,
	cma_t_integer	number)
#else	/* no prototypes */
	(buffer, number)
	char		*buffer;
	cma_t_integer	number;
#endif	/* prototype */
    {
    cma_t_integer	ctrstr[2];
    cma_t_integer	temp_desc[2];
    cma__t_short	outlen;
    cma_t_integer	buf_length;


    buf_length = cma__strlen(buffer);
    cma__int_sprintf (&buffer[buf_length], "%d", number);
    return (cma__strlen(buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__putint_5 - takes an integer by value and places it to
 *	a buffer which is put to standard output upon a call cma__
 *	puteol. The number is right-justified in a blank-filled field of 5.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		number	- integer by value.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of formatted integer
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__putint_5
#ifdef _CMA_PROTO_
	(
	char		*buffer,
	cma_t_integer	number)
#else	/* no prototypes */
	(buffer, number)
	char		*buffer;
	cma_t_integer	number;
#endif	/* prototype */
    {
    cma_t_integer	ctrstr[2];
    cma_t_integer	temp_desc[2];
    cma__t_short	outlen;
    cma_t_integer	buf_length;


    buf_length = cma__strlen(buffer);
    cma__int_sprintf (&buffer[buf_length], "%5d", number);
    return (cma__strlen(buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__putint_10 - takes an integer by value and places it to
 *	a buffer which is put to standard output upon a call cma__
 *	puteol. The number is right-justified in a blank-filled field of 10.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		number	- integer by value.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of formatted integer
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__putint_10
#ifdef _CMA_PROTO_
	(
	char		*buffer,
	cma_t_integer	number)
#else	/* no prototypes */
	(buffer, number)
	char		*buffer;
	cma_t_integer	number;
#endif	/* prototype */
    {
    cma_t_integer	ctrstr[2];
    cma_t_integer	temp_desc[2];
    cma__t_short	outlen;
    cma_t_integer	buf_length;


    buf_length = cma__strlen(buffer);
    cma__int_sprintf (&buffer[buf_length], "%10d", number);
    return (cma__strlen(buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__puthex - takes a hexidecimal number by value and places it to
 *	a buffer which is put to standard output when cma__puteol is called.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		number	- integer by value.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of formatted integer
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__puthex
#ifdef _CMA_PROTO_
        (
	char		*buffer,
        cma_t_integer   number)
#else   /* no prototypes */
        (buffer, number)
	char		*buffer;
        cma_t_integer   number;
#endif  /* prototype */
    {
    cma_t_integer	buf_length;


    buf_length = cma__strlen(buffer);
    cma__int_sprintf (&buffer[buf_length], "%lx", number);
    return (cma__strlen(buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__puthex_long - takes a long int by value and formats it to
 *	a buffer which is put to standard output when cma__puteol is called.
 *	The number is 0-filled and right-justified in the field.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		number	- integer by value.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Length of formatted integer
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int
cma__puthex_long
#ifdef _CMA_PROTO_
        (
	char		*buffer,
        cma_t_integer   number)
#else   /* no prototypes */
        (buffer, number)
	char		*buffer;
        cma_t_integer   number;
#endif  /* prototype */
    {
    cma_t_integer	buf_length;


    buf_length = cma__strlen(buffer);
    cma__int_sprintf (&buffer[buf_length], "%lx", number);
    return (cma__strlen(buffer) - buf_length);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__puteol - put what is in cma___g_put_buffer to standard
 *	output.
 *
 *  FORMAL PARAMETERS:
 *
 *	None
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__puteol
#ifdef _CMA_PROTO_
        (
	char		*buffer)
#else   /* no prototypes */
        (buffer)
	char		*buffer;
#endif  /* prototype */
    {

    /*
     * Call the alternate eol routine if one is defined. Otherwise call the
     * default output routine for the platform.
     */
    if (cma___g_alt_eol_rtn != (cma__t_eol_routine)cma_c_null_ptr)
	(*cma___g_alt_eol_rtn) (buffer);
    else
	cma__int_printf ("%s\n", buffer);

#if _CMA_OS_ == _CMA__UNIX
    fflush (stdout);
#endif
    buffer[0] = '\0';
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *  cma__set_eol_routine is called to specify an alternate end of line
 *  routine.  If the current alternate end of line routine exists,
 *  it will be called instead of performing the default eol action
 *  when cma__put_eol is called.
 *
 *  FORMAL PARAMETERS:
 *
 *	new_eol_routine	    in	New alternate end-of-line routine
 *	prior_eol_routine   out	Prior alternate end-of-line routine
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The current alternate eol routine is returned and the
 *	new alternate eol routine is established.
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__set_eol_routine 
#ifdef _CMA_PROTO_
	(
	cma__t_eol_routine	new_eol_routine,
	cma__t_eol_routine	*prior_eol_routine
	)
#else	/* no prototypes */
	(new_eol_routine,
	prior_eol_routine)
	cma__t_eol_routine	new_eol_routine;
	cma__t_eol_routine	*prior_eol_routine;
#endif	/* prototype */
    {

    if (prior_eol_routine != (cma__t_eol_routine *)0)
	*prior_eol_routine	= cma___g_alt_eol_rtn;

    cma___g_alt_eol_rtn	= new_eol_routine;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__strlen - calculates the length of the string specified in 
 *	arg 1 by scanning the string looking for a null byte terminator.
 *
 *  FORMAL PARAMETERS:
 *
 *	const char *s
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	Returns length of string.
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern cma_t_integer
cma__strlen
#ifdef _CMA_PROTO_ 
	(
	char	*s)
#else	/* no prototypes */
	(s)
	char	*s;
#endif	/* prototype */
    {
    char *tmpstr = s;

    while (*tmpstr++ != '\0');

    return (tmpstr - s) - 1;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__strncmp - compares the character array specified by the first 
 *	argument with the character array specified by the second argument.
 *	Up to the number of characters specified by the third argument
 *	are compared.
 *
 *  FORMAL PARAMETERS:
 *	Inputs:
 *		string1 - pointer to first string to be compared.
 *		string2 - pointer to second string to be compared.
 *		length - length of characters to be compared.
 *
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *      The first character array is compared with the second character array.
 *      If the character arrays are equal, then zero is returned as the func-
 *      value. Otherwise the difference between the mismatched characters is
 *      returned as the function value. Up to the number of characters speci-
 *      fied by the third argument are compared.
 *
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern int 
cma__strncmp
#ifdef _CMA_PROTO_
	(
	char		*str1,
	char		*str2,
	cma_t_integer	length)
#else	/* no prototypes */
	(str1, str2, length)
	char		*str1;
	char		*str2;
	cma_t_integer	length;
#endif	/* prototype */
    {
    cma_t_integer	match_count;
    cma_t_integer	i;

    i = 0;
    match_count = 0;

    while ((str1[i] != '\0') && (str2[i] != '\0')
	    && (i < length) && (str1[i] == str2[i])) {
	match_count++;
	i++;
	}

    if (match_count == length)
	return 0;
    else
	return -1;

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__gettimespec - Returns the current system time as a 'timespec'
 *	structure (an offset from the UNIX epoch in seconds and
 *	nanoseconds).
 *
 *  FORMAL PARAMETERS:
 *
 *	time	Address of 'struct timespec' to receive time.
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *      0 (success).
 *
 *  SIDE EFFECTS:
 *
 *	Any failure in VMS system calls results in a bugcheck (error is not
 *	related to inputs, can't be corrected, and may indicate serious
 *	problems in the system or process).
 */
extern int
cma__gettimespec
#ifdef _CMA_PROTO_
	(
	struct timespec	*time_struc)
#else	/* no prototypes */
	(time_struc)
	struct timespec	*time_struc;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    cma_t_integer	    factor[2];	/* Diff Jan 1, 1970 - Nov 17, 1858 */
    cma_t_integer	    sys_time[2];
    cma_t_integer	    out_time[2];
    cma_t_integer	    divisor1;
    cma_t_integer	    status;


    factor[0] = 0x4beb4000;
    factor[1] = 0x007c9567;
    divisor1 = 10000000;

    status = sys$gettim (sys_time);

    if (status != SS$_NORMAL)
	cma__bugcheck ("gettimeval: gettime");

    status = lib$subx (sys_time, factor, out_time);

    if (!(status&1))
	cma__bugcheck ("gettimeval: subx");

    status = lib$ediv (
	    &divisor1,
	    out_time,
	    &out_time[0],
	    &out_time[1]);

    if (!(status&1))
	cma__bugcheck ("gettimeval: ediv");

    time_struc->tv_sec = out_time[0];
    time_struc->tv_nsec = out_time[1] * 100;
#else
    struct timeval	tmptime;
    struct timezone	zone;


    if (gettimeofday (&tmptime, &zone) == -1)
	cma__bugcheck ("gettimeval: gettimeofday");

    time_struc->tv_sec = tmptime.tv_sec;
    time_struc->tv_nsec = tmptime.tv_usec * 1000;
#endif

    return 0;

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__strtoul - Converts ascii string to a long integer.  The string
 *	may be preceeded by an arbitrary number of tabs or spaces and
 *	an optional sign.  The first non_digit terminates the string.
 *
 *  FORMAL PARAMETERS:
 *
 *	string	pointer to a numeric string (may be 0x, 0X, 0 prefixed)
 *
 *  IMPLICIT INPUTS:
 *
 *
 *  IMPLICIT OUTPUTS:
 *
 *
 *  FUNCTION VALUE:
 *
 *	Integer value of string.
 *
 *  SIDE EFFECTS:
 *
 */
extern unsigned long int
cma__strtoul
#ifdef _CMA_PROTO_
	(
	_CMA_CONST_ char	*string,
	char			**endptr,
	int			in_base)
#else	/* no prototypes */
	(string, endptr, in_base)
	char	*string;
	char	**endptr;
	int	in_base;
#endif	/* prototype */
    {
    char		*ptr;
    unsigned long int	result;
    cma_t_integer	radix;
    char		max_ll, max_ul, max_digit;


    if (in_base != 0 && (in_base < 2 || in_base > 36)) return 0;

    result = 0L; radix = in_base;
    ptr = (char *) string;

    if (endptr != (char **)cma_c_null_ptr) *endptr = (char *) string;

    for (;;ptr++) {
	switch (*ptr) {
	    case ' ':
	    case '\t':
	    case '\n':
	    case '\v':
	    case '\f':
	    case '\r':
		    continue;
	    }
	break;
	}

    if (*ptr == '0') {			/* There might be a radix */
	ptr++;

	if (*ptr == 'x' || *ptr == 'X') {
	    /*
	     * If radix is already 16, ignore the "0x" prefix. If radix is 0,
	     * then adapt to the specified radix. If some other radix was
	     * specified, the "x" is illegal, so return without converting
	     * anything.
	     */
	    ptr ++;

	    if (radix != 16 && radix != 0)
		return 0;

	    radix = 16;
	    }
	else {
	    /*
	     * If there's a leading 0, and radix is 0, then adapt to the
	     * specified radix (octal). If radix is specified as anything
	     * else, we just ignore the leading 0.
	     */

	    if (radix == 0)
		radix = 8;

	    }
	    
	}

    if (radix == 0)			/* If not oct or hex, assume decimal */
	radix = 10;

    if (radix < 10)
	max_digit = '0' + (char)radix;
    else
	max_digit = '9';

    max_ll = 'a' + (char)(radix - 10);
    max_ul = 'A' + (char)(radix - 10);

    while (*ptr != '\000' && ((*ptr >= '0' && *ptr <= max_digit)
	    || (*ptr >= 'a' && *ptr < max_ll)
	    || (*ptr >= 'A' && *ptr < max_ul))) {

	if (*ptr <= '9')
	    result = (result * radix) + (*ptr++ - '0');
	else if (*ptr <= 'Z')
	    result = (result * radix) + (*ptr++ - 'A') + 10;
	else
	    result = (result * radix) + (*ptr++ - 'a') + 10;

	}

    if (endptr != (char **)cma_c_null_ptr) *endptr = ptr;

    return result;
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_UTIL.C */
/*  *33    2-JUL-1993 09:43:55 KEANE "Fix DEC C warnings: add return statements to non-void routines" */
/*  *32   15-JUN-1993 08:00:51 KEANE "resolve compile problems with DEC C on OSF/1 AXP" */
/*  *31    1-JUN-1993 11:08:11 BUTENHOF "Drop tis disable from trace" */
/*  *30   27-MAY-1993 14:32:50 BUTENHOF "Add manager thread trace class" */
/*  *29   14-MAY-1993 15:57:02 BUTENHOF "Disable TIS lock on cma__trace" */
/*  *28   16-APR-1993 13:06:40 BUTENHOF "Add '-' option to trace decode" */
/*  *27   19-JAN-1993 12:56:45 BUTENHOF "Handle dump file open error" */
/*  *26   16-OCT-1992 12:19:52 BUTENHOF "Get rid of 0-filled addresses" */
/*  *25   15-SEP-1992 13:50:56 BUTENHOF "Return length of formatted output" */
/*  *24   31-JUL-1992 15:31:47 BUTENHOF "Fix trace and strlen" */
/*  *23   24-MAR-1992 13:46:39 BUTENHOF "Put bugcheck output in file" */
/*  *22   13-MAR-1992 14:10:35 BUTENHOF "Add tracing" */
/*  *21   10-MAR-1992 16:27:58 BUTENHOF "Change ftime to gettimeofday" */
/*  *20   18-FEB-1992 15:31:17 BUTENHOF "Fix cma__puteol" */
/*  *19    7-FEB-1992 10:30:41 BUTENHOF "Fix bug in unix cma__getenv" */
/*  *18   30-JAN-1992 11:56:09 BUTENHOF "Add support for trace classes" */
/*  *17   27-JAN-1992 05:50:07 BUTENHOF "Fix type mismatch" */
/*  *16   20-DEC-1991 11:01:52 BUTENHOF "Change trace file sharing" */
/*  *15   19-DEC-1991 13:08:36 BUTENHOF "Remove some RMS options" */
/*  *14   27-NOV-1991 09:25:17 BUTENHOF "Fix RMS trace code" */
/*  *13   26-NOV-1991 16:15:49 BUTENHOF "Move RAB to C code" */
/*  *12   21-NOV-1991 10:15:23 CURTIN "Remove include of starlet.h" */
/*  *11   20-NOV-1991 13:39:22 CURTIN "Alpha port" */
/*  *10   17-SEP-1991 13:21:51 BUTENHOF "Chase some compilation warnings" */
/*  *9    21-AUG-1991 16:45:19 CURTIN "Removed VMS include of stdio.h" */
/*  *8    26-JUL-1991 15:56:05 CURTIN "remove CRTL (vms) dependence" */
/*  *7    10-JUN-1991 18:24:58 SCALES "Add sccs headers for Ultrix" */
/*  *6    29-MAY-1991 17:46:12 BUTENHOF "flush stdout on cma__abort()" */
/*  *5     2-MAY-1991 13:59:40 BUTENHOF "Standardize trace messages" */
/*  *4     1-APR-1991 18:09:47 BUTENHOF "Add trace function" */
/*  *3     8-MAR-1991 18:49:14 BUTENHOF "Don't need strtok anymore" */
/*  *2    27-FEB-1991 15:48:23 BUTENHOF "Fix for IBM" */
/*  *1    12-DEC-1990 21:55:19 BUTENHOF "Utilities" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_UTIL.C */
