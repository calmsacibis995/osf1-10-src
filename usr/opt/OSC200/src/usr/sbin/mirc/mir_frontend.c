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
static char *rcsid = "@(#)$RCSfile: mir_frontend.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/10/08 16:13:44 $";
#endif
/*
 * Copyright © (C) Digital Equipment Corporation 1990, 1991, 1992, 1993.
 * All Rights Reserved.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * MODULE DESCRIPTION:
 *
 * Common Agent Management Information Repository
 *
 * Module MIR_FRONTEND.C
 *      Contains functions that make up the "front-end" of the MIR MSL
 *      compiler.  These function provide for parsing the command line,
 *      processing the "builtin_types.dat" file and opening the MSL files
 *      for compilation in turn.
 *
 * WRITTEN BY:
 *    Networks & Communications Software Engineering - B. M. England
 *
 * MODIFIED BY:
 *    Enterprise Management Architecture/DECWest Engineering
 *    D. D. Burns   January 1991
 *
 * BIRD'S EYE VIEW:
 *    Context:
 *       The Common Agent Protocol Engines have need of network management
 *       architecture information in order to properly process incoming
 *       management request messages.  This information must reside in a
 *       Management Information Repository, accessible by ISO Object ID.
 *       The information is derived from MSL files based on SMI documents.
 *
 *    Purpose:
 *       This module contains the functions that make up the "front-end" of
 *       the MIR's MSL Compiler which is used to compile the MSL files that
 *       contain the descriptions (of the objects to be managed) to be
 *       loaded into a MIR.
 *
 *       See the Common Agent Management Information Repository
 *       Functional Specification document for details.
 *
 * History
 *   Version    Date            Who             What
 *      V0.0    January 1990    D. D. Burns     (Lifted 90%+ from B.M.E's
 *                                               DECnet-Ultrix Compiler)
 *                                     
 *      V1.0    July 1991       D. D. Burns     Add support for wildcarding to
 *                                               VMS version
 *
 *      V1.6    Sept 1991       D. D. Burns     Add support for resolving
 *                                               forward references.
 *      V1.90   June 1992       D. D. Burns     Conversion to MCC syntax
 *
 *      V1.91   July 1992       D. D. Burns     Support for -f and -b cmd-line
 *                                               switches
 *
 *      V1.95   July 1992       D. D. Burns     Support for Binary file format
 *                                               "2"
 *
 *      V1.96   July 1992	P Burgess       Added debugging support
 *
 *      V1.98   Oct 1992        P Burgess       Added spt for calling MTU
 *                                               for non-MSL file processing.
 *
 *      V1.99   Oct 1992        D. D. Burns     Support for AUGMENT & MERGE
 *
 *      V2.00   Jan 1993        D. D. Burns     Port to Alpha per Adam Peller
 *      V2.00   Feb 1993        D. D. Burns     Fix include-file logic problems

Module Overview:

All the functions that perform I/O for the compiler and the main loop for
the compiler are found in this module.


MODULE CONTENTS:

USER-LEVEL INTERFACE:
Function Name           Synopsis
main()                  Main function of the MIR Compiler.

yyerror                 Handles Errors for yacc generated grammer
                         code in "mir_yacc.y/c".

warn                    Handles warnings for yacc generated grammer
                         code in "mir_yacc.y/c".

info                    Handles info-level messages for yacc generated grammer
                         code in "mir_yacc.y/c".

open_include_files      Handles opening the INCLUDE statement in MSL files

close_include_files     Handles closing the INCLUDED files in MSL files

inputch                 Called by LEX-generated lexical analyzer code
                         to obtain next character of input

set_top_file            Establishes top of MSL include-file stack

get_cur_file            Returns currently open MSL file name

mirf_elapsed            Returns elapsed time as a string (for user msgs)


INTERNAL FUNCTIONS:
mirf_cmd_args           Parse Command Line Arguments
next_ms_filename        Processes the next filename, either thru a call to
                         MTU or by allowing MIRC to do it's thing.
mirf_next_filename      Returns next file name in a VMS/ULTRIX independent
                         fashion from command line
*/

#ifdef VMS
#include "vaxcshr.h"    /* translate from VAX C to DEC C RTL names */
#endif

/* Module-wide Stuff */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/* For Internationalization */
#ifdef NL
#include <langinfo.h>
#include <locale.h>
#include <nl_types.h>
#endif


/*
| Maximum Filename Size
*/
#define MAX_FILEN_SIZE 200

/*
| Include this stuff to make VMS cmd-line wildcarding work (LIB$FIND_FILE)
| in function mirf_next_filename().
*/
#ifdef VMS
#include <descrip.h>
#include <rmsdef.h>
#include <lib$routines.h>
#endif

/* Request definitions for compiler modules from "mir.h" */
#define MIR_COMPILER
#include "mir.h"


#ifndef VMS
/* For "next_ms_filename()": */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifndef strcasecmp
extern int strcasecmp PROTOTYPE((const char *s1, const char *s2));
#endif
#ifndef vfork
extern int vfork();
#endif
#endif

/*
|  Defined in mir_yacc.y -- needed here to see if an error occurred
*/
extern int eflag;


/*
|  Defined in mir_yacc.y -- needed here to enable a reset to "CM_NORMAL"
|  or "CM_MERGE" (in "mirf_next_filename()").
*/
extern CM_TYPE  compiler_mode;


/* Augment Support
|
|  These are defined in mir_yacc.y, and control the operation of the
|  compiler when an AUGMENT MSL file is parsed.
|
|  For "normal" or "merge" compilations on a standard MSL file, both of
|  these cells are set to zero and remain zero during the compilation of
|  a file.
|
|  For a compilation of an "Augment" MSL file, both are set to zero by
|  the frontend code here, and the mir_yacc.y parser code sets
|  "augment_max_pass" to the number of passes that the parser will have
|  to make over the file in order to process all the entity-classes named
|  in it.  At the end of each pass, "augment_pass" is incremented.  Code in
|  the frontend here is responsible for ending the passes when
|  "augment_max_pass" is reached.
*/
extern int  augment_pass;
extern int  augment_max_pass;


/*
|  These global locations corresponds directly to the "-Xi" compiler
|  and "-Xt" flags, and may be set TRUE by "mirf_cmd_args()".
*/
BOOL    supplementary_info=FALSE;
BOOL    supplementary_time=FALSE;
extern  BOOL    ad_hack_switch;         /* Defined in mir_yacc.y */


/*
|  This global location gets set TRUE if the undocumented, development-only
|  "-Xb" switch is supplied on the command-line in the initial-switches area.
|
|  TRUE: Try to push out a binary file regardless of error status of the
|        compilation (could crash the compiler).
*/
BOOL    force_binary_output=FALSE;


/* Function Prototypes for this Module */

/* mirf_cmd_args - Parse Command Line Arguments */
void mirf_cmd_args PROTOTYPE((
char    **,     /* MIR binary output File Name                     */
char    **,     /* MIR symbolic dump File Name                     */
char    **,     /* Binary input MIR Database File Name             */
IDX     **,     /* --> Hdr of list of keywords selected via "-k"   */
int     *,      /* Index into argv[] to next command line argument */
int      ,      /* Count of arguments supplied by user on cmd-line */
char    *[]     /* The command line arguments themselves via ptr   */
));

/* mirf_next_filename - Return next filename string */
char *
mirf_next_filename PROTOTYPE((
int     *,            /* Index into argv[] to next command line argument */
int     ,             /* Count of arguments supplied by user on cmd-line */
char    *[],          /* The command line arguments themselves via ptr   */
back_end_context *    /* Pointer to back-end context block               */
));

/* next_ms_filename - Return next MSL filename */
char *
next_ms_filename PROTOTYPE((
int     *,           /* Index into argv[] to next command line argument */
int     ,            /* Count of arguments supplied by user on cmd-line */
char    *[],	     /* The command line arguments themselves via ptr   */
back_end_context *   /* Pointer to back-end context block               */
));

#if DEBUG
unsigned int mcc_msl_log;
# define mcc_msl_debug_lex 1
#endif

/*
| Internationalization Message Phrase "Externs"
*/
#ifdef NL
extern nl_catd _m_catd;         /* For the Message Catalog */
extern char *mp001();
extern char *mp002();
extern char *mp003();
extern char *mp004();
extern char *mp005();
extern char *mp006();
extern char *mp007();
/* extern char *mp008(); */     /* Not used anymore */
extern char *mp009();
extern char *mp010();
extern char *mp011();
extern char *mp012();
extern char *mp013();
extern char *mp014();
extern char *mp015();
extern char *mp016();
extern char *mp017();
extern char *mp018();
extern char *mp019();
extern char *mp020();
extern char *mp021();
extern char *mp022();
extern char *mp023();
extern char *mp024();
extern char *mp025();
extern char *mp026();
extern char *mp027();
extern char *mp028();
extern char *mp029();
extern char *mp030();
extern char *mp031();
extern char *mp032();
extern char *mp033();
extern char *mp034();
extern char *mp035();
extern char *mp036();
extern char *mp037();
extern char *mp038();
extern char *mp039();
extern char *mp040();
extern char *mp041();
extern char *mp042();
extern char *mp043();
extern char *mp044();
extern char *mp045();
extern char *mp046();
extern char *mp047();
extern char *mp048();
extern char *mp049();
extern char *mp050();
extern char *mp051();
extern char *mp052();
extern char *mp053();
extern char *mp054();
extern char *mp055();
extern char *mp056();
extern char *mp057();
extern char *mp058();
extern char *mp059();
extern char *mp060();
extern char *mp061();
extern char *mp062();
extern char *mp063();
extern char *mp064();
extern char *mp065();
extern char *mp066();
#endif



main(argc, argv)

int argc;
char *argv[];

/*

   "mirc" accepts the following command line syntax:

        .............................initial options.............................
   mirc [-o[<out-filename>] -d[<dmp-filename>] -k<kewyord> -X*  -I<Include Path>] . . . . 

                               Optional
                     Input Binary MIR Database file
        . . . . . . .     -b<Input MIR Filename>   . . . . . . . .

        ....MSL Filenames....  ..File of Filenames...  ....MSL Filenames....
        . . .<filenames> . . .   -f<file of names>     . . . <filenames> . . 

        .Remove E-C Specifier.  ....Include Path....   ....MSL Filenames....
         -r<Entity-Class OID>   -I<New Include Path>   . . . <filenames> . . 

        ...Merge MSL Filename....
        . . .-m<filename> . . . 

   The initial options, if present, must precede the MSL filenames.

        Here is some cluttered English to illuminate the cluttered diagram.

The diagram sez:

        * First you have "Initial Options" on the command line, all of which
          are optional.

        * Next you may optionally specify exactly one binary Input MIR
          Filename using the "-b" switch.  You use this when you want to
          add/remove something to/from an existing MIR database file.

        * The remaining command line may be filled with any combination of
                - ASCII MSL filenames,
                - Filelist Filenames (specified using the "-f" switch),
                - Entity-Classes to be removed (specified using "-r" switch)
                - New Include Path(s) applying to all subsequent ASCII
                  MSL files until another Include Path is specified (using 
                  the "-I" switch).
                - ASCII MSL files prefaced with the -m Merge switch

Initial Options:
================

-o[<out-filename>]      If present, this option controls whether or not a
                        binary MIR output file is produced and what name it
    takes.  If this option is omitted, then the MIR compiler creates a binary
    output datafile by default with name "mir.dat" if no errors were
    encountered during compilation.

    If just -o is present with no filename, then no output file is produced,
    and no dump can be created (see -d below), although diagnostics from the
    compilation are output to stderr.

    If "-oxxx.dat" is submitted, diagnostics appear on stderr and if no errors
    were detected, the binary MIR output file created is "xxx.dat".


-d[<dmp-filename>]      If present, this option (on a successful compilation
                        with no errors) triggers the creation of a symbolic
    dump of the generated output file.

    This option uses the generated output file as input.  If no output file is
    created for some reason, then no dump is produced.  The symbolic dump is
    generated to a file whose name is optionally specified immediately after
    the -d, as in -ddata.dmp produces symbolic dump output to file "data.dmp".

    If -d is specified alone, then the symbolic dump filename is derived from
    the filename used to receive the MIR binary output file.  By stripping any
    portion of the MIR binary output filename that follows the last ".", a
    "basename" is created.  The symbolic dump is directed to a file named
    "basename.dmp".  (If there is no "." in the MIR binary output filename,
    the "basename" is simply the entire name).


-k<keyword>             If present, this option specifies a keyword for
                        INCLUDE statements in the MSL files.  Specifying the
    keyword allows the INCLUDE file to be compiled.  Any number of these -k
    switches may be supplied on the command line.


-I<Include-Path>        The compiler attempts to open all include files in
                        the current working directory.  If such an attempt
    fails, then if present, this option specifies a string that is prefixed
    onto the name of an include file before a second attempt to open the
    include file is made.  If more than one of these switches is placed on
    the command line, only the last one is effective.   The last one is
    in effect for all subsequent MSL file names mentioned until another
    -I occurence specifies a new value.  This switch may be embedded in a
    "file-list" file (see "-f" below).


Input Files and Switches:
========================

-b<binary MIR file>     Following any of the initial options shown above, an
                        optional binary MIR database file name may be supplied
    preceded by the "-b" switch.   The contents of this file is read in by
    the compiler and forms the basis for further compilation of ASCII MSL
    files (see "The Entity-Class Definition-Ordering Rule" below).  Only one
    binary MIR database file may be specified for the compilation.

Following any optional binary input filename, the filenames of ASCII MSL files
to be compiled are supplied, either directly on the command line (where 
wildcards are allowed) or in a "file-list" file.  All ASCII MSL file names
should end in the extension ".ms".  Under Ultrix, files end in ".mib" are
presumed to be candidates for processing by "MTU", the MIB Translation Utility,
and will be passed to that program for pre-processing before being processed by
MIRC.

-m<MSL filename>        Specifies the name of an ASCII MSL file that is to be
                        compiled in "merge-mode" instead of "normal-mode".  The
    compiler ignores MSL syntax in a "merge-mode" MSL file that describes any
    MIR object that has already been compiled (either in a previously compiled
    ASCII MSL file or by way of a binary MIR database file input to this
    compilation).


-f<file-list file>      Specifies the name of a "file-list" file that contains
                        the full path and filenames of ASCII MSL files to be
    compiled.  Each ASCII MSL filename must appear on a separate line in
    the file.  Lines in this file beginning with "#" or "!" are comment lines
    and are ignored by the compiler as are empty lines.  "-I" and "-r"
    switches may also appear in the file, one to a line.  Any number of
    "file-list" files may be used, each with a separate "-f" switch to
    introduce it on the command line.  Wildcards are not allowed as part of
    filenames specified within the "file-list" file.


-r<Entity-Class OID>    Specifies an Entity-Class by Object Identifier to be
                        removed from the MIR.  The Entity-Class *and all of
    its children* are removed.  Only Entity-Classes may be removed.  The
    Object Identifier is specified immediately after the "-r" with no
    intervening space.  The arcs of the OID may be separated by a period
    (".") or a blank.  The removal of the Entity-Class occurs immediately
    after the switch is encountered on the command line and before the next
    switch or file name is processed.   This has significance for
    "The Entity-Class Definition-Ordering Rule" (see below).  This switch
    may be used within a "file-list" file (see "-f" above), on a separate line
    within the "file-list" file.


The Entity-Class Definition-Ordering Rule:
------------------------------------------

The compiler requires that the definition of each CHILD Entity (supplied in an
ASCII MSL file) must *follow* the definitions of all of it's parent entities.


This implies all of the following:

* One or more parent definitions may precede a CHILD entity definition inside
  one ASCII MSL file, (in which case the CHILD entity definition will actually
  be nested inside the parent definitions in the file).

* The parent definitions may be contained in one or more ASCII MSL files
  processed by the compiler before the ASCII MSL file containing the CHILD
  entity is processed.

* The parent definitions may be contained in the input binary MIR database file
  (specified with the "-b" switch).

* Parent definitions may be defined by any combination of the above three
  methods.

* If no input binary MIR database file is specified, then the first file
  supplied *must* be an MSL file for a "global" entity.  All succeeding
  filenames supplied must be for child entities of the global entity or must
  define another global entity.


To compile two global entities (global0.ms and global1.ms) and their child
entities into one MIR output file, enter:

    mirc global0.ms  *0.ms  global1.ms  *1.ms


Further Examples:

    mirc -d global.ms

        ...compiles global entity MSL file "global.ms" into output file
        "mir.dat" (named by default) and produces a symbolic dump in file
        "mir.dmp" (named by derivation and triggered by -d).


    mirc -oms1.dat -d global1.ms

        ...compiles global entity MSL file "global1.ms" into output file
        "msl1.dat" (named explicitly) and produces a symbolic dump in file
        "msl1.dmp" (named by derivation).


    mirc -d -odna.dat node.ms *.ms

        ...compiles global entity MSL file "node.ms" and all the child
        entity files into output file "dna.dat" (named explicitly) with
        symbolic dump in "dna.dmp" (named by derivation).


    mirc -o node.ms child*.ms

        ...compiles global entity MSL file "node.msl" and all the child
        entity files.  Only diagnostics are produced to stderr since the
        output file creation is suppressed.


    mirc -d -boldmir.dat -I/usr/include/ ascii.ms

        ...reads in the existing binary MIR database file "oldmir.dat" and
        then compiles "ascii.ms".  The output file "mir.dat" (named by
        default) receives the contents of "oldmir.dat" along with that of
        "ascii.ms".  Any INCLUDE files specified within the body of
        "ascii.ms" will be sought first in the current working directory
        and failing that in the place specified by prepending the string
        "/usr/include/" to the front of the Include file name.  Note that
        the trailing "/" must be specified under ULTRIX.  "-d" specifies
        a symbolic dump directed to "mir.dmp" (named by derivation).


    mirc -boldmir.dat -I/usr/include/ ascii0.ms -I/usr/header/ ascii1.ms

        ...reads in the existing binary MIR database file "oldmir.dat" and
        then compiles "ascii0.ms" while searching for INCLUDE files in
        directory "/user/include/".  The INCLUDE directory is then switched
        to "/usr/header/" and "ascii1.ms" is compiled.  Binary output is
        produced to "mir.dat", no symbolic dump is produced.


    mirc -boldmir.dat -fmsl.list

        where file "msl.list" contains:

        #
        # File List file "msl.list"           --comments
        #

        ! Specify current INCLUDE directory   --more comments
        -I/usr/include/ 

        ! Specify ASCII MSL file to compile
        ascii0.ms

        ! Switch INCLUDE directory to new place
        -I/usr/header/

        ! Specify the last ASCII MSL file to compile
        ascii1.ms


        ...accomplishes the same result as the previous example but through
        the use of a "file-list" file.


    mirc -boldmir.dat -fremove.list

        where file "remove.list" contains:

        #
        # Removal specified for the following Entity-Classes:
        #
        -r1.3.12.2.1011.2.1.1
        -r1.3.12.2.1011.2.2.1.4.0.126.0
        -r1.3.12.2.1011.2.2.1.126.1

        ...reads in the existing binary MIR database file "oldmir.dat" and
        removes the three Entity-Classes specified by OID using the "-r"
        switch.  The binary output is generated to "mir.dat" (named by
        default) and contains whatever remains from "oldmir.dat" after the
        Entity-Classes have been removed.


Diagnostic output is produced to stderr.

------------------------------------------------------------------------------

Undocumented Compiler Developer Switches:

-Xi                     If present, the compiler provides supplementary
                        information about the process of the compilation,
    announcing the creation of each DNA MIR object that receives an ISO
    Object ID, showing the DNA Code and Text Name given to the object.


-Xt                     If present the compiler provides supplementary 
    wall-clock timing information on some of its processing.


-Xb                     If present, this forces the compiler to try to
                        generate a binary output MIR database file that
    would have otherwise been suppressed due to compile-time errors.
    A binary output file generated by "-Xb" in the face of compile-time
    errors should be considered corrupt, and may cause the compiler to
    segment-fault when trying to generate it or when QIM is trying to dump
    it.  This is intended as an aide for the analysis of compiler bugs.  
    No dump is produced regardless of "-d" if there were errors.  To get a
    dump after the fact, use "QIM".  The binary file is marked with a bad
    "Endian-ness" indicator to prevent it from being loaded by the Tier 0
    initialization function.

-Xadhack                If present, this forces the compiler to
    "Apply-Defaults" in the same manner as the MCC MSL translator did.  It
    is hoped that this hack can eventually be removed.  See discussion in
    "mir_yacc.y".

NOTE: Each of these must be supplied separately;  you can not 'pile them up'
      as in "-Xitb", and they must all be supplied at the beginning of the
      line in the initial options.

*/

/* Select support for automatically calling CA-MTU under ULTRIX *only* */
#ifndef VMS
#define GET_NEXT_MS_FILE next_ms_filename
#else
#define GET_NEXT_MS_FILE mirf_next_filename
#endif

{
extern FILE *yyin, *yyout;
int status;
back_end_context  bec;    /* Back-End Context, storage for back-end of mirc  */
char    *output_fn;       /* MIR binary output File Name                     */
char    *symdump_fn;      /* MIR symbolic dump File Name                     */
char    *binary_in_fn;    /* Input Binary MIR Database File Name             */
int     next_cmdarg;      /* Index into argv[] to next command line argument */
char    *next_file;       /* Pointer to next file name to be processed       */
time_t  compile_start;    /* Start time of the compiler                      */
time_t  file_start;       /* Stort time for next file being compiled         */
time_t  now;              /* (a constantly changing) Current Time            */
char    f_elapsed[50];    /* String description of elapsed time for file     */
char    c_elapsed[50];    /* String description of elapsed time for compile  */
extern int yylineno;	  /* linkage to YACC-produced line number            */
IDX     *kw_list=NULL;    /* List of Keywords user selected w/"-k" switch    */

/*
| Grab compiler start-time
*/
time(&compile_start);

#if DEBUG
/*
 * Store possible environement control variable value in mcc_msl_log
 */
{
  static char *mcc_msl_log_s = "MIRC_LOG";
  char *ptr_envvar;
  mcc_msl_log = 0;
  ptr_envvar = (char *)getenv (mcc_msl_log_s);	/* Retrieve possible envvar */
  if (ptr_envvar != NULL) {
    mcc_msl_log = strtol (ptr_envvar, 0, 16); /* Convert string to longword */
    printf ("\n%s = %d\n", mcc_msl_log_s, mcc_msl_log);
  }
}
#endif

/* For Internationalization, open the catalog of messages */
#ifdef NL
setlocale(LC_ALL,"");
_m_catd = catopen("mirc.cat", NL_CAT_LOCALE);
#endif

/*
| Suck in the command line options up to the first ASCII MSL filename
|  (If we return, no errors found)
*/
mirf_cmd_args(&output_fn,      /* MIR Binary Output Filename to use          */
              &symdump_fn,     /* MIR Symbolic Dump Filename to use          */
              &binary_in_fn,   /* MIR Input Binary Filename to use           */
              &kw_list,        /* List of INCLUDE Keywords user has selected */
              &next_cmdarg,    /* Next argument index we're on               */
              argc, argv);     /* (Arguments)                                */


/*
| The following initializations handle the case where MIRC runs on VMS,
| since the mir_lec.c can't initialize yyin and yyout at compile time on
| VAX "C" compiler
*/
yyin = stdin;
yyout = stdout;


/* Init the compiler Back-End:
| 
|  * Initialize the "Back-End" Context Block used by all back-end functions
|
|  * Register the Intermediate Context block (in the Back-End Context Block)
|    with the Intermediate Module so Intermediate Module functions have a
|    context to work in.
|
|  * If no input binary MIR database file was supplied on the command line:
|
|     - Create a "root" MIR object that is the 'world' that is the parent of
|       the 'topmost-things' the compiler sees (e.g. Global Entity classes in
|       DNA)
|
|     - Build Intermediate Data Structures that represent MIR Relationship
|       Objects, put references to them on the compiler-internal list used to
|       look them up by-name during compilation, and register their OIDs in the
|       nascent index.
|
|     - Open the 'builtin_types.dat' file and parse it, thereby creating a
|       compiler-internal list of 'approved, builtin-datatypes and
|       builtin-templates' (as MIR objects in Intermediate Data Structure form)
|       for *each* SMI described in the file.  (Set the reference counts for
|       the MIR objects to 1 to be certain they all get written even if the MSL
|       files we compile don't reference them (so build-from-binary works OK)).
|       (Environment symbol "ECA_MIR_BT_FILE" may override the search in
|       the current directory for "builtin_types.dat" with another path and
|       filename).
|
|    Else if there was a binary MIR database file input:
|
|     - All existing MIR objects in the binary input MIR database file are
|       loaded, including the The World, all MIR Relationship objects and
|       the dataconstruct objects for each SMI that was defined in the
|       builtin-types file at the time the binary MIR database file was
|       compiled (which includes the list of valid keywords for INCLUDE stmts).
|
|     - The compiler-internal lists for the dataconstructs are re-built from
|       the loaded objects.
|
|     - The current 'builtin_types.dat' file is opened and parsed as far as
|       it's Version String which is extracted and compared to the Version
|       String for the original 'builtin_types.dat' file that was stored in
|       the input MIR database file.  If they differ, a warning is issued.
*/
bec.selected_keyword_list = kw_list;    /* Load user-selected keywords  */
mirc_init_backend(&bec, binary_in_fn);  /* Init the rest of the backend */


/* FYI-NOTE:
|  At this point, if no input binary MIR database file was supplied as input
|  the parser has already been used to parse the builtin-types file, and we
|  should be at the point between "smi_bit_decls" and "spec_start"
|  in grammar rule:
|       mgmt_spec :  smi_bit_decls
|                  | spec_start type_decls spec_body spec_end
|
|  If an input binary MIR database file was supplied, then the builtin-types
|  file has only been parsed as far as it's Version String.
*/

next_file = NULL;          /* No file to process yet                         */

status = 0;                 /* Assume "success" */

/* while last compile was OK and Input MSL filenames remain . . . */
while (   (status == 0)
       && (next_file = GET_NEXT_MS_FILE (&next_cmdarg,
                                          argc,
                                          argv,
                                          &bec))
       != NULL) {

    /* Pass the word to the user according to the style of the compile
    |
    |  At this point, the function behind the GET_NEXT_MS_FILE mask has either
    |  set the global "compiler_mode" cell (in "mir_yacc.y") to "CM_MERGE"
    |  or left it to it's initial default or default from the last file: NORMAL
    */
    if (compiler_mode == CM_NORMAL) {
        fprintf(stderr,
                MP(mp002,"\nmirc - Info: Compiling MSL file\n       \"%s\"\n"),
                next_file);
        }
    else {   /* it must be "CM_MERGE" */
        fprintf(stderr,
                MP(mp065,"\nmirc - Info: Merging MSL file\n       \"%s\"\n"),
                next_file);
        }

    /* Record start time for this file */
    time(&file_start);

    /*
    | Potentially loop if the file turns out to be an AUGMENT ENTITY MSL file
    |
    | If the file is a normal MSL file (being processed in "CM_NORMAL" or
    | "CM_MERGE" mode), the loop won't. . . and we'll have exactly one pass
    | which is all we want for a non-AUGMENT ENTITY file.
    |
    | For AUGMENT ENTITY files, "augment_pass" and "augment_max_pass" will be
    | set appropriately by the parser before it returns to cause proper
    | looping.  For a non-AUGMENT ENTITY file ("normal" MSL file), both are
    | left at the initial value of zero by the parser.
    */

    augment_pass = augment_max_pass = 0;        /* Assume it's not AUGMENT */

    do {   /* For every Entity-Class mentioned in AUGMENT ENTITY file */

        if (freopen( next_file, "r", stdin ) == NULL) {
            fprintf(stderr,
                    MP(mp001,
                     "mirc - Error: Failed to open MSL file\n       \"%s\"\n"),
                    next_file);
            exit(BAD_EXIT);
            }

        /* Reset line number for yacc */
        yylineno = 1;

        /* remember this is top-level file */
        set_top_file( next_file );

        /* Compile it! */
        status = yyparse();
        }
        while ( augment_pass < augment_max_pass) ;

    /* Reset the backend of the compiler in order to compile the next one */
    mirc_reset();

    /* Show compilation of that file complete */
    fprintf(stderr,
            MP(mp003,"mirc - Info: Ending Compilation of MSL file (%d lines)\n"),
            yylineno);

    if (supplementary_time == TRUE) {
        time(&now);
        mirf_elapsed((now-file_start), f_elapsed);
        mirf_elapsed((now-compile_start), c_elapsed);
        fprintf(stderr,
                "       Wallclock Elapsed Times - File: %s  Compile: %s\n",
                f_elapsed, c_elapsed);
        }
    }

/*
|  If there were no errors (or we're forcing binary output) 
|  and an output file was specified, do the final pass to
|  "externalize" our intermediate heap representation.
*/
if ((force_binary_output == TRUE || eflag == FALSE)
     && strcmp(output_fn, "") != 0) {

    /* Record start time for this operation */
    time(&file_start);

    fprintf(stderr, MP(mp004,"\nmirc - Info: Generating Binary Output to \"%s\"\n"),
            output_fn);
    if (e_gen_external (&bec.i_context, output_fn) != TRUE) {
        fprintf(stderr,
                MP(mp005,"mirc - Error: Binary output generation failed.\n"));
        }

    /* Compute and Print Elapsed Times */
    if (supplementary_time == TRUE) {
        time(&now);
        mirf_elapsed((now-file_start), f_elapsed);
        mirf_elapsed((now-compile_start), c_elapsed);
        fprintf(stderr,
                "mirc - Info: Wallclock Elapsed Times, Binary: %s  Compile: %s\n",
                f_elapsed, c_elapsed);
        }
    }
else {
    fprintf(stderr, MP(mp006,"mirc - Info: No binary output file produced\n"));
    }

/* if there was output and they specified an symbolic output filename */
if (eflag == FALSE && strcmp(symdump_fn, "") != 0
                   && strcmp(output_fn, "") != 0) {

    /* Record start time for this operation */
    time(&file_start);

    fprintf(stderr, MP(mp007,"\nmirc - Info: Generating Symbolic dump to \"%s\"\n"),
                symdump_fn);
    mirc_symdump(output_fn, symdump_fn, FALSE);    /* FALSE = not a raw dump */

    if (supplementary_time == TRUE) {
        /* Compute and Print Elapsed Times */
        time(&now);
        mirf_elapsed((now-file_start), f_elapsed);
        mirf_elapsed((now-compile_start), c_elapsed);
        fprintf(stderr,
                "mirc - Info: Wallclock Elapsed Times, Dump: %s  Compile: %s\n",
                f_elapsed, c_elapsed);
        }
    }

if (eflag == TRUE) {
    status = BAD_EXIT;
    }
else {
    status = GOOD_EXIT;
    }
return (status);
}

/*
 *
 * Copyright (C) 1989 by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 *
 * Networks & Communications Software Engineering
 *
 * input.c - module to implement read I/O for the MSL file
 */

/* static char *sccsid = "@(#)input.c	1.4	10/2/90"; */

  /* tell YACC what value of terminal string is */
extern int yylval;	/* linkage to YACC-produced y.tab.c module */
extern int yylineno;	/* linkage to YACC-produced line number */
extern FILE *yyin;	/* linkage to YACC-produced input file ptr */

  /*
   * The following stack of structures corresponds to the current
   * nesting of include files in the MSL translator's input stream.
   * An include file is pushed on the stack whenever an INCLUDE statement
   * is encountered, and it is popped off the stack whenever the end
   * of the include file is encountered.
   */

#define INCL_STACK_SIZE 25

static struct {
	char file_name[MAX_FILEN_SIZE];	/* name of include file */
	FILE *file_ptr;		/* pointer to include file */
	int lineno;		/* line number at which file was included */
	} include_stack[ INCL_STACK_SIZE ];
static int include_tos;	/* top-of-stack index */

/*
| These guys are used to parse "DEPENDS ON" clauses
*/
static int saved_yylineno = 0;
static char *use_memory = 0;
static char *depend_ctx = 0;

/* Full Include-File Names are built here   */
static char FileNameBuf[MAX_FILEN_SIZE];

/* Points to string prefixed to all include file names. */
static char *include_path=NULL;


/*
 * open_include_file - saves current line # in current input stream,
 * opens new file, and saves the current file pointer and new file name
 * before changing the lex input file to the new file.  Data is saved
 * on a stack to implement the LIFO behavior of include files.
 *
 ***************************************************************************
 */
MC_STATUS open_include_file( pFileName )
char *pFileName;
{
FILE *pNewFile;
char    *pFNwithinclude;      /* Pointer to filename with Include path in it*/
char    *cp;
int     filename_size;        /* How big the final include filename may be */

/* see if stack overflow occurred */
if (include_tos >= INCL_STACK_SIZE)
	{
	return (MC_INC_LIMIT_EXCEEDED);
	};

/* open the include file */

/*
| We build the full include file name by prefixing any Include File Path
| supplied on the MIRC command-line via the "-I" switch.
*/

/* Null Terminate the buffer in case of no Include Path string */
FileNameBuf[0] = '\0';

filename_size = strlen(pFileName);
if (include_path != NULL)
    filename_size += strlen(include_path);

if (filename_size > MAX_FILEN_SIZE) {
    yyerror(MP(mp009,"Include Filename (plus -I Include Path string) is too long"));
    /* failed to open it , but continue anyway */
    return (MC_OPENFAIL_CONTINUE);
    }

/* If we have an include path string, slap it into the build-buffer */
if (include_path != NULL)
    strcpy (FileNameBuf, include_path);

/* Build the final filename */
pFNwithinclude = strcat(FileNameBuf, pFileName);

#ifdef unix
for (cp = pFileName; *cp; cp++)
	{
	if (*cp == '$') *cp = '_'; /*strip $ signs */
	}
#endif

pNewFile = fopen( pFileName, "r" );     /* Try open w/o Include directory */

/* if we couldn't open in current directory. . . */
if (pNewFile == NULL) {

    /* Try open with include directory */
    pNewFile = fopen( pFNwithinclude, "r" );

    if (pNewFile == NULL) /* if we couldn't open */
	{
	perror( pFileName );
	yyerror( MP(mp010,"Failed to open include file") );

        /* failed to open it , but continue anyway */
	return (MC_OPENFAIL_CONTINUE);
	};

    /* Record the right file name of the file we really opened */
    pFileName = pFNwithinclude;
    }

/* save where we are */
include_stack[include_tos].file_ptr = yyin;
strcpy( include_stack[include_tos].file_name, pFileName );
include_stack[include_tos].lineno = yylineno;
include_tos++;

 /* switch to new input stream */
yylineno = 1;
yyin = pNewFile;

return (MC_SUCCESS);
}

/*
 * this routine closes the current input file, pops the include file stack
 * to find out where we were in the preceding input file, and resumes
 * processing at that point.  This routine returns 1 if the stack is
 * empty (i.e. end of the compilation), 0 otherwise.
 ***************************************************************************
 */
int close_include_file()
{

if (include_tos <= 1) /* if stack empty */
	return 1;

fclose(yyin);

/* pop the stack */
include_tos--;
yyin = include_stack[include_tos].file_ptr;
yylineno = include_stack[include_tos].lineno;
return 0;
}

/* ========================================================================= */
/*
 * the inputline routine reads a line of text from the current input file
 * (MSL file or an include file referenced therein).  If it hits end of file,
 * it attempts to pop the include file stack.  If this succeeds, it just
 * picks up where it left off in the popped file.  If not, it returns
 * end-of-file indication.
 *
 * The MSL compiler's lexical analyzer redefines LEX's input() macro
 * to call this routine.
 */
int inputch()
{
register int ch;

if (use_memory)
    {
    ch = *depend_ctx++;
    if (ch == '\0')
	{
	free(use_memory);
	use_memory = 0;
        yylineno = saved_yylineno;      /* Restore */
	}
    else
	return ch;
    }

/* ========================================================================= */
ch = getc(yyin);
if (ch != EOF) {
	return ch;}

/* if we've reached end of file */

if (close_include_file() == 0) /* if we popped an include file off the stack*/
	return inputch();

return EOF;
}


/* ========================================================================= */
void
parse_depends_expr(in_str)
char *in_str;
{
use_memory = in_str;
depend_ctx = in_str;
saved_yylineno = yylineno;      /* Save */
}


/* ========================================================================= */
void
set_top_file( pFileName )
char *pFileName;
{
strcpy( include_stack[0].file_name, pFileName );
include_tos = 1;
}


/* ========================================================================= */
get_cur_file( ppFileName )
char **ppFileName;
{
if (include_tos <= 0) /* if stack isn't set up right */
	*ppFileName = NULL;
else
	*ppFileName = include_stack[include_tos-1].file_name;
return (*ppFileName) ? 0 : 1;
}


/*
 *
 * error.c - module to perform error handling, etc. for MD compiler
 *
 * note that all output is directed to stderr
 * 
 */

/* static char *sccsid = "@(#)yacc_error.c	5.2	1/4/90"; */

extern int yylineno;	/* MD source line number */

/* to get YACC to print out line number of error */
void
yyerror(message)
char *message;
{
char *pFile;

eflag = TRUE;

/* If we're in the main file (ie, NOT an include file). . . */
if (include_tos == 1) {
    fprintf(stderr, MP(mp011,"mirc - Error at line %ld:\n       %s\n"),
            yylineno, message);
    }
else {  /* we're in an include file, give the name */
    get_cur_file( &pFile );
    fprintf(stderr,
            MP(mp012,"mirc - Error at line %ld in file:\n       \"%s\"\n       %s\n"),
            yylineno, pFile, message);
    }
}

void
warn(message)
char *message;
{
char *pFile;

/* If we're in the main file (ie, NOT an include file). . . */
if (include_tos == 1) {
    fprintf(stderr, MP(mp013,"mirc - Warning at line %ld:\n       %s\n"),
                   yylineno, message);
    }
else {  /* we're in an include file, give the name */
    get_cur_file( &pFile );
    fprintf(stderr,
            MP(mp014,"mirc - Warning at line %ld in file:\n       \"%s\"\n       %s\n"),
            yylineno, pFile, message);
    }
}


void info(message)
char *message;
{
char *pFile;

/* If we're in the main file (ie, NOT an include file). . . */
if (include_tos == 1) {
    fprintf(stderr, MP(mp015,"mirc - Info at line %ld:\n       %s\n"),
            yylineno, message);
    }
else {  /* we're in an include file, give the name */
    get_cur_file( &pFile );
    fprintf(stderr,
            MP(mp016,"mirc - Info at line %ld in file:\n       \"%s\"\n       %s\n"),
            yylineno, pFile, message);
    }
}

yywrap()
{
	return(1);
}

/* mirf_cmd_args - Parse Command Line Arguments */
/* mirf_cmd_args - Parse Command Line Arguments */
/* mirf_cmd_args - Parse Command Line Arguments */

void
mirf_cmd_args(output_fn,
              symdump_fn,
              binary_in_fn,
              kw_list,
              next_cmdarg,
              argc,
              argv)

char    **output_fn;      /* MIR binary output File Name                     */
char    **symdump_fn;     /* MIR symbolic dump File Name                     */
char    **binary_in_fn;   /* Binary input MIR File Name                      */
IDX     **kw_list;        /* --> Hdr of list of selected keywords            */
int     *next_cmdarg;     /* Index into argv[] to next command line argument */
char    *argv[];          /* The command line arguments themselves via ptr   */
int     argc;             /* Count of arguments supplied by user on cmd-line */


/*
INPUTS:

    "output_fn" is the address of a character pointer that will be set to
    point to the output filename to be used if output is to be generated.

    "symdump_fn" is the address of a character pointer that will be set to
    point to the symbolic dump output filename to be used if a dump is to
    be generated.

    "binary_in_fn" is the address of a character pointer that will be set to
    point to any binary MIR Input database filename to be used.  If none
    was supplied, a NULL is returned.

    "kw_list" is the address of a pointer to the first keyword IDX selected
    by the user.  This pointer is initially NULL on input.  If keywords for
    INCLUDE statements were supplied by the user on the command line, this
    pointer will be changed.

    "next_cmdarg" is the address of an integer that will be set to the number
    of the next cmd-line argument to be parsed.

    "argc" is the count of the number of arguments we were passed in argv[].

    "argv[]" is the array of pointers to the command line arguments.

OUTPUTS:

    The function returns:
        * the filenames to be used for binary MIR output (ifany) and
        * symbolic dump output (if any) and 
        * binary MIR database input (if any) using any user-supplied
          command-line arguments.
        * a non-null list of IDX blocks describing the keywords (if any)
          the user supplied using "-k".

    The index of the next argument (either a -f<filename> or a filename) is
    also returned.

    If the function returns, there were no errors.

BIRD'S EYE VIEW:
    Context:
        The caller is the main() loop in the front-end.  The command-line
        arguments need to be parsed.

    Purpose:
        This function takes care of the details of applying the rules about
        what filenames are to be used for compiler output.


ACTION SYNOPSIS OR PSEUDOCODE:

    if (count of arguments is less than 2)
        <issue "usage" message>
        <exit>

    <show "o" not seen>
    <show "d" not seen>
    <show "b" not seen>
    <show local IDX keyword list empty>
    <show "derive-d-name" FALSE>
    <set default symbolic dump filename to "">
    <set default output filename to "mir.dat">

    while (next-arg-index < total-count)

        if (first character is not "-" or first & second are "-f" or "-m")
            if (derive-d-name is TRUE)
                if (binary output filename is NOT "")
                    <construct local copy of derived symbolic dump name>
                    <set symdump output filename to point to it>
                else
                    <issue "No symbolic dump possible without binary output">
                    <exit>

            <return any keyword list>
            <return>

        <extract second character>

        switch (second character)

            case 'b':
                if (b not seen)
                    <show b seen>
                    if (length of remaining string > 0)
                        <set binary input fn to remainder>
                    else
                        <show binary input fn as "">
                else
                    <issue "erroneous multiple '-b' option>
                    <exit>
                <break>


            case 'd':
                if (d not seen)
                    <show d seen>
                    if (length of remaining string > 0)
                        <set symdump output fn to remainder>
                    else
                        <show derive-d-name TRUE>
                else
                    <issue "erroneous multiple '-d' option>
                    <exit>
                <break>


            case 'k':
                if (length of remaining string > 0)
                    if (attempt to allocate an IDX failed)
                        <issue "Out of Memory during initialization">
                        <exit>
                    <insert IDX into top of keyword IDX list>
                    <insert remainder of string into IDX "name">
                else
                    <issue "Missing Keyword for -k">
                    <exit>
                <break>

            case 'o':
                if (o not seen)
                    <show o seen>
                    if (length of remaining string > 0)
                        <set binary output fn to remainder>
                    else
                        <set binary output fn to "">
                else
                    <issue "erroneous multiple '-o' option>
                    <exit>
                <break>


            case 'i':
            case 'I':
                if (no Include Path string has been recorded)
                    <record Include Path string>
                else
                    <issue "Only one -I Include Path switch allowed">
                    <exit>


            default:
                <issue "erroneous option: ..">
                <exit>

        <increment next-cmd-index by one>

    <issue "missing input filename(s) on command line">
    <exit>


OTHER THINGS TO KNOW:

    This function has most of the jazz in it that determines how "mirc"
    responds to command line options.

*/

{
BOOL    b_not_seen;     /* TRUE: -b option not yet parsed       */
BOOL    o_not_seen;     /* TRUE: -o option not yet parsed       */
BOOL    d_not_seen;     /* TRUE: -d option not yet parsed       */
BOOL    derive_dn;      /* TRUE: Derive symbolic dump filename  */
int     i;              /* General index                        */
IDX    *keywords;       /* local Keyword IDX list header        */
IDX    *next_kw;        /* Next keyword IDX we're allocating    */


/* Static build area for derived dump filename */
static char local_fn[MAX_FILEN_SIZE];


/* if (count of arguments is less than 2) */
if (argc < 2) {
    /* issue "usage" message */
    printf(MP(mp017,"Usage for MIRC Version V%d.%02d: \n\n"), VERSION_MAJOR, VERSION_MINOR);

    printf(MP(mp018,"     ..............................initial options........................\n"));
    printf(MP(mp019,"mirc [-o[<out-filename>]  -d[<dmp-filename>]  -k<keyword> -I<include-path>]. . .\n\n"));

    printf(MP(mp020,"                              Optional\n"));
    printf(MP(mp021,"                   Input Binary MIR Database file\n"));
    printf(MP(mp022,"     . . . . . . .     -b<input MIR Filename>   . . . . . . . .\n\n"));

    printf(MP(mp023,"     ....MSL Files....  ..List of MSL files..   ...MSL files....\n"));
    printf(MP(mp024,"     ....<filename>...   -f<filelist name>      ...<filename>...\n\n"));

    printf(MP(mp025,"     .Remove E-C Specifier.  ....Include Path....   ....MSL Filenames....\n"));
    printf(MP(mp026,"      -r<Entity-Class OID>   -I<New Include Path>   . . . <filenames> . .\n\n"));

    printf(MP(mp027,"    -o               Suppress binary MIR output (diagnostics only)\n"));
    printf(MP(mp028,"    -o<filename>     Direct error-free output to binary <filename>.\n"));
    printf(MP(mp029,"    -d               Create symbolic dump upon error-free output to\n"));
    printf(MP(mp030,"                     filename derived from output filename (\".dmp\")\n"));
    printf(MP(mp031,"    -d<filename>     Create symbolic dump on error-free output to\n"));
    printf(MP(mp032,"                     specified <filename>.\n"));
    printf(MP(mp033,"    -k<keyword>      Specifies keyword for INCLUDE statement.\n"));
    printf(MP(mp034,"    -I<include-path> Specifies a path to a directory where include\n"));
    printf(MP(mp035,"                     files are sought.  (Only one -I switch active at once.)\n"));
    printf(MP(mp036,"    -b<MIR database> Specifies *one* input binary MIR Database file.\n"));
    printf(MP(mp037,"    -f<filelist>     Supply the name of a file as <filelist> that\n"));
    printf(MP(mp038,"                     contains the names of MSL files, one to a line.\n"));
    printf(MP(mp039,"                     Wildcards are not allowed, \"-r\" and \"-I\" may\n"));
    printf(MP(mp040,"                     be used one-to-a-line, \"#\" and \"!\" start\n"));
    printf(MP(mp041,"                     comment lines within the file.\n"));
    printf(MP(mp042,"    -r<E-C OID>      Specifies one Entity-Class & Children to be removed,\n"));
    printf(MP(mp043,"                     Object Identifier must specify Entity-Class\n"));
    printf(MP(mp066,"    -m<filename>     Specifies MSL <filename> to be 'merged' into compilation\n"));
    printf(MP(mp044,"     The definition of each CHILD Entity (supplied in an ASCII MSL file)\n"));
    printf(MP(mp045,"     must FOLLOW the definitions of all of it's parent entities.\n"));
    exit(GOOD_EXIT);
    }

b_not_seen = TRUE;      /* show "b" not seen */
o_not_seen = TRUE;      /* show "o" not seen */
d_not_seen = TRUE;      /* show "d" not seen */

derive_dn = FALSE;      /* show "derive-d-name" FALSE */
*binary_in_fn = NULL;   /* set default binary input filename to NULL  */
*symdump_fn = "";       /* set default symbolic dump filename to "" */
*output_fn = "mir.dat"; /* set default output filename to "mir.dat" */
keywords = NULL;        /* show local IDX keyword list empty */

*next_cmdarg = 1;       /* Skip the name "mirc" */

/* while (next-arg-index < total-count) */
while (*next_cmdarg < argc) {

    /* if (first char is not "-" or 1st two chars are "-f", "-m" or "-r") */
    if (*argv[*next_cmdarg] != '-'
        || (*(argv[*next_cmdarg] + 1) == 'f')
        || (*(argv[*next_cmdarg] + 1) == 'm')
        || (*(argv[*next_cmdarg] + 1) == 'r')
        ) {

        /* if (derive-d-name is TRUE) */
        if (derive_dn == TRUE) {

            /* if (binary output filename is NOT "") */
            if (strlen(*output_fn) != 0) {

                /* construct local copy of derived symbolic dump name */
                strcpy (local_fn, *output_fn);  /* Make local copy    */
                i = strlen(local_fn);           /* Get index to end   */
                while ( i > 0 ) {       /* Step backward to find "."  */
                    if ( local_fn[--i] == '.')
                        break;
                    }
                if (i == 0) {
                    i = strlen(local_fn);   /* Append on end if no "."*/
                    }
                strcpy (&local_fn[i], ".dmp");

                /* set symdump output filename to point to it */
                *symdump_fn = local_fn;
                }
            else {
                /* issue "No symbolic dump possible without binary output" */
                fprintf(stderr,
                        MP(mp046,"mirc - Error: No symbolic dump possible without binary output\n"));
                exit(BAD_EXIT);
                }
            }

        /* Return any keyword list */
        *kw_list = keywords;

        return;
        }

    /* extract second character */
    /* switch (second character) */
    switch (*(argv[*next_cmdarg] + 1)) {

        case 'b':

            /* if (b not seen) */
            if (b_not_seen == TRUE) {

                b_not_seen = FALSE; /* show b seen */

                /* if (length of remaining string  > 0) */
                if (strlen(argv[*next_cmdarg] + 2) > 0) {

                    /* set binary output fn to remainder */
                    *binary_in_fn = argv[*next_cmdarg] + 2;
                    }
                else {
                    fprintf(stderr,
                            MP(mp047,"mirc - Error: -b switch requires filename\n"));
                    exit(BAD_EXIT);
                    }
                }

            else {
                /* issue "erroneous multiple '-b' option */
                fprintf(stderr,
                        MP(mp048,"mirc - Erroneous multiple '-b' option\n"));
                exit(BAD_EXIT);
                }
            break;


        case 'd':

            /* if (d not seen) */
            if (d_not_seen == TRUE) {

                d_not_seen = FALSE;     /* show d seen */

                /* if (length of remaining string  > 0) */
                if (strlen(argv[*next_cmdarg] + 2) > 0) {

                    /* set symdump output fn to remainder */
                    *symdump_fn = argv[*next_cmdarg] + 2;
                    }
                else {
                    /* show derive-d-name TRUE */
                    derive_dn = TRUE;
                    }
                }
            else {
                /* issue "erroneous multiple '-d' option */
                fprintf(stderr,
                        MP(mp049,"mirc - Erroneous multiple '-d' option\n"));
                exit(BAD_EXIT);
                }
            break;


        case 'k':

            /* if (length of remaining string > 0) */
            if (strlen(argv[*next_cmdarg] + 2) > 0) {

                /* if (attempt to allocate an IDX failed) */
                if ((next_kw = (IDX *) malloc(sizeof(IDX))) == NULL) {
                    /* issue "Out of Memory during initialization" */
                    fprintf(stderr,
                            MP(mp050,"mirc - Out of Memory during initialization\n"));
                    exit(BAD_EXIT);
                    }

                /* insert IDX into top of keyword IDX list */
                next_kw->next = keywords;
                keywords = next_kw;

                /* insert remainder of string into IDX "name" */
                next_kw->name = argv[*next_cmdarg] + 2;
                }
            else {
                /* issue "Missing Keyword for -k" */
                fprintf(stderr,
                        MP(mp051,"mirc - Missing keyword for '-k' option\n"));
                exit(BAD_EXIT);
                }
            break;


        case 'o':

            /* if (o not seen) */
            if (o_not_seen == TRUE) {

                o_not_seen = FALSE; /* show o seen */

                /* if (length of remaining string  > 0) */
                if (strlen(argv[*next_cmdarg] + 2) > 0) {

                    /* set binary output fn to remainder */
                    *output_fn = argv[*next_cmdarg] + 2;
                    }
                else {
                    *output_fn = "";    /* set binary output fn to "" */
                    }
                }

            else {
                /* issue "erroneous multiple '-o' option */
                fprintf(stderr,
                        MP(mp052,"mirc - Erroneous multiple '-o' option\n"));
                exit(BAD_EXIT);
                }
            break;


        case 'i':  /* Under VMS, all unquoted uppercase characters get */
        case 'I':  /* down-cased, so we have to handle "I" as "i" here */
            /* if (no Include Path string has been recorded) */
            if (include_path == NULL) {
                /* record Include Path string */
                include_path = (argv[*next_cmdarg]+2);
                }
            else {
                /* issue "Only one -I Include Path switch allowed" */
                fprintf (stderr,
 MP(mp053,"mirc - Only one -I Include Path switch may be active at once.\n"));
                exit(BAD_EXIT);
                }
            break;


        case 'x':  /* VMS converts Uppercase cmd-line to lowercase */
        case 'X':
            /*
            |  Compiler Development Switch:
            |
            |  "-Xb" - If the developer supplied this switch, we'll try to
            |          force the generation of a binary output file for
            |          development-debugging purposes.
            |    NOTE: (The output may be junk!!  Don't let it get loose!)
            |
            |  "-Xi" - Supplementary Information on OIDs/Objects created
            |  "-Xt" - Wallclock times for certain phases.
            |  "-Xadhack" - Apply-Defaults Hack.  See the documentation
            |          in "mir_yacc.y".
            */
            if (*(argv[*next_cmdarg] + 2) == 't') {
                supplementary_time = TRUE;
                }
            else if (*(argv[*next_cmdarg] + 2) == 'i') {
                supplementary_info = TRUE;
                }
            else if (*(argv[*next_cmdarg] + 2) == 'b') {
                force_binary_output = TRUE;
                }
            else if (strcmp((argv[*next_cmdarg] + 2), "adhack") == 0) {
                ad_hack_switch = TRUE;
                }
            break;


        default:
            /* issue "erroneous option: .." */
            fprintf (stderr, MP(mp054,"mirc - Erroneous option: %s\n"),
                     argv[*next_cmdarg]);
            exit(BAD_EXIT);

        }

    /* increment next-cmd-index by one */
    *next_cmdarg += 1;
    }

/* issue "missing input filename(s) on command line" */
fprintf(stderr,
        MP(mp055,"mirc - Error: missing input filename(s) on command line\n"));
exit(BAD_EXIT);

}

/* Select support for automatically calling CA-MTU under ULTRIX *only* */
#ifndef VMS

char  *next_ms_filename (next_cmdarg, argc, argv, bec)

int     *next_cmdarg;     /* Index into argv[] to next command line argument */
int     argc;             /* Count of arguments supplied by user on cmd-line */
char    *argv[];          /* The command line arguments themselves via ptr   */
back_end_context *bec;    /* Pointer to back-end context block               */

{
  static  char ms_filename[MAX_FILEN_SIZE];    
  char *next_file;
  char *ptr, *base_ptr;
  pid_t pid, child_pid;

#ifdef __osf__
  int status;
#endif

#if defined(ultrix) || defined(__ultrix)
  union wait status;
#endif

  int ret_code;
  char *arg_vector[4];

/*
 ** Produce next .ms file from list of user command files
 ** If user specified file with a ".mib" file extension,
 ** then this routine attempts to transform the RFC mib definition
 ** to a .ms file file.
 **
 ** (We should spiff this up so that we have a more formal list of file
 **  extensions:  I believe global entity files should have a special
 **  extension, while files that are INCLUDEd have another, as well as
 **  yet another for AUGMENT files).
 */
  while (next_file = mirf_next_filename(next_cmdarg, argc, argv, bec)) {

/*
 ** If file extension is ".ms", then
 ** 	vfork.
 **	[child]:  - Perform mib-to-msl translation
 **	[parent]: - provide next msl file name
 */
    if ((ptr = (char*)strrchr(next_file, '.')) && 
	(strlen(ptr) == strlen(".mib")) && (!(strcasecmp(ptr, ".mib")))) {

      /* Perform VFORK */
      if ((child_pid = vfork()) == -1) {
	fprintf(stderr, MP(mp060,"mirc - Error: VFORK failure\n"));
	exit(BAD_EXIT);
      }

      /*
       ** If parent, then
       ** 	Wait for child to complete
       **	If Translation success, then return .ms file specification
       **	Else display error message and fetch next file-spec from input cmd stream
       */
      if (child_pid) { 
	if (pid = waitpid(child_pid, &status, 0) == -1) {
	  fprintf(stderr, MP(mp061,"mirc - Error: WAITPID failure\n"));
	  exit(BAD_EXIT);
	}

#ifdef __osf__
	if (WIFEXITED(status)) {
	  if (!(ret_code = WEXITSTATUS(status))) {
#endif

#if defined(ultrix) || defined(__ultrix)
	if (WIFEXITED(status.w_status)) {
	  if (!(ret_code = WEXITSTATUS(status.w_status))) {
#endif
	    if (base_ptr = (char*)strrchr(next_file, '/')) 
	      base_ptr++;
	    else 
	      base_ptr = next_file;
 
	    strncpy (ms_filename, base_ptr, (ptr - base_ptr));
	    ms_filename[(ptr - base_ptr)] = 0;
	    strcat (ms_filename, ".ms");
	    return (ms_filename);	       /* Return newly created .ms filename" */
	  }
	}
	else {
	  fprintf(stderr,
		  MP(mp062,"mirc - Error: MIB translation of %s failed\n"),
		  next_file);
	}
      }
      /*
       ** Else if Child path ...
       **	execve "/usr/(s)bin/mtu -i <filename.mib>"
       */
      else {	/* Child:	Execve ECA MTU */
#ifdef __osf__
	arg_vector[0] = "/usr/sbin/mtu"; /* MIB Translation Utility program   */
#else
	arg_vector[0] = "/usr/bin/mtu"; /* MIB Translation Utility program   */
#endif
	arg_vector[1] = "-i";		/* Produce MOMGEN informational file */
	arg_vector[2] = next_file;	/* Possible RFC file                 */
	arg_vector[3] = 0;		/* Terminate Command line            */
	execve (*arg_vector, arg_vector, 0);
	fprintf(stderr, MP(mp063,"mirc - Error: EXECVE failure on \'%s\'\n"),
		arg_vector[0]);
	fprintf(stderr,
		MP(mp064,"       Unable to process file \'%s\'\n"),
		next_file);
	exit(BAD_EXIT);
      }
    }	/* End-if <file>.mib */

    /*
     ** Else file must be a MSL file 
     */
    else break;
  }	/* End while-do */

/* Return 'next_file' */
  return (next_file);
}
#endif

/* mirf_next_filename - Return next filename string */
/* mirf_next_filename - Return next filename string */
/* mirf_next_filename - Return next filename string */

char *
mirf_next_filename(next_cmdarg, argc, argv, bec)

int     *next_cmdarg;     /* Index into argv[] to next command line argument */
int     argc;             /* Count of arguments supplied by user on cmd-line */
char    *argv[];          /* The command line arguments themselves via ptr   */
back_end_context *bec;    /* Pointer to back-end context block               */

/*

INPUTS:

    "next_cmdarg" is the address of an integer that will be set to the number
    of the next cmd-line argument to be parsed.

    "argc" is the count of the number of arguments we were passed in argv[].

    "argv[]" is the array of pointers to the command line arguments.

    "bec" is the pointer to the back-end context needed for mirc_remove_EC()
    calls.


OUTPUTS:

    The function returns the next filename found on the command line *OR*
    in a file marked with the "-f" switch.

    This function also processes every occurence of a "-r" (remove) switch or
    "-I" (include-path) switch which may be present on the command line or in a
    filename-list file (specified by a -f switch).

    If a file is prefaced with a "-m" switch either on the command-line or
    in a file-list file, this function handles setting the global "merge"
    switch.

    The function returns NULL when the command line is exhausted.

BIRD'S EYE VIEW:
    Context:
        The caller is the main() loop in the front-end.  The name of the next
        file needs to be returned from the next point on the command line
        or within a "-f" filenames file.  

    Purpose:
        This function takes care of the details of returning the next string
        from the command line in an OS independent manner as well as handling
        the "-r", "-I" and "-m" switches.


ACTION SYNOPSIS OR PSEUDOCODE:

ULTRIX:

forever

    if (filename list file is OPEN)
        if (attempt to read the next line succeeded)
            <strip newline>
            <scan pointer to first non-space>
            if (line begins with "#" or "!")
                <continue>

            if ( line begins "-r" )
                <compute pointer to just the OID string>
                <issue "mirc - Removing Entity-Class (OID)">
                if (perform mirc_remove_EC() processing returned message)
                    <issue "mirc - Warning: <message>">
                <continue>

            if ( line begins "-I" )
                <compute pointer to just the Include Path>
                <copy the Include Path to local static buffer>
                <store the pointer to static buffer to Include Path pointer>
                <continue>

            if ( line begins "-m" )
                <compute pointer to just the file name>
                <set compiler-mode to "Merge">

            <return pointer to specified string>
        else
            <close the filename list file>
            <show no file open>

    if (next arg counter < total argument count)
        <increment next arg counter>
        if ( next argument begins "-f" )
            <compute pointer to just the filename list filename>
            if (attempt to open filename list file failed)
                <issue "Unable to open filename list file">
                <exit>
            <continue>

        if ( next argument begins "-r" )
            <compute pointer to just the OID string>
            <issue "mirc - Removing Entity-Class (OID)">
            if (perform mirc_remove_EC() processing returned message)
                <issue "mirc - Warning: <message>">
            <continue>

        if ( next argument begins "-I" )
            <compute pointer to just the Include Path>
            <store the pointer into the Include Path pointer>
            <continue>

        if ( next argument begins "-m" )
            <compute pointer to just the file name>
            <set compiler-mode to "Merge">

        <return pointer to next argument string>
    else
        <return NULL>

VMS:

    <statically init "in_expansion" flag to FALSE>
    <statically init "user flags" "context" and descriptors>
    for (ever)

        if (in_expansion is FALSE)

            if (filename list file is OPEN)
                if (attempt to read the next line succeeded)
                    <strip newline>
                    <scan pointer to first non-space>
                    if (line begins with "#" or "!")
                        <continue>

                    if ( line begins "-r" )
                        <compute pointer to just the OID string>
                        <issue "mirc - Removing Entity-Class (OID)">
                        if (perform mirc_remove_EC() processing returned message)
                            <issue "mirc - Warning: <message>">
                        <continue>

                    if ( line begins "-I" )
                        <compute pointer to just the Include Path>
                        <copy the Include Path to local static buffer>
                        <store the pointer to static buffer to Include Path pointer>
                        <continue>

                    if ( line begins "-m" )
                        <compute pointer to just the file name>
                        <set compiler-mode to "Merge">

                    <return pointer to line read>
                else
                    <close the filename list file>
                    <show no file open>

            if (next arg counter < total argument count)

                if ( next argument begins "-f" )
                    <compute pointer to just the filename list filename>
                    if (attempt to open filename list file failed)
                        <issue "Unable to open filename list file">
                        <exit>
                    <increment next arg counter>
                    <continue>

                if ( next argument begins "-r" )
                    <compute pointer to just the OID string>
                    <issue "mirc - Removing Entity-Class (OID)">
                    if (perform mirc_remove_EC() processing returned message)
                        <issue "mirc - Warning: <message>">
                    <increment next arg counter>
                    <continue>

                if ( next argument begins "-I" )
                    <compute pointer to just the Include Path>
                    <store the pointer into the Include Path pointer>
                    <increment next arg counter>
                    <continue>

                if ( next argument begins "-m" )
                    <compute pointer to just the file name>
                    <set compiler-mode to "Merge">

                <build descriptor to point at this argument>
                <context should be clear>
                <set user-flags to MULTIPLE=SET>
                <increment next arg counter>
                <execute LIB$FIND_FILE and obtain status>
                switch (status)
                    CASE RMS$_NORMAL:
                        <show in_expansion TRUE>
                        <copy returned file name to local static buffer & null term>
                        <return ptr to static buffer>
                    CASE RMS$_NMF:
                    DEFAULT:
                       <execute LIB$FIND_FILE_END>
                       <return NULL>

           else  (* We're out of cmd line arguments to consider *)
               <show in_expansion FALSE>
               <return NULL>

       else (* We are in process of expanding *)

           <execute LIB$FIND_FILE and obtain status>
           switch (status)
               CASE RMS$_NORMAL:
                   <copy returned file name to local static buffer & null term>
                   <return ptr to static buffer>
               CASE RMS$_NMF:
               DEFAULT:
                   <execute LIB$FIND_FILE_END>
                   <show in_expansion FALSE>
                   <continue>



OTHER THINGS TO KNOW:

    This function tries to make VMS work like Ultrix cmd line wildcarding.

    With V1.91, this function is enhanced to parse filenames out of the
    filename list file that can be specified with a "-f" switch.

    With V1.98, this function handles the "-r" switch plus multiple "-I"
    switches appearing either in the command line or in the -f file on
    separate lines.

    With V1.99, this function handles the "-m" switch on either the
    command line or the file-list file.
*/

{
char    *cp;   /* General purpose Character pointer */

/*
|  When this is non-NULL, the next line in this file contains the next
|  file to open.
*/
static FILE    *filename_list=NULL;

/*
| Next file name gets read into here, then the address of this static buffer
| is returned to the caller.
*/
static char    fnbuf[MAX_FILEN_SIZE];

/*
| Any Include-Path string parsed from a -f file is copied here
*/
static char    inc_path_buf[MAX_FILEN_SIZE];

/*
| Error message from mirc_remove_EC() call
*/
char    *msg;

/*================================= ULTRIX  ================================ */
#ifndef VMS
int     i;      /* General purpose index                    */


for (;/* ever */;) {

    /* if (filename list file is OPEN) */
    if (filename_list != NULL) {

        /* if (attempt to read the next line succeeded) */
        if (fgets(fnbuf, MAX_FILEN_SIZE, filename_list) != NULL) {

            /* Strip any newline */
            for (cp = fnbuf; *cp != '\0'; cp++) {
                if (*cp == '\n')
                    *cp = '\0';
                }

            /* scan pointer to first non-space */
            for (cp = fnbuf; (isspace(*cp) && *cp != '\0'); cp += 1);

            /* if (line begins with "#" or "!" or is empty) */
            if (*cp == '#' || *cp == '!' || strlen(cp) == 0)
                continue;       /* Skip it */

            /* if ( line begins "-r" ) .... process -r "remove" switch */
            if (strncmp(cp, "-r", 2) == 0) {
                /* compute pointer to just the OID string */
                cp += 2;

                /* issue "mirc - Removing Entity-Class (OID)" */
                fprintf(stderr,
                        MP(mp056,"\nmirc - Info: Removing Entity-Class '%s'\n"),
                        cp);
                
                /* if (perform mirc_remove_EC() processing returned message) */
                if ((msg = mirc_remove_EC(cp, bec)) != NULL) {
                    /* issue "mirc - Warning: <message>" */
                    fprintf(stderr,
                            MP(mp057,"mirc - Warning: %s\n       No Remove done.\n"),
                            msg);
                    }

                continue;
                }

            /* if ( line begins "-I" ) */
            if (strncmp(cp, "-I", 2) == 0) {
                /* compute pointer to just the Include Path */
                cp += 2;

                /* copy the Include Path to local static buffer */
                strcpy(inc_path_buf, cp);

                /* store pointer to static buffer to Include Path pointer */
                include_path = inc_path_buf;

                continue;
                }

            /* if ( line begins "-m" ) */
            if (strncmp(cp, "-m", 2) == 0) {
                /* compute pointer to just the file name */
                cp += 2;

                /* set the compiler mode to "Merge" */
                compiler_mode = CM_MERGE;
                }

            /* return pointer to specific string */
            return (cp);
            }

        else {
            /* close the filename list file */
            fclose(filename_list);

            /* show no file open */
            filename_list = NULL;
            }
        }

    /* if (next arg counter < total argument count) */
    if ( *next_cmdarg < argc) {

        /* increment next arg counter */
        i = *next_cmdarg;
        *next_cmdarg += 1;

        cp = argv[i];   /* Grab pointer to next argument */

        /* if ( next argument begins "-f" ) */
        if (strncmp(cp, "-f", 2) == 0) {
            /* compute pointer to just the filename list filename */
            cp += 2;

            /* if (attempt to open filename list file failed) */
            if ((filename_list = fopen(cp, "r")) == NULL) {
                /* issue "Unable to open filename list file" */
                fprintf(stderr,
                        MP(mp058,"mirc - Error: Unable to open filename list file '%s'\n"),
                        cp);
                exit(BAD_EXIT);
                }
            /* Go 'round again to get first line in the file */
            continue;
            }


        /* if ( next argument begins "-r" ) */
        if (strncmp(cp, "-r", 2) == 0) {
            /* compute pointer to just the OID string */
            cp += 2;

            /* issue "mirc - Removing Entity-Class (OID)" */
            fprintf(stderr,
                    MP(mp059,"\nmirc - Info: Removing Entity-Class '%s'\n"),
                    cp);

            /* if (perform mirc_remove_EC() processing returned message) */
            if ((msg = mirc_remove_EC(cp, bec)) != NULL) {
                /* issue "mirc - Warning: <message>" */
                fprintf(stderr,
                        MP(mp057,"mirc - Warning: %s\n       No Remove done.\n"), msg);
                }

            /* Go 'round again to get next cmd */
            continue;
            }


        /* if ( next argument begins "-I" ) */
        if ((strncmp(cp, "-I", 2) == 0) || (strncmp(cp, "-i", 2) == 0)) {
            /* compute pointer to just the Include Path */
            cp += 2;

            /* store the pointer into the Include Path pointer */
            include_path = cp;

            /* Go 'round again to get next cmd */
            continue;
            }


        /* if ( line begins "-m" ) */
        if (strncmp(cp, "-m", 2) == 0) {
            /* compute pointer to just the presumed file name */
            cp += 2;

            /* set the compiler mode to "Merge" */
            compiler_mode = CM_MERGE;
            }


        /* return pointer to next argument string */
        return (cp);
        }
    else {
        return (NULL);
        }

    } /* Forever */

#else
/*================================= VMS ==================================== */
/* statically init "in_expansion" flag to FALSE */
static BOOL in_expansion=FALSE; 

/* statically init "user flags" "context" and descriptors */
static  unsigned long   user_flags=2;   /* Bit 1 = SET = "MULTIPLE" */
static  unsigned long   context=0;      /* 0 = No context yet       */
static  unsigned long   stv=0;          /* 0 = No status yet        */

static  struct dsc$descriptor_s file_spec = {0,DSC$K_DTYPE_T, DSC$K_CLASS_S,0};
static  struct dsc$descriptor_s result_spec = {0,DSC$K_DTYPE_T, DSC$K_CLASS_D,0};
static  struct dsc$descriptor_s default_spec = {0,DSC$K_DTYPE_T, DSC$K_CLASS_S,0};
static  struct dsc$descriptor_s related_spec = {0,DSC$K_DTYPE_T, DSC$K_CLASS_S,0};

int     i;                      /* General purpose integer */

    /* for (ever) */
    for (;;) {

        /* if (in_expansion is FALSE) */
        if (in_expansion == FALSE) {

            /* if (filename list file is OPEN) */
            if (filename_list != NULL) {

                /* if (attempt to read the next line succeeded) */
                if (fgets(fnbuf, MAX_FILEN_SIZE, filename_list) != NULL) {

                    /* Strip any newline */
                    for (cp = fnbuf; *cp != '\0'; cp++) {
                        if (*cp == '\n')
                            *cp = '\0';
                        }

                    /* scan pointer to first non-space */
                    for (cp = fnbuf; (isspace(*cp) && *cp != '\0'); cp += 1);

                    /* if (line begins with "#" or "!" or is empty) */
                    if (*cp == '#' || *cp == '!' || strlen(cp) == 0)
                        continue;       /* Skip it */


                    /* if ( line begins "-r" ).. process -r "remove" switch */
                    if (strncmp(cp, "-r", 2) == 0) {
                        /* compute pointer to just the OID string */
                        cp += 2;

                        /* issue "mirc - Removing Entity-Class (OID)" */
                        fprintf(stderr,
                                MP(mp056,"\nmirc - Info: Removing Entity-Class '%s'\n"),
                                cp);

                        /* if (perform mirc_remove_EC() processing returned message) */
                        if ((msg = mirc_remove_EC(cp, bec)) != NULL) {
                            /* issue "mirc - Warning: <message>" */
                            fprintf(stderr,
                                    MP(mp057,"mirc - Warning: %s\n       No Remove done.\n"), msg);
                            }

                        continue;
                        }

                    /* if ( line begins "-I" ) */
                    if ((strncmp(cp, "-I", 2) == 0)
                        || (strncmp(cp, "-i", 2) == 0)) {
                        /* compute pointer to just the Include Path */
                        cp += 2;

                        /* copy the Include Path to local static buffer */
                        strcpy(inc_path_buf, cp);

                        /* store pointer to static buffer to Include Path pointer */
                        include_path = inc_path_buf;

                        continue;
                        }

                    /* if ( line begins "-m" ) */
                    if (strncmp(cp, "-m", 2) == 0) {
                        /* compute pointer to just the presumed filename */
                        cp += 2;

                        /* set the compiler mode to "Merge" */
                        compiler_mode = CM_MERGE;
                        }

                    /* return pointer to specified string */
                    return (cp);
                    }

                else {
                    /* close the filename list file */
                    fclose(filename_list);

                    /* show no file open */
                    filename_list = NULL;
                    }
                }

            /* if (next arg counter < total argument count) */
            if (*next_cmdarg < argc) {

                cp = argv[*next_cmdarg];   /* Grab pointer to next argument */

                /* if ( next argument begins "-f" ) */
                if (strncmp(cp, "-f", 2) == 0) {
                    /* compute pointer to just the filename list filename */
                    cp += 2;

                    /* if (attempt to open filename list file failed) */
                    if ((filename_list = fopen(cp, "r")) == NULL) {
                        /* issue "Unable to open filename list file" */
                        fprintf(stderr,
                                MP(mp058,"mirc - Error: Unable to open filename list file '%s'\n"),
                                cp);
                        exit(BAD_EXIT);
                        }

                    /* increment next arg counter */
                    *next_cmdarg += 1;

                    /* Go to the top and re-evaluate the situation */
                    continue;
                    }

                /* if ( next argument begins "-r" ) */
                if (strncmp(cp, "-r", 2) == 0) {
                    /* compute pointer to just the OID string */
                    cp += 2;

                    /* issue "mirc - Removing Entity-Class (OID)" */
                    fprintf(stderr,
                            MP(mp059,"\nmirc - Info: Removing Entity-Class '%s'\n"),
                            cp);

                    /* if (perform mirc_remove_EC() processing rtned msg) */
                    if ((msg = mirc_remove_EC(cp, bec)) != NULL) {
                        /* issue "mirc - Warning: <message>" */
                        fprintf(stderr,
                                MP(mp057,"mirc - Warning: %s\n       No Remove done.\n"), msg);
                        }

                    /* increment next arg counter */
                    *next_cmdarg += 1;

                    /* Go 'round again to get next cmd */
                    continue;
                    }

                /* if ( next argument begins "-I" or "-i" ) */
                if ((strncmp(cp, "-I", 2) == 0)
                    || (strncmp(cp, "-i", 2) == 0)) {
                    /* compute pointer to just the Include Path */
                    cp += 2;

                    /* store the pointer into the Include Path pointer */
                    include_path = cp;

                    /* increment next arg counter */
                    *next_cmdarg += 1;

                    /* Go 'round again to get next cmd */
                    continue;
                    }

                /* if ( next argument begins "-m" ) */
                if (strncmp(cp, "-m", 2) == 0 ) {
                    /* compute pointer to just the presumed filename */
                    cp += 2;

                    /* set compiler mode to "Merge" */
                    compiler_mode = CM_MERGE;
                    }

                /* build descriptor to point at this argument */
                file_spec.dsc$w_length = strlen(cp);
                file_spec.dsc$a_pointer = cp;

                /* context should be clear */
                context = 0;

                /* set user-flags to MULTIPLE=SET (Done at init time) */

                /* increment next arg counter */
                *next_cmdarg += 1;

                /* execute LIB$FIND_FILE and obtain status */
                stv = lib$find_file(&file_spec,
                                    &result_spec,
                                    &context,
                                    &default_spec,
                                    &related_spec,
                                    &stv,
                                    &user_flags);

                switch (stv) {
                    case RMS$_NORMAL:
                        in_expansion = TRUE;    /* show in_expansion TRUE */

                        /*
                        | copy returned file name to local static buffer &
                        | null terminate it.
                        */
                        i = result_spec.dsc$w_length;  /* Convert to integer */
                        strncpy(fnbuf, result_spec.dsc$a_pointer, i);
                        fnbuf[i] = '\0';               /* Null terminate     */

                        /* return ptr to static buffer */
                        return (fnbuf);

                    case RMS$_FNF:
                        /* issue "Unable to open filename list file" */
                        fprintf(stderr,
                                MP(mp001,
                     "mirc - Error: Failed to open MSL file\n       \"%s\"\n"),
                                cp);
                        exit(BAD_EXIT);

                    case RMS$_NMF:
                    default:
                       /* execute LIB$FIND_FILE_END */
                       lib$find_file_end(&context);

                       return (NULL);
                    }
                }

            else { /* We're out of cmd line arguments to consider */
                in_expansion = FALSE;   /* show in_expansion FALSE */
                return (NULL);
                }
            }

        else { /* We are in process of expanding */

            /* execute LIB$FIND_FILE and obtain status */
            stv = lib$find_file(    &file_spec,
                                    &result_spec,
                                    &context,
                                    &default_spec,
                                    &related_spec,
                                    &stv,
                                    &user_flags);
            switch (stv) {
                case RMS$_NORMAL:
                    /* copy returned file name to local static buffer & null term */
                    i = result_spec.dsc$w_length;   /* Convert to integer */
                    strncpy(fnbuf, result_spec.dsc$a_pointer, i);
                    fnbuf[i] = '\0';                /* Null terminate     */

                    /* return ptr to static buffer */
                    return (fnbuf);

                case RMS$_NMF:
                default:
                   /* execute LIB$FIND_FILE_END */
                   lib$find_file_end(&context);

                   /* show in_expansion FALSE */
                   in_expansion = FALSE;
                   continue;
                }
            }
    }
#endif
}

/* mirf_elapsed - Compute Elapsed-Time string for compiler statistics */
/* mirf_elapsed - Compute Elapsed-Time string for compiler statistics */
/* mirf_elapsed - Compute Elapsed-Time string for compiler statistics */

void
mirf_elapsed(etime, msg)

int     etime;          /* Elapsed time in seconds */
char    *msg;           /* Message buffer          */

/*
INPUTS:

    "etime" is the elapsed time in binary seconds that must be formatted into
    a nice string into "msg".

OUTPUTS:

    The function returns into the passed buffer the phrase "xx min. yy secs.",
    where "xx min." is skipped in "xx" turns out to be 0.

BIRD'S EYE VIEW:
    Context:
        The caller is the any high level function in the compiler that
        needs to report elapsed time to the user.

    Purpose:
        This function takes care of the details of computing a string
        that reflects the proper elapsed time.


ACTION SYNOPSIS OR PSEUDOCODE:

    <compute number if minutes>
    <subtract the proper number of minute-seconds from elapsed time>
    if (number of minutes <= 0)
        <format "yy secs.">
    else
        <format "xx mins. yy secs.">

OTHER THINGS TO KNOW:
                                                               2
    If you travel in a space-ship that accelerates at 32-ft/sec  (the rate
    at which things accelerate while falling near the surface in the Earth's
    gravitational field) half-way to Pluto, and then decelerates at the
    same rate the rest of the way to Pluto, it'll take a month to get there.

*/

{
int             minutes;        /* Computed number of minutes */


/* compute number if minutes */
minutes = etime/60;

/* subtract the proper number of minute-seconds from elapsed time */
etime -= minutes*60;

/* if (number of minutes <= 0) */
if (minutes <= 0) {
    /* format "yy secs. */
    sprintf(msg, "%d sec.", etime);
    }
else {
    /* format "xx mins. yy secs." */
    sprintf(msg, "%d min. %d sec.", minutes, etime);
    }

}
