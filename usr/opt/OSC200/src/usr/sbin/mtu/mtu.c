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
static char *rcsid = "@(#)$RCSfile: mtu.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/08 16:13:59 $";
#endif
/*
 **  Title: mtu.c	Main module for CA MIB Translator Utility
 **
 **  Copyright (c) Digital Equipment Corporation, 1990, 1991, 1992, 1993
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
 **
 */

/*
 **++
 **  FACILITY:  PolyCenter Common Agent & DECmcc -- MIB Translator Utility
 **
 **  MODULE DESCRIPTION:
 **
 **      Main module for MTU program
 **
 **	For VMS --
 **		The program can be built with eca_fake_vm.b32 in order to automatically
 **		report memory leaks, clobbers, etc during execution.
 **  AUTHORS:
 **
 **      Rahul Bose
 **
 **  CREATION DATE:  01-March-1991
 **
 **  DESIGN ISSUES:
 **	o Variation between DECmcc ms and CA ms
 **	o Protocol output representation
 **	
 **  MODIFICATION HISTORY:
 **
 ** 01	   01-Mar-1991	Rahul Bose 	Created.
 ** 02     06-Aug-1992  Peter Burgess   Common Agent modifications to support new dictionary (MIR)
 ** 03     14-Aug-1992	Peter Burgess   Changes for CA FT2
 **					o c89 -std (ANSI C -std)
 **					o Internalization (I18n)
 **					o Restructured code for performance and maintainability
 **					o Incoperated Rahul's Bose DECmcc v1.3 enhancments (helpfile production,
 **						and enterprise trap support)
 ** 	   21-Apr-1993 Peter Burgess	Eliminated prune option (again) now that mib->msl mapping is improved
 **--
 */

#include <errno.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mtu.h"
#include "mtu_msg.h"
#include "iso_defs.h"
#ifdef NL
# include <nl_types.h>
# include <langinfo.h>
# include <locale.h>
#endif
/*
 ** External functions
 */
#if VMS
extern int FAKE_VM_INTERCEPT_XFER_VECTOR(), FAKE_VM(), FAKE_VM_HISTOGRAM();
extern void FAKE_VM_SET_FILENAME();
#endif

extern int getopt();
extern int ca_check_base_license();
extern int yyparse();
extern int produce_entity_tree();
extern int produce_ms_file();
extern int init_mibtree(), init_entitytree();

/*
 ** External data
 */
#ifdef NL
extern nl_catd _m_catd;				/* NL catalog descriptor */
#endif

extern FILE *yyin, *yyout;		  	/* Lex input and output files */
extern int yylineno;			  	/* linkage to YACC-produced line number */
extern int yynerrs;  				/* number of errors in yyparse*/

extern char *sys_errlist[];			/* vector of message strings */
extern int sys_nerr;				/* number of messages */

/* 
 ** Global program data
 */
char *prefix_event_oid;
char *prefix_event_enterprise_arg;
char *prefix_event_agent_addr_arg;
char *prefix_event_generic_trap_arg;
char *prefix_event_specific_trap_arg;
char *prefix_event_time_stamp_arg;
char *prefix_event_varbind_arg;

typedef struct s_prefix_table {
  int oid_length;
  int oid[25];
  char ** oid_string;
}  t_prefix_table;

t_prefix_table prefix_table[] = {
  {PREFIX_EVENTTYPE_LENGTH, 	{ PREFIX_EVENTTYPE_SEQ },	&prefix_event_oid},
  {PREFIX_ENTERPRISE_LENGTH,	{ PREFIX_ENTERPRISE_SEQ },	&prefix_event_enterprise_arg},
  {PREFIX_AGENTADDR_LENGTH,	{ PREFIX_AGENTADDR_SEQ },	&prefix_event_agent_addr_arg},
  {PREFIX_GENERICTRAP_LENGTH,	{ PREFIX_GENERICTRAP_SEQ },	&prefix_event_generic_trap_arg},
  {PREFIX_SPECIFICTRAP_LENGTH,	{ PREFIX_SPECIFICTRAP_SEQ },	&prefix_event_specific_trap_arg},
  {PREFIX_EVENTTIME_LENGTH,	{ PREFIX_EVENTTIME_SEQ },	&prefix_event_time_stamp_arg},
  {PREFIX_VARBINDARG_LENGTH,	{ PREFIX_VARBINDARG_SEQ },	&prefix_event_varbind_arg},
  {0,0,0}
};

T_CMDLINE_OPTION cmdline_table[N_CMDLINE_OPTIONS]; /* Parsed command line */
T_MIB_OBJECT_NODE *mib_root;           	 	/* Root of mib tree */
T_ENTITY_NODE *entity_root;	        	/* Root of entity model tree */
T_TYPEDEF_OBJECT *typedef_object_list[2];	/* List head of typedef objects */
FILE *info_stream = 0;				/* Information file descriptor */
FILE *help_stream = 0;				/* Help file descriptor */
FILE *log_stream = 0;				/* Log file descriptor */
FILE *ms_stream = 0;				/* ms file descriptor */
char mib_filename[SIZE_OF_FILENAME];		/* File Names ... */
char ms_filename[SIZE_OF_FILENAME];
char help_filename[SIZE_OF_FILENAME];
char info_filename[SIZE_OF_FILENAME];
char log_filename[SIZE_OF_FILENAME];
char module_name[SIZE_OF_IDENTIFIER];		/* RFC Module name */

int ECA_MTU_LOG;				/* Program Logical (control switch register) */
char *mtu_version = "V1.1.0";			/* MTU Version # */

/* Protocol definition tables 
 ** SMI		PARENT ENTITY Name and SNMP OID
 **------------------------------------------------------*/
T_PROTOCOL_DEF protocol_definition_table [] = {
  {"SNMP",	"SNMP",	{6, {1,3,6,1,2,1}}},		
  {"DNA",	"NODE",	{6, {1,3,6,1,2,1}}}			
};
T_PROTOCOL_DEF * protocol_definition;		/* Ptr to selected protocol */

int log_pageno;
char log_header_str1[PAGE_WIDTH];
char log_header_str2[PAGE_WIDTH];

/* 
 ** Error thresholding data
 **	Given that a mib may produce many, many errors...: Error thresholding
 **	is supported to limit the number of mib errors to the value of ERROR_THRESHOLD.
 **	The std library rtns: setjmp and longjmp are used to check-point the state,
 **	and to abort mib processing if the error threshold is exceeded.
 **
 **	The macro MIB_ERROR expands to...
 **		if (++mib_error_count > ERROR_THRESHOLD) longjmp(mib_env, 1);
 */
jmp_buf mib_env;
int mib_error_count;


/********************************************************************************
  debug_break	-- Transfer control to dbx (U*X)/debug-32 (VMS)
       if (getenv("ECA_MTU_LOG") & 1) then xfr to dbx
  
       MACRO: DEBUG_BREAK invokes this routine (if compiled -DDEBUG)
  ********************************************************************************/
void debug_break ()
{
#ifdef DEBUG
  if (ECA_MTU_LOG & MTU_M_DEBUG) {
#if defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
#  include <sys/types.h>
#  include <signal.h>
#  include <unistd.h>
  kill (getpid(), SIGTRAP);
# else
#  include <ssdef.h>
#  include <lib$routines.h>
  lib$signal (SS$_DEBUG);
# endif
  }
#endif
}


/*****************************************************************************
 SunOS on Sparc does not have strerror(), so we write our own.
 *****************************************************************************/

#if defined(sun) || defined(sparc)

static char *strerror(errno)
int errno;
{
  static char buf1[40];
  char buf2[20];

  if ((errno > 0) && (errno <= sys_nerr))
    return(sys_errlist[errno]);
  else {
    strcpy(buf1, "Unknown errno: ");
    sprintf(buf2, "%d", errno);
    strcat(buf1, buf2);
    return(buf1);
  }
}

#endif


/*********************************************************************************
  ** get_cmdline	- Parse User command line, producing parse structure of parameters and options
  **
  ** cmdline  : cmd_list
  ** cmd_list : cmd | cmd cmd_list
  ** cmd      : option_list  filename
  ** option_list   : option | option_list | <empty>
  ** option   : -a | -c <code> | -d | -h | -i | -m | -s |
  ** filename : file-path 	(* May include '*' characters *)
  ** 
  ** Returns:
  ** True   - If success
  ** False  - If failure
  ********************************************************************************/
static int get_cmdline 
#ifdef PROTOTYPE_ALLOWED
  (int cmd_argc,					/* CMDline: Argument count (input) */
   char **cmd_argv, 					/* CMDline: Argument vector(input) */
   T_CMDLINE_OPTION p_cmdline_table[N_CMDLINE_OPTIONS])	/* CMDline: Table of cmdline options (output) */
#else
(cmd_argc, cmd_argv, p_cmdline_table)
int cmd_argc;
char **cmd_argv;
T_CMDLINE_OPTION p_cmdline_table[N_CMDLINE_OPTIONS];
#endif
{
  extern int optind, opterr;		/* GETOPT global variables */
  extern char *optarg;
  
  static  char *optstring = "ac:dhlmips--";	/* Command line option codes */
  static int previous_optind;
  int cl_opt_code;
  int option, start_code;
  
  /*
   * Parse input commandline, producing parsed table of command line options and argument values.
   * Usage: mtu [-a] [-c code] [-d] [-i] [-h] [-m] [-s] <mib-filename> ...
   *	 -a  Produce AGENT orientated MS**
   *	 -c <code> Provide starting EMA entity code
   *	 -d  Produce DNA CMIP orientated MS**
   *	 -h  Produce DECmcc help-file
   *	 -i  Produce MOMgen Info-file
   *	 -l  Produce Listing file
   *	 -m  Produce DECmcc director orientated MS**
   *	 -s  Produce SNMP orientated MS**
   *
   *     ** Switches not supported for CAU v1.0
   *
   * Use GETOPT to perform parse.  GETOPT output variables:
   *	optarg - option argument value
   *	optind - is read/write index into input list of arguments
   *	unget optional arg by decrementing  optind
   */
  
  /*
   ** Parse body of command line options and arguments
   */
  previous_optind = optind;					/* Remember initial index */
  while ((option = getopt (cmd_argc, cmd_argv, optstring)) != EOF) {
    switch (option) {
    case 'a' : {
      if (p_cmdline_table[OPT_DECMCC_MS].is_enabled) {
	fprintf (stdout, MSG(msg1, "MTU warning -- -m switch is superceded by -a switch\n"));
	p_cmdline_table[OPT_DECMCC_MS].is_enabled = FALSE;
      }
      p_cmdline_table[OPT_AGENT_MS].is_enabled = TRUE;
      break;
    }
    case 'c' : {
#if defined(sun) || defined(sparc)
      start_code = (int) strtol (optarg, 0, 10);
#else
      start_code = strtoul (optarg, 0, 10);
#endif
      if ((start_code > MAX_ENTITY_START_CODE) || (start_code < DEFAULT_ENTITY_START_CODE)) {
	fprintf(stdout, MSG(msg2, "MTU ERROR -- Invalid entity code value (%d) -\nMust be a number between %d and %d\n"),
		start_code,  DEFAULT_ENTITY_START_CODE, MAX_ENTITY_START_CODE);
	return FALSE;
      }
      p_cmdline_table[OPT_ENTITY_CODE].is_enabled = TRUE;
      p_cmdline_table[OPT_ENTITY_CODE].value.option_int = start_code;
      break;
    }
    case 'd' : {
      if (p_cmdline_table[OPT_SNMP_PROTOCOL].is_enabled) {
	fprintf (stdout, MSG(msg3, "MTU warning -- -s switch is superceded by -d switch\n"));
	p_cmdline_table[OPT_SNMP_PROTOCOL].is_enabled = FALSE;
      }
      p_cmdline_table[OPT_DNA_CMIP_PROTOCOL].is_enabled = TRUE;
      p_cmdline_table[OPT_DNA_CMIP_PROTOCOL].value.option_protocol = protocol_definition_table + DNA_CMIP_PROTOCOL;
      protocol_definition = protocol_definition_table + DNA_CMIP_PROTOCOL;
      break;
    }
    case 'h' : {
      p_cmdline_table[OPT_HELPFILE].is_enabled = TRUE;
      break;
    }
    case 'i' : {
      p_cmdline_table[OPT_INFOFILE].is_enabled = TRUE;
      break;
    }
    case 'l' : {
      p_cmdline_table[OPT_LISTFILE].is_enabled = TRUE;
      break;
    }
    case 'm' : {
      if (p_cmdline_table[OPT_AGENT_MS].is_enabled) {
	fprintf (stdout, MSG(msg4, "MTU warning -- -a switch is superceded by -m switch\n"));
	p_cmdline_table[OPT_AGENT_MS].is_enabled = FALSE;
      }
      p_cmdline_table[OPT_DECMCC_MS].is_enabled = TRUE;
      break;
    }
    case 's' : {
      if (p_cmdline_table[OPT_DNA_CMIP_PROTOCOL].is_enabled) {
	fprintf (stdout, MSG(msg5, "MTU warning -- -d switch is superceded by -s switch\n"));
	p_cmdline_table[OPT_DNA_CMIP_PROTOCOL].is_enabled = FALSE;
      }
      p_cmdline_table[OPT_SNMP_PROTOCOL].is_enabled  = TRUE;
      p_cmdline_table[OPT_SNMP_PROTOCOL].value.option_protocol = protocol_definition_table + SNMP_PROTOCOL;
      protocol_definition = protocol_definition_table + SNMP_PROTOCOL;
      break;
    }
      /*
       ** Error case
       */
    case '?' : {
      fprintf (stdout, MSG(msg6, "\nUsage: [-c code] [-i] [-l] <mibfile> ... \n"));
      fprintf (stdout, MSG(msg6a, "\t-c code\t Provide starting EMA entity code\n"));
      fprintf (stdout, MSG(msg6c, "\t-i \t Produce MOMgen Info-file\n"));
      fprintf (stdout, MSG(msg6d, "\t-l \t Produce listing file\n"));
      return FALSE;
    }
    } /* End switch */
  } /* End while-do */
  
  /* Retrieve MIB file specification from command line */
  if (cmd_argc > optind) {
    if (p_cmdline_table[OPT_MIB_FILENAME].value.option_string)
	free (p_cmdline_table[OPT_MIB_FILENAME].value.option_string);
    p_cmdline_table[OPT_MIB_FILENAME].value.option_string = (char *)malloc (SIZE_OF_FILENAME);
    strcpy (p_cmdline_table[OPT_MIB_FILENAME].value.option_string, *(cmd_argv+optind++));
  }
  else {
    if ((optind != 1) && (previous_optind == optind)) /* EOF case with at least one CMD */
      return FALSE;  
    fprintf (stdout, MSG(msg6, "\nUsage: [-c code] [-i] [-l] <mibfile> ... \n"));
    fprintf (stdout, MSG(msg6a, "\t-c code\t Provide starting EMA entity code\n"));
    fprintf (stdout, MSG(msg6c, "\t-i \t Produce MOMgen Info-file\n"));
    fprintf (stdout, MSG(msg6d, "\t-l \t Produce listing file\n"));
    return FALSE;
  }
  
  /*
   ** Process cmdline options table, setting non-null default values
   **	(ms = agent-style, protocol=snmp)
   */
  if ((!(p_cmdline_table[OPT_AGENT_MS].is_enabled)) && (!(p_cmdline_table[OPT_DECMCC_MS].is_enabled)))
    p_cmdline_table[OPT_AGENT_MS].is_enabled = TRUE;
  
  if ((!(p_cmdline_table[OPT_DNA_CMIP_PROTOCOL].is_enabled)) && (!(p_cmdline_table[OPT_SNMP_PROTOCOL].is_enabled))) {
    p_cmdline_table[OPT_SNMP_PROTOCOL].is_enabled = TRUE;
    p_cmdline_table[OPT_SNMP_PROTOCOL].value.option_protocol = protocol_definition_table + SNMP_PROTOCOL;
    protocol_definition = protocol_definition_table + SNMP_PROTOCOL;
  }
  return TRUE;
}

int init_prefix_oid_table()
{
  int i, j;
  char s1[255], s2 [225];
  int *oid;
  char *oid_string_ptr;

  for (i=0; prefix_table[i].oid_length != '\0'; i++) {
    oid = prefix_table[i].oid;

    s1[0] = '\0';
    for (j=0; j < prefix_table[i].oid_length; j++) {
      sprintf (s2, "%d ", oid[j]);
      strcat (s1, s2);
    }
   s1[strlen(s1)-1] = '\0';
   oid_string_ptr = malloc (strlen(s1)+1);
   strcpy (oid_string_ptr, s1);
   *(prefix_table[i].oid_string) = oid_string_ptr;
  }
  return 1;
}

/*********************************************************************************
 * main - Main routine for ECA_MTU
 *
 *	Description:  
 *		 perform initialization
 *		 while (get_cmd) {
 *			open i/o files
 *			parse MIB file, producing mib-parse-tree
 *			transform mib-parse-tree, producing entity-tree
 *			process entity-tree, producing MSL file [and CA MOMGEN info file] [and DECmcc helpfile]
 *			close i/o files
 *			}
 * 	Returns:
 *    		0 - success
 *		1 - failure
 *********************************************************************************/
main 
#ifdef PROTOTYPE_ALLOWED 
  (int argc, char **argv)
#else
(argc,argv)

int argc;
char **argv;
#endif

{
  char filename[SIZE_OF_FILENAME], tmp_filename[SIZE_OF_FILENAME];
  char *ptr;
  char *envvar_ptr;
  int  status;
  
  /*
   ** Load program logical (`control switch register')
   */
  ECA_MTU_LOG = 0;
  if ((unsigned int)(envvar_ptr = (char *)getenv ("ECA_MTU_LOG")))
    ECA_MTU_LOG = strtol(envvar_ptr, 0, 16);
  
#if VMS
  if (ECA_MTU_LOG & MTU_M_FAKE_VM) {
    FAKE_VM_INTERCEPT_XFER_VECTOR();
  }
#endif
  
#ifdef NL
  /*
   ** Open NLS message catalog file 
   ** Initialize NLS locale enviroment
   */
  _m_catd = catopen("mtu.cat", NL_CAT_LOCALE);
  setlocale(LC_ALL, "");
#endif
  
  /*
   * Perform license check for Common Agent and/or DECmcc
   * 	CA Base Kit license,
   * 	CA Developer Kit license, or
   *	DECmcc TCPIP AM license 
   *
   * For short-term just perform license checks on ULTRIX
   */
#if defined(__ultrix) || defined(__osf__) || defined(sun) || defined(sparc)
  if ((ca_check_base_license()) != 0) {
#ifdef VMS 
    /* The follow constant is defined in MCC_TCPIP_AM_MGT_IF.H */
# define MCC_K_CLASS_TCPIP_AM 17
    if (!((mcc_check_license (MCC_K_CLASS_TCPIP_AM)) & 1)) {
      fprintf(stdout, MSG(msg7, "\nMTU error -- Licence for DECMCC TCPIP AM is not loaded,"));
#endif      
      fprintf (stdout, MSG(msg8, "\nMTU error -- License for Common Agent Base Kit is not loaded, or\nMTU error -- License for Common Agent Developer Kit is not loaded.\n"));
      exit(1);
#ifdef VMS 
    }
#endif
  }	/* End of license check */
#endif  
  
  /*
   ** Initialize data structures
   **	MIB-tree, entity-tree, typedef-list and user-command-table
   */
  mib_root = 0;							
  entity_root = 0;
  typedef_object_list[0] = typedef_object_list[1] = 0;
  memset(cmdline_table, 0, sizeof(cmdline_table));
  init_prefix_oid_table ();  
  /*
   ******************************************************************************************************
   ** Process User Command line ( body of User commands ),
   ** 	Producing MSL files [,listing files] [, DECmcc helpfiles] [, CA MOMGEN information files]
   ******************************************************************************************************
   */
  status = FALSE;    /* Assume processing failure */
  while (get_cmdline(argc, argv, cmdline_table)) {
    
    /*******************************************************************************************
     **		Open i/o files
     ********************************************************************************************/
    /*
     ** Open Input MIB specification file, providing input stream for lexical analyzer (yyin)
     */
    strcpy (mib_filename, cmdline_table[OPT_MIB_FILENAME].value.option_string);
    if ((yyin = fopen(mib_filename,"r")) == NULLPTR) {
      fprintf(stdout, MSG(msg9, "MTU error -- Failure opening MIB input file (%s)\n%s\n"), mib_filename, strerror(errno));
      exit(1);
    }
    
    /*
     ** Transform full MIB file specification, producing root(filename)
     */
#if VMS
    getname(fileno(yyin), tmp_filename);
    if ((ptr = (char *)strrchr(tmp_filename, ']')) == NULLPTR)
      strcpy(filename, tmp_filename);
    else
      strcpy(filename, ptr+1);
#else
    if ((ptr = (char *)strrchr(mib_filename, '/')) == NULLPTR)
      strcpy(filename, mib_filename);
    else
      strcpy(filename, ptr+1);
#endif
    if ((ptr = (char *)strrchr(filename, '.')) != NUL) *ptr = NUL;
    
    /*
     ** Open MSL file, providing output stream (ms_stream)
     */
    sprintf (ms_filename, "%s.ms", filename);
    if ((ms_stream = fopen (ms_filename, "w")) == NULLPTR) {
      fprintf(stdout, MSG(msg10, "MTU error -- Failure opening ms file (%s)\n%s\n"), ms_filename, strerror(errno));
      exit(1);
    }
    
    /*
     ** Conditionally process opening DECmcc HELP file
     ** and Write help file header
     */
    if (cmdline_table[OPT_HELPFILE].is_enabled) {
      sprintf (help_filename, "mcc_%s.help", filename);
      if ((help_stream = fopen (help_filename, "w")) == NULLPTR) {
	fprintf(stdout, MSG(msg11, "MTU error -- Failure opening help file (%s)\n%s\n"), help_filename, strerror(errno));
	exit(1);
      }
      fprintf (help_stream, MSG(msg12, 
				"\n! Help file for %s.ms :\n! Produced by Polycenter Common Agent MIB Translation Utility, version %s\n\n"),
	       filename, mtu_version);
      fprintf (help_stream, "<COMMON_TOPICS>\n");
    } /* End-if */
    
    /*
     ** Conditionally process opening MOMGEN informational file
     ** and Write MOMGEN information file heading
     */
    if (cmdline_table[OPT_INFOFILE].is_enabled) {
      sprintf (info_filename, "%s.par", filename);
      if ((info_stream = fopen(info_filename, "w")) == NULLPTR) {
	fprintf(stdout, MSG(msg13, "MTU error -- Failure opening infofile (%s)\n%s\n"),
		info_filename, strerror(errno));
	exit(1);
      }
      fprintf(info_stream, "OID_Type= %s\n", protocol_definition->protocol_smi);
      fprintf(info_stream, "Num_threads = 1\n");
      fprintf(info_stream, "MOM_name = %s\n", filename);
      fprintf(info_stream, "Copywright = <<copywright owner>>\n");
      fprintf(info_stream, "Author = <<author>>\n");
      fprintf(info_stream, "Organization = <<organization>>\n");
      fprintf(info_stream, "Facility = <<facility>>\n");
    } /* End-if */
    
    /* 
     ** Conditionally open translation listing file (./<filename>.lis)
     ** and write heading
     */
    if (cmdline_table[OPT_LISTFILE].is_enabled) {
      sprintf (log_filename, "%s.lis", filename);
      if ((yyout = fopen(log_filename,"w")) == NULLPTR) {
	fprintf(stdout, MSG(msg14, "MTU error -- Failure opening translation log_filename (%s)\n%s\n"), 
		log_filename, strerror(errno));
	exit(1);
      }
      sprintf(log_header_str1,MSG(msg15, "\n\tPolyCenter Common Agent MIB Translation Utility %s"), mtu_version);
      fprintf(yyout,MSG(msg16, "%s\tPage %2d\n\n"), log_header_str1, ++log_pageno);
      sprintf(log_header_str2, MSG(msg17, "RFC Input: %s  Management Specification output: %s\n\n"), mib_filename, ms_filename);
      fprintf(yyout, log_header_str2);
      fprintf(yyout,MSG(msg18, "Section 1\tRFC Listing:\n\n"));
      fprintf(yyout,"%4d\t", yylineno);
    }
    
    /*
     ******************************************************************************************************
     ** Parse MIB input file (RFC) and produce MIB parse-tree (mib_root) and typedef_object_list.
     ******************************************************************************************************
     */
    fprintf (stdout, MSG(msg0,"MTU INFORMATIONAL -- Translating MIB file (%s)\n"), mib_filename);
    init_mibtree();				/* Initialize MIB tree and typedef list */
    init_entitytree();				/* Initialize Entity tree */
    status = FALSE;				/* Assume failure */
    mib_error_count = 0;			
    
    /* 
     ** Begin MIB processing:  POSIT valid mib, QUIT if error occurances exceed threshold
     */
    if (setjmp(mib_env) == 0) {
      yyparse();					/* Perform YACC parse of MIB file */
      if (yynerrs == 0) {				/* If successful parse, then ...*/
	/*
	 * Transform MIB parse-tree, producing EMA parse-tree
	 */
	if (produce_entity_tree (&mib_root, &entity_root)) {
	  /*
	   ** If successful, then
	   ** Process entity-tree and generate .MS files [,helpfile] [, MOMGEN informational file]
	   */
	  status = produce_ms_file(&entity_root);
	}
      }
    }						/* End of POSIT */
      
    else {					/* ADMIT error-occurances exceed threshold */
      fprintf (stdout, MSG(msg61,"MTU ERROR -- Aborting translation after %d errors\n"), ERROR_THRESHOLD);
    }
      
    fclose (yyin);				/* Close MIB input file */
    fclose (ms_stream);				/* Close MSL file */
    if (info_stream) fclose (info_stream);	/* Close MOMgen Infofile */
    if (help_stream) fclose (help_stream);	/* Close DECmcc helpfile */
    if (cmdline_table[OPT_LISTFILE].is_enabled) fclose (yyout);	/* Close log file */
      
    /* Report Command status */
    if (status)
      fprintf (stdout, MSG(msg19, "MTU SUCCESS -- Success translating MIB (%s)\n\n"), mib_filename);
    else 
      fprintf (stdout, MSG(msg20, "MTU ERROR -- Failure translating MIB (%s)\n\n"), mib_filename);
  }
  
#if VMS
  /*
   ** If VMS Fake VM reporting requested, then 
   **	free known vm; produce vm usage histogram and vm leak reports
   */
  if (ECA_MTU_LOG & MTU_M_FAKE_VM) {
    init_mibtree();					
    init_entitytree();				
    FAKE_VM_SET_FILENAME("eca_mtu.vm_use");
    FAKE_VM_HISTOGRAM();
    FAKE_VM_SET_FILENAME("eca_mtu.vm_leaks");
    FAKE_VM(1);
  }
#endif
    
  /*
   ** Return status of last user command
   */
  if (status) return (0);
  else return (1);
}
