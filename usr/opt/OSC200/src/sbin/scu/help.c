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
static char *rcsid = "@(#)$RCSfile: help.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/05/05 11:42:37 $";
#endif
/*
 * %W%  %G%  (UNIX "sccs" id data)
 *
 * COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1991.
 * ALL RIGHTS RESERVED.
 * 
 * THIS  SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE
 * USED  AND  COPIED ONLY IN ACCORDANCE WITH THE TERMS OF
 * SUCH  LICENSE  AND  WITH  THE  INCLUSION  OF THE ABOVE
 * COPYRIGHT  NOTICE.   THIS SOFTWARE OR ANY OTHER COPIES
 * THEREOF   MAY   NOT  BE  PROVIDED  OR  OTHERWISE  MADE
 * AVAILABLE  TO  ANY  OTHER  PERSON.   NO  TITLE  TO AND
 * OWNERSHIP OF THE SOFTWARE IS HEREBY TRANSFERRED.
 * 
 * THE  INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE
 * WITHOUT  NOTICE  AND  SHOULD  NOT  BE  CONSTRUED  AS A
 * COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * 
 * DIGITAL  ASSUMES  NO  RESPONSIBILITY  FOR  THE  USE OR
 * RELIABILITY  OF  ITS SOFTWARE ON EQUIPMENT THAT IS NOT
 * SUPPLIED BY DIGITAL.
 */

/*
 * MODULE DESCRIPTION:
 *
 *      Provide help text to user, portable between VMS, UNIX, and DOS.
 *      On UNIX and DOS, this uses a VMS-style help library source 
 *      file (not the library file itself),
 *
 *
 * HISTORY:
 *
 *      29-Mar-1991     Initial version.
 *
 *      09-Apr-1991     Add new call parameter to enable/disable paging.
 *                      Add ability to use "more" on UNIX (via "popen").
 *                      Fix bug in getchar usage on UNIX (set/reset cbreak).
 */

/*
 * Multi-routine data definitions
 */
/*
 * Common include files
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef VMS
#include <ssdef.h>
#include <rmsdef.h>
#include <dvidef.h>
#include <descrip.h>
#include <libdef.h>
#endif

#ifdef unix
#include <termios.h>
#endif

#ifdef OSF
#include <sys/ioctl.h>
#endif

#if defined(VMS) || defined(mips) || defined(__alpha)
#define PROTO(args) args
#else
#define PROTO(args) ()
#endif



/*
 * Function prototype and macro to get system error message
 */
char *get_message PROTO((int));

#ifdef VMS
#define SYS_ERRMSG  get_message(vaxc$errno)
#else
#define SYS_ERRMSG  get_message(errno)
#endif

#define COD_ERRMSG(code) get_message(code)

/*
 * Macros to output text to the user
 *
 * On VMS, we have a choice of user callback or direct output, and
 * any paging decisions lie only with specified user parameter.
 *
 * On UNIX, we have a choice of user callback, piping through "more"
 * via "popen" (only if we have complete control over user input/output),
 * or direct output.  Paging decisions lie with specified user parameter,
 * but not if piping to "more" (more does its own paging).
 */
#ifdef VMS  /* --- VMS system ------------------------------------------ */

#define TTY_PUT(text)  \
  if (sav_ttyput != NULL) { (*sav_ttyput)(text); }  \
  else                    { fputs(text,stdout);  }

#define TTY_LINCNT (sav_paged)

#else
#ifdef unix /* --- UNIX system ----------------------------------------- */

#define TTY_OPEN  \
  popen_stream = NULL;  \
  if ((sav_ttyput == NULL) && (sav_ttyget == NULL) && (sav_paged))  \
  { popen_stream = popen("more -d", "w"); }

#define TTY_PUT(text)  \
  if (sav_ttyput != NULL)        { (*sav_ttyput)(text); }  \
  else if (popen_stream != NULL) { fputs(text, popen_stream); fflush(popen_stream); }  \
  else                           { fputs(text,stdout); }

#define TTY_CLOSE  \
  if (popen_stream != NULL) { pclose(popen_stream); popen_stream = NULL; }

#define TTY_LINCNT ((sav_paged) && (popen_stream == NULL))

#else       /* --- DOS system ------------------------------------------ */

#define TTY_OPEN

#define TTY_PUT(text)  \
  if (sav_ttyput != NULL) { (*sav_ttyput)(text); }  \
  else                    { fputs(text,stdout);  }

#define TTY_CLOSE

#define TTY_LINCNT (sav_paged)

#endif
#endif      /* --------------------------------------------------------- */

/*
 * Return codes
 */
#define RETURN_SUCCESS  1   /* Completed normally */
#define RETURN_NONE     0   /* Key not found (only used internally) */
#define RETURN_FILE    -1   /* Open error on help file */

/*
 * Common definitions
 */
#define TRUE  1
#define FALSE 0

#define TOPIC_SIZE 512

/*
 * Common data
 */
#define HELP_BUFFER_SIZE 512                    /* Max size for help text */

static char cmd_topic[TOPIC_SIZE+1];            /* Current topic for search */

static int   number_lines;              /* Maximum lines for "MORE" */
static int   count_lines;               /* Line counter for "MORE" */
static int   number_columns;            /* Number of columns for topic wrap */

static int   sav_paged;                 /* Saved paging flag */

static int (*sav_ttyget)();             /* User input callback address */
static int (*sav_ttyput)();             /* User output callback address */
static int (*sav_ttyerr)();             /* Error callback address */

#ifndef VMS /* --------------------------------------------------------- */

#define LIST_TOPICS '?'             /* Char used to get topic list */

#define HELP_TAB     20             /* Tab stop for subtopic list */
#define HELP_STOP    60             /* Line stop for subtopic list */
#define LINE_STOP    75             /* Line stop for subtopic list */

static char current_topic[HELP_BUFFER_SIZE+1];      /* Current topic for display */
static char last_current_topic[HELP_BUFFER_SIZE+1]; /* Last topic for display */
static char last_cmd_topic[TOPIC_SIZE+1];           /* Last topic for search */
static FILE *help_file;                             /* Help file pointer */

static char  line_file[HELP_BUFFER_SIZE+4];     /* Line from help file */
static char  key_file[HELP_BUFFER_SIZE+1];      /* Key from help file */
static char  oops_string[HELP_BUFFER_SIZE * 4]; /* Subtopic list accumulator */
static char init_key[] = "!";                   /* Fake key for first time */
static int   oops_cc;
static int   save_level;

#ifdef unix
static FILE           *popen_stream; /* Use to pipe output to "more" */
static int            tty_fileno;    /* File number for user input */
static struct termios tty_saved;     /* For saving tty settings */
static struct termios tty_cbreak;    /* For setting tty to cbreak mode */
#endif

#endif      /* --------------------------------------------------------- */

/*
 * Defaults for user defined output strings
 *
 * In the "str_err_lbr" string, the first "%s" represents the library file
 * name, and the second represents the system error text.
 */
#ifdef VMS
static char str_err_lbr[] = "Error - Cannot access help library\n        %s\n        %s";
static char str_inf_mor[] = "Press RETURN to continue ... ";
#else
static char str_err_lbr[] = "Error - Cannot access help library\n        %s\n        %s";
static char str_inf_mor[] = "--Press space for more--";
static char str_inf_ins[] = "\nType a topic or subtopic keyword to get help text\nType \"?\" for a list of subtopics\nType RETURN to back up to the previous topic level\n\n";
static char str_inf_mta[] = "  Topics available:\n\n";
static char str_inf_sta[] = "  Subtopics available:\n\n";
static char str_inf_mtl[] = "\n  No topics are available\n\n";
#ifdef notdef
static char str_inf_stl[] = "  No subtopics are available, type RETURN to go to the previous topic\n\n";
#else
static char str_inf_stl[] = "";	/* To force VMS like help behavior... */
#endif
static char str_inf_mtp[] = "Topic: ";
static char str_inf_stp[] = "subtopic: ";
static char str_inf_non[] = "\nNo help is available for:\n  ";
#endif

/*
 * Pointers to user defined output strings
 */
#ifdef VMS
static char *pnt_err_lbr = &str_err_lbr[0];
static char *pnt_inf_mor = &str_inf_mor[0];
#else
static char *pnt_err_lbr = &str_err_lbr[0];
static char *pnt_inf_mor = &str_inf_mor[0];
static char *pnt_inf_ins = &str_inf_ins[0];
static char *pnt_inf_mta = &str_inf_mta[0];
static char *pnt_inf_sta = &str_inf_sta[0];
static char *pnt_inf_mtl = &str_inf_mtl[0];
static char *pnt_inf_stl = &str_inf_stl[0];
static char *pnt_inf_mtp = &str_inf_mtp[0];
static char *pnt_inf_stp = &str_inf_stp[0];
static char *pnt_inf_non = &str_inf_non[0];
#endif

/*
 * portable_help_strings --- Global routine
 *
 * Set the user-visible text strings used by the portable_help function.
 * This allows the calling program to change the text used, or to use
 * text in a language other than English.  This function should be called
 * once before calling the portable_help function.
 *
 * If the strings are not set by this call, they will default to a set of
 * standard English strings.
 *
 * This call creates local static pointers to the specified strings.  
 * This means that the caller's string buffers must remain allocated and
 * unmodified (unless this function is called prior to every call to
 * the portable_help function).
 *
 * See the static "str_xxx_yyy" variables for suggested strings to use.
 * Note that any printf formatting variables (e.g. "%s") must be kept
 * in the same relative order as in these suggested strings.
 *
 * Inputs:
 *      (char *) usr_err_lbr - Help file access error
 *      (char *) usr_inf_mor - More help text to come
 *      (char *) usr_inf_ins - Instructions           (not used on VMS)
 *      (char *) usr_inf_mta - Topics header          (not used on VMS)
 *      (char *) usr_inf_sta - Subtopics header       (not used on VMS)
 *      (char *) usr_inf_mtl - No topics              (not used on VMS)
 *      (char *) usr_inf_stl - No subtopics           (not used on VMS)
 *      (char *) usr_inf_mtp - Topic prompt           (not used on VMS)
 *      (char *) usr_inf_stp - Subtopic prompt        (not used on VMS)
 *      (char *) usr_inf_non - No help                (not used on VMS)
 *
 *      If running on VMS, and the string is "not used on VMS", you must 
 *      still provide the function argument, but it will not be used or 
 *      referenced (the argument must be supplied solely for portability).
 *
 * Outputs:
 *      None.
 *
 * Notes:
 *      If any pointer value is supplied as NULL, the default string will
 *      be used.
 *
 *      Normally, on a VMS system, the help prompt "sticks" at the last
 *      topic level that had subtopics.  When the user types a subtopic,
 *      the system checks to see if that subtopic has its own subtopics.
 *      If so, the next help prompt is for one of the sub-subtopics.  If
 *      not, the next help prompt is for one of the subtopics at the same
 *      level as last time.
 *
 *      The default behaviour on a UNIX or DOS system is to always ask
 *      for a subtopic at the next level down, whether or not one actually
 *      exists.  This allows the user to enter "?" to see the same help text
 *      again.
 *
 *      To force the UNIX/DOS behaviour to emulate that of VMS, supply
 *      a pointer to a zero-length string (i.e. "") for the "usr_inf_stl" 
 *      parameter, rather than a specific string or a NULL pointer.
 */

int portable_help_strings(usr_err_lbr, usr_inf_mor, usr_inf_ins, usr_inf_mta, usr_inf_sta, usr_inf_mtl, usr_inf_stl, usr_inf_mtp, usr_inf_stp, usr_inf_non)

char *usr_err_lbr;
char *usr_inf_mor;
char *usr_inf_ins;
char *usr_inf_mta;
char *usr_inf_sta;
char *usr_inf_mtl;
char *usr_inf_stl;
char *usr_inf_mtp;
char *usr_inf_stp;
char *usr_inf_non;

{

    if ((pnt_err_lbr != NULL) && (*pnt_err_lbr != '\0')) {pnt_err_lbr = usr_err_lbr;}
    if ((pnt_inf_mor != NULL) && (*pnt_inf_mor != '\0')) {pnt_inf_mor = usr_inf_mor;}
                          
#ifndef VMS
                          
    if (pnt_inf_ins != NULL) {pnt_inf_ins = usr_inf_ins;}
    if (pnt_inf_mta != NULL) {pnt_inf_mta = usr_inf_mta;}
    if (pnt_inf_sta != NULL) {pnt_inf_sta = usr_inf_sta;}
    if (pnt_inf_mtl != NULL) {pnt_inf_mtl = usr_inf_mtl;}
    if (pnt_inf_stl != NULL) {pnt_inf_stl = usr_inf_stl;}
    if (pnt_inf_mtp != NULL) {pnt_inf_mtp = usr_inf_mtp;}
    if (pnt_inf_stp != NULL) {pnt_inf_stp = usr_inf_stp;}
    if (pnt_inf_non != NULL) {pnt_inf_non = usr_inf_non;}

#endif

}

/*
 * portable_help --- Global routine
 *
 * Provide command line help.
 *
 * Inputs:
 *      (char *)  inp_libr   - File name for help file
 *      (char *)  inp_usrtop - User-specified topic ("" or NULL if none)
 *      (char *)  inp_initop - Topic if none from user ("" or NULL if none)
 *      (int)     inp_paged  - TRUE (1) if use paged output, FALSE (0) if no
 *      (int *()) inp_ttyget - Routine to call to get user input
 *      (int *()) inp_ttyput - Routine to call to put user output
 *      (int *()) inp_ttyerr - Routine to call to report errors
 *
 * Outputs:
 *      (int) RETURN_SUCCESS (+1) = Completed normally
 *            RETURN_FILE    (-1) = Open error on help file
 *
 * Notes concerning "inp_usrtop" and "inp_initop":
 *
 *      If the user specifies a topic, indicate that by "inp_usrtop".  If
 *      the user does not specify a topic, supply NULL or a pointer to
 *      a zero-length string.
 *
 *      If there is no user-specified topic, a default topic can be defined
 *      using "inp_initop".  If this is NULL or points at a zero-length
 *      string, the first display will simply be a topics list.  If this
 *      points at a positive-length string, an initial default topic will
 *      be displayed before the main topics list.  On VMS, the initial topic
 *      is always "HELP", regardless of what "inp_initop" points to.  On
 *      UNIX and DOS, the initial topic is defined by the "inp_initop" string
 *      value (this should normally be "HELP", for VMS compatibility).  
 *
 * Notes concerning "inp_ttyget":
 *
 *      This argument points to a routine that will obtain input from the user
 *      in a manner that is consistent with the rest of the application.
 *
 *      This routine must be declared and structured as follows:
 *
 *          int routine_name(prompt_pointer, buffer_pointer, buffer_length)
 *
 *          char *prompt_pointer;   (Address of prompt to issue to user)
 *          char *buffer_pointer;   (Address of buffer to receive input)
 *          int buffer_length;      (Max chars that will fit in the buffer)
 *
 *          {
 *              <code to prompt the user and return the input>
 *              <input must terminate with '\0' unless it fills the buffer>
 *              <input may be zero-length>
 *              if (<user typed EOF character>) {return (-1);}
 *              return (1);
 *          }
 *
 *      If you wish to use the normal mechanisms (LIB$GET_INPUT on VMS and
 *      puts/gets on UNIX and DOS), pass a NULL as the value for this
 *      argument.
 *
 * Notes concerning "inp_ttyput":
 *
 *      This argument points to a routine that will print output to the user
 *      in a manner that is consistent with the rest of the application.
 *
 *      This routine must be declared and structured as follows:
 *
 *          int routine_name(buffer_pointer)
 *
 *          char *buffer_pointer;   (Address of buffer containing output)
 *
 *          {
 *              <code to print the output>
 *              <output bufffer is null-terminated>
 *              <output bufffer contains all required carriage control>
 *              return (1);
 *          }
 *
 *      If you wish to use the normal mechanisms (puts to stdout on VMS,
 *      UNIX and DOS), pass a NULL as the value for this argument.
 *
 * Notes concerning "inp_ttyerr":
 *
 *      This argument points to a routine that will report help file access
 *      errors to the user in a manner that is consistent with the rest of
 *      the application.
 *
 *      This routine must be declared and structured as follows:
 *
 *          int routine_name(errno_code)
 *
 *          int errno_code;         (code describing exact access error)
 *
 *          {
 *              <for UNIX, use strerror routine or equivalent>
 *              <for VMS,  use SYS$GETMSG or equivalent>
 *              return (1);
 *          }
 *
 *      If you wish the get_help routines to print the error message using
 *      fprintf(stderr), pass a NULL as the value for this argument.
 *
 * Notes concerning returned value:
 *
 *      On a RETURN_FILE error, an error message has already been printed
 *      on "stderr".
 */

int portable_help(inp_libr, inp_usrtop, inp_initop, inp_paged, inp_ttyget, inp_ttyput, inp_ttyerr)

char  *inp_libr;
char  *inp_usrtop;
char  *inp_initop;
int    inp_paged;
int  (*inp_ttyget)();
int  (*inp_ttyput)();
int  (*inp_ttyerr)();

{
    /*
     * Local data
     */
#ifdef VMS
    extern unsigned long int LBR$OUTPUT_HELP();
    int vms_input();
    int vms_output();
    struct dsc$descriptor_s help_command;
    struct dsc$descriptor_s help_library;
    unsigned long int help_flags;
    register unsigned long int i_result;
#else
    int unix_help();
    register int i_result;
    register int i_offset;
    int print_flag;
    char *pnt1;
#endif
    char null_string = '\0';

    /*
     * Number of lines/columns for paging control
     */
    if (inp_paged)
    {
        number_lines = terminal_size('l');
        if (number_lines <= 0) {number_lines = 24;}
        count_lines = number_lines;
    }

    number_columns = terminal_size('c');
    if (number_columns <= 0) {number_columns = 80;}

    /*
     * Copy input information to local storage (only for things that
     * might be modified)
     */
    if (inp_usrtop == NULL) {inp_usrtop = &null_string;}
    if (inp_initop == NULL) {inp_initop = &null_string;}

    strncpy(&cmd_topic[0], inp_usrtop, TOPIC_SIZE);
    cmd_topic[TOPIC_SIZE] = '\0';
    input_compress(&cmd_topic[0]);

    /*
     * Move the input parameters to common storage
     */
    sav_paged  = inp_paged;
    sav_ttyget = inp_ttyget;
    sav_ttyput = inp_ttyput;
    sav_ttyerr = inp_ttyerr;

#ifdef VMS

    /*
     * Build the descriptors.  
     */
    help_command.dsc$a_pointer = &cmd_topic[0];
    help_command.dsc$w_length = strlen(&cmd_topic[0]);
    help_command.dsc$b_dtype = DSC$K_DTYPE_T;
    help_command.dsc$b_class = DSC$K_CLASS_S;

    help_library.dsc$a_pointer = inp_libr;
    help_library.dsc$w_length = strlen(inp_libr);
    help_library.dsc$b_dtype = DSC$K_DTYPE_T;
    help_library.dsc$b_class = DSC$K_CLASS_S;

    help_flags = 0x0001 | /* Provide interactive help prompting */
                 0x0002 | /* Search process log name table for def help libr */
                 0x0004 | /* Search group log name table for def help libr */
                 0x0008;  /* Search system log name table for def help libr */

    if (*inp_initop != '\0')
    {
        help_flags |= 0x0020;   /* Use HELP as the default keyword */
    }

    /*
     * Provide help and prompt for additional topics.
     */
    TTY_PUT("\n");
    i_result = LBR$OUTPUT_HELP(&vms_output, &number_columns, &help_command, &help_library, &help_flags, &vms_input);
    if (i_result != SS$_NORMAL)
    {
        if (sav_ttyerr == NULL)
        {
            fprintf(stderr, pnt_err_lbr, inp_libr, COD_ERRMSG(i_result));
        }
        else
        {
            (*sav_ttyerr)(i_result);
        }
        return (RETURN_FILE);
    }

    TTY_PUT("\n");
    return (RETURN_SUCCESS);

#else /* if UNIX or DOS */

    /*
     * Initialize some data
     */
    current_topic[0] = '\0';
    last_current_topic[0] = '\0';
    last_cmd_topic[0] = '\0';

    /*
     * Save the current TTY settings (UNIX only)
     */
#ifdef unix
    tty_fileno = fileno(stdin);
    tcgetattr (tty_fileno, &tty_saved);
    bcopy (&tty_saved, &tty_cbreak, sizeof(struct termios));
    tty_cbreak.c_lflag     &= (~(ICANON | ECHO));
    tty_cbreak.c_cc[VMIN]   = 1;
    tty_cbreak.c_cc[VTIME]  = 0;
#endif

    /*
     * If output is being paged, and we can use "more" to do it, set up to
     * pipe output through "more" (UNIX only)
     */
    TTY_OPEN;

    /*
     * Give the initial header message
     */
    TTY_PUT(pnt_inf_ins);

    pnt1 = pnt_inf_ins;
    if (TTY_LINCNT)
    {
        while (*pnt1 != '\0') {if (*pnt1++ == '\n') {count_lines -= 1;}}
    }

    /*
     * Keep looping until user indicates no more
     */
    print_flag = TRUE;
    while (TRUE)
    {
        /*
         * Provide help on the topic
         */
        if (print_flag)
        {
            /*
             * Search for the key and print the text
             */
            if ((cmd_topic[0] != '\0') || (*inp_initop == '\0'))
            {
                i_result = unix_help(inp_libr, &cmd_topic[0], TRUE, TRUE);
            }
            else
            {
                i_result = unix_help(inp_libr, inp_initop, TRUE, FALSE);
                if (i_result != RETURN_FILE) {i_result = unix_help(inp_libr, "", FALSE, TRUE);}
            }

            /*
             * If found the topic, save it as a valid topic, otherwise
             * back up to the last valid topic.
             *
             * If got file error, just quit.
             */
            if (i_result == RETURN_SUCCESS)
            {
                strcpy(&last_cmd_topic[0], &cmd_topic[0]);
                strcpy(&last_current_topic[0], &current_topic[0]);
            }
            else if (i_result == RETURN_NONE)
            {
                strcpy(&cmd_topic[0], &last_cmd_topic[0]);
                strcpy(&current_topic[0], &last_current_topic[0]);
                unix_help(inp_libr, &cmd_topic[0], FALSE, FALSE);

#if 0
%%%
                i_result = strlen(&cmd_topic[0]);
                while ((i_result > 0) && (cmd_topic[i_result] == ' ')) {i_result--;}
                while ((i_result > 0) && (cmd_topic[i_result] != ' ')) {i_result--;}
                cmd_topic[i_result] = '\0';
                input_compress(&cmd_topic[0]);
#endif
            }
            else if (i_result == RETURN_FILE)
            {
                TTY_CLOSE;
                return (RETURN_FILE);
            }
        }

        /*
         * Get the next sub-topic.  The "i_offset" variable must be left
         * pointing at the new sub-topic string (or '\0' if none).
         */
        if (TTY_LINCNT)
        {
            count_lines -= 1;
            if (count_lines <= 2)
            {
                TTY_PUT(pnt_inf_mor);
#ifdef unix
                tcsetattr(tty_fileno, TCSANOW, &tty_cbreak);
#endif
                getchar();
#ifdef unix
                tcsetattr(tty_fileno, TCSANOW, &tty_saved);
#endif
                TTY_PUT("\r\233K");
            }
        }

        TTY_CLOSE;
        i_offset = strlen(&cmd_topic[0]);
        if (i_offset == 0)
        {
            if (inp_ttyget != NULL)
            {
                if ((*inp_ttyget)(pnt_inf_mtp, &cmd_topic[0], TOPIC_SIZE) == -1) {break;}
            }
            else
            {
                TTY_PUT(pnt_inf_mtp);
                if (fgets(&cmd_topic[0], TOPIC_SIZE, stdin) == NULL) {break;}
                cmd_topic[TOPIC_SIZE] = '\0';
                i_result = strlen(&cmd_topic[0]);
                if (i_result-- > 0)
                {
                    if (cmd_topic[i_result] == '\n') {cmd_topic[i_result] = '\0';}
                }
            }
        }
        else
        {
            char prompt_buffer[HELP_BUFFER_SIZE+4];
            int  prompt_length;
            int  topic_length;
            int  topic_offset;

            if (cmd_topic[i_offset-1] != ' ') {cmd_topic[i_offset++] = ' ';}
            cmd_topic[i_offset] = '\0';

            prompt_buffer[0] = '\0';
            prompt_length = strlen(pnt_inf_stp);
            topic_length  = strlen(&current_topic[0]);
            topic_offset = 0;

            if (topic_length > (number_columns - prompt_length - 10))
            {
                topic_offset = topic_length - (number_columns - prompt_length - 10) + 3;
                strcpy(&prompt_buffer[0], "...");
            }

            strncat(&prompt_buffer[0], &current_topic[topic_offset], (HELP_BUFFER_SIZE - prompt_length));
            prompt_buffer[HELP_BUFFER_SIZE - prompt_length] = '\0';
            if (prompt_buffer[0] == '\0') {strcpy(&prompt_buffer[0], "??? ");}
            strcat(&prompt_buffer[0], pnt_inf_stp);

            if (inp_ttyget != NULL)
            {
                if ((*inp_ttyget)(&prompt_buffer[0], &cmd_topic[i_offset], (TOPIC_SIZE - i_offset)) == -1) {break;}
            }
            else
            {
                TTY_PUT(&prompt_buffer[0]);
                if (fgets(&cmd_topic[i_offset], (TOPIC_SIZE - i_offset), stdin) == NULL) {break;}
                cmd_topic[TOPIC_SIZE] = '\0';
                i_result = strlen(&cmd_topic[0]);
                if (i_result-- > 0)
                {
                    if (cmd_topic[i_result] == '\n') {cmd_topic[i_result] = '\0';}
                }
            }
        }

        input_compress(&cmd_topic[i_offset]);
        if (TTY_LINCNT) {count_lines = number_lines;}
        TTY_OPEN;

        /*
         * Check for move up one topic (uses i_offset from above)
         */
        if (cmd_topic[i_offset] == '\0')
        {
            /*
             * Strip off right-most topic.
             * If nothing to strip off, then finished with help.
             * If sonething, get next subtopic without printing the help text.
             */
            if (cmd_topic[0] == '\0') {break;}

            i_result = strlen(&cmd_topic[0]) - 1;
            while ((i_result > 0) && (cmd_topic[i_result] == ' ')) {i_result--;}
            while ((i_result > 0) && (cmd_topic[i_result] != ' ')) {i_result--;}
            cmd_topic[i_result] = '\0';

            i_result = strlen(&current_topic[0]) - 1;
            while ((i_result > 0) && (current_topic[i_result] == ' ')) {i_result--;}
            while ((i_result > 0) && (current_topic[i_result] != ' ')) {i_result--;}
            current_topic[i_result++] = ' ';
            current_topic[i_result] = '\0';

            strcpy(&last_cmd_topic[0], &cmd_topic[0]);
            strcpy(&last_current_topic[0], &current_topic[0]);
            print_flag = FALSE;
        }

        /*
         * If re-printing the current topic, strip off the special character
         * and set to print the topic.
         */
        else if (cmd_topic[i_offset] == LIST_TOPICS)
        {
            cmd_topic[i_offset] = '\0';
            input_compress(&cmd_topic[0]);
            print_flag = TRUE;
        }

        /*
         * If neither of the above, must have a new subtopic.
         * Set to print it.
         */
        else 
        {
            input_compress(&cmd_topic[0]);
            print_flag = TRUE;
        }
    }

    TTY_PUT("\n");
    TTY_CLOSE;
    return (RETURN_SUCCESS);

#endif

}

/*
 * vms_input --- Local routine
 *
 * Special input routine for VMS help, to provide paging.
 *
 * Inputs (identical to LIB$GET_INPUT):
 *      (struct dsc$descriptor_s *) input_string  - String input from SYS$INPUT
 *      (struct dsc$descriptor_s *) prompt_string - String to use as prompt
 *      (unsigned short *)          result_length - Place to put string length
 *
 * Outputs:
 *      (int) - Condition code (SS$_NORMMAL)
 */

#ifdef VMS /* Only necessary on VMS systems */

static int vms_input(input_string, prompt_string, result_length)

struct dsc$descriptor_s *input_string;
struct dsc$descriptor_s *prompt_string;
unsigned short          *result_length;

{
    /*
     * Local data
     */
    extern unsigned long int LIB$GET_INPUT();
    register char *pnt1;
    register char *pnt2;
    register int  msg_length;
    register int  inpchar;
    char msg_buffer[256 + 1];

    /*
     * Make a local copy of the text so we can null-terminate it.
     * We can also count any embedded line feeds.
     * Don't allow a zero-length output string (can mess up carriage control).
     */
    msg_length = prompt_string->dsc$w_length;
    if (msg_length > 256) {msg_length = 256;}
    pnt2 = prompt_string->dsc$a_pointer;
    pnt1 = &msg_buffer[0];

    if (msg_length == 0) {*pnt1++ = ' ';}

    if (TTY_LINCNT) count_lines -= 1;
    while (msg_length-- > 0)
    {
        if (TTY_LINCNT) {if (*pnt2 == '\n') {count_lines -= 1;}}
        *pnt1++ = *pnt2++;
    }
    *pnt1 = '\0';

    /*
     * Don't let line count go to zero because we want to retain context.
     */
    if (TTY_LINCNT) 
    {
        if (count_lines <= 2)
        {
            /*
             * Filled up the screen, have user type RETURN
             */
            if (sav_ttyget != NULL)
            {
                char more_buffer[81];
                if ((*sav_ttyget)(str_inf_mor, &more_buffer[0], 80) == -1)
                {
                    TTY_PUT(" \n");
                    count_lines = number_lines;
                    return (RMS$_EOF);
                }
            }
            else
            {
                struct dsc$descriptor_s more_dsc;
                struct dsc$descriptor_s prompt_dsc;
                char more_buffer[81];
    
                more_dsc.dsc$a_pointer = &more_buffer[0];
                more_dsc.dsc$w_length = 80;
                more_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
                more_dsc.dsc$b_class = DSC$K_CLASS_S;

                prompt_dsc.dsc$a_pointer = str_inf_mor;
                prompt_dsc.dsc$w_length = strlen(str_inf_mor);
                prompt_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
                prompt_dsc.dsc$b_class = DSC$K_CLASS_S;

                if (LIB$GET_INPUT(&more_dsc, &prompt_dsc, NULL) != SS$_NORMAL)
                {
                    TTY_PUT(" \n");
                    count_lines = number_lines;
                    return (RMS$_EOF);
                }
            }
    
            TTY_PUT("\r\233A\233K");
        }
    }

    /*
     * Get the user input
     */
    if (sav_ttyget != NULL)
    {
        inpchar = (*sav_ttyget)(&msg_buffer[0], input_string->dsc$a_pointer, input_string->dsc$w_length);
        *result_length = strlen(input_string->dsc$a_pointer);
    }
    else
    {
        if (LIB$GET_INPUT(input_string, prompt_string, result_length) == SS$_NORMAL) {inpchar = 1;}
        else {inpchar = -1;}
    }

    /*
     * Fix things up and return
     */
    if (TTY_LINCNT) {count_lines = number_lines;}

    if (inpchar == -1)
    {
        TTY_PUT(" \n");
        return (RMS$_EOF);
    }

    return (SS$_NORMAL);
}

#endif

/*
 * vms_output --- Local routine
 *
 * Special output routing for VMS help, to provide paging.
 *
 * Inputs (identical to LIB$PUT_OUTPUT):
 *      (struct dsc$descriptor_s *) message_string - String to output
 *
 * Outputs:
 *      (int) - Condition code (SS$_NORMMAL)
 */

#ifdef VMS /* Only necessary on VMS systems */

static int vms_output(message_string)

struct dsc$descriptor_s *message_string;

{
    /*
     * Local data
     */
    extern unsigned long int LIB$GET_INPUT();
    register char *pnt1;
    register char *pnt2;
    register int  msg_length;
    register int  inpchar;
    char msg_buffer[256 + 2];

    /*
     * Make a local copy of the text so we can null-terminate it.
     * We can also count any embedded line feeds.
     * Don't allow a zero-length output string (can mess up carriage control).
     */
    msg_length = message_string->dsc$w_length;
    if (msg_length > 256) {msg_length = 256;}
    pnt2 = message_string->dsc$a_pointer;
    pnt1 = &msg_buffer[0];

    if (msg_length == 0) {*pnt1++ = ' ';}

    if (TTY_LINCNT) {count_lines -= 1;}
    while (msg_length-- > 0)
    {
        if (TTY_LINCNT) {if (*pnt2 == '\n') {count_lines -= 1;}}
        *pnt1++ = *pnt2++;
    }
    *pnt1++ = '\n';
    *pnt1 = '\0';

    /*
     * Don't let line count go to zero because we want to retain context.
     */
    if (TTY_LINCNT)
    {
        if (count_lines <= 2)
        {
            /*
             * Filled up the screen, have user type RETURN
             */
            count_lines = number_lines;
    
            if (sav_ttyget != NULL)
            {
                char more_buffer[81];
                if ((*sav_ttyget)(str_inf_mor, &more_buffer[0], 80) == -1)
                {
                    TTY_PUT(" \n");
                    return (SS$_NOTPRINTED);
                }
            }
            else
            {
                struct dsc$descriptor_s more_dsc;
                struct dsc$descriptor_s prompt_dsc;
                char more_buffer[81];
    
                more_dsc.dsc$a_pointer = &more_buffer[0];
                more_dsc.dsc$w_length = 80;
                more_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
                more_dsc.dsc$b_class = DSC$K_CLASS_S;
    
                prompt_dsc.dsc$a_pointer = str_inf_mor;
                prompt_dsc.dsc$w_length = strlen(str_inf_mor);
                prompt_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
                prompt_dsc.dsc$b_class = DSC$K_CLASS_S;
    
                if (LIB$GET_INPUT(&more_dsc, &prompt_dsc, NULL) != SS$_NORMAL)
                {
                    TTY_PUT(" \n");
                    return (SS$_NOTPRINTED);
                }
            }
    
            TTY_PUT("\r\233A\233K");
        }
    }

    /*
     * Print the text
     */
    TTY_PUT(&msg_buffer[0]);
    return (SS$_NORMAL);
}

#endif

/*
 * unix_help --- Local routine
 *
 * Provide help for an UNIX/DOS system, using a VMS-style help library
 * source file.
 *
 * Inputs:
 *      (char *) help_libr_name - the key text
 *      (char *) full_key_user  - the key text
 *      (int)    print_text     - print help text list if TRUE
 *      (int)    print_topics   - print subtopics list if TRUE
 *
 * Outputs:
 *      (int) - One of RETURN_SUCCESS = Key found, help provided
 *                     RETURN_NONE    = Key not found
 *                     RETURN_FILE    = file access failure
 *
 * Notes:
 *      The "current_topic" variable is filled in with a string that
 *      indicated the current topic being displayed.
 *
 *      If the topic is found, the help text is shown (if print_text is
 *      set), the subtopics are shown (if print_topics is set).
 *
 *      If the topic is not found, a "no help" message is printed.
 *      If the topic is not the "list topics" character, then subtopics
 *      will not be listed.  Instead, the calling routine should re-call
 *      this function to list the subtopics for the last valid topic.
 *
 *      Any file access failure messages have already been printed.
 */

#ifndef VMS /* Only necessary on UNIX and DOS systems */

static int unix_help(help_libr_name, full_key_user, print_text, print_topics)

char    *help_libr_name;
char    *full_key_user;
int      print_text;
int      print_topics;

{
    char *unix_key_search();
    register char *cc;
    register char *cd;
    register int ind1;
    register int ind2;
    register int inpchar;
    register int  help_level;
    int topic_result;
    char key_user[HELP_BUFFER_SIZE+1];

    topic_result = RETURN_SUCCESS;  /* Assume we find user's topic */
    help_level = 1;

    if (full_key_user == NULL) return (RETURN_SUCCESS);

    current_topic[0] = '\0';
    save_level = 0;

    /*
     * Open the help file and get the first line from it.
     */    
    if ((help_file = fopen(help_libr_name, "r")) == NULL)
    {
        if (sav_ttyerr == NULL)
        {
            fprintf(stderr, pnt_err_lbr, help_libr_name, SYS_ERRMSG);
        }
        else
        {
            (*sav_ttyerr)(errno);
        }
        return (RETURN_FILE);
    }

    line_file[0] = ' ';
    line_file[1] = ' ';
    if ((fgets(&line_file[2], HELP_BUFFER_SIZE, help_file)) == NULL)
    {
        if (sav_ttyerr == NULL)
        {
            fprintf(stderr, pnt_err_lbr, help_libr_name, SYS_ERRMSG);
        }
        else
        {
            (*sav_ttyerr)(errno);
        }
        fclose(help_file);
        return (RETURN_FILE);
    }

    /*
     * Loop through each word of the key to search for, incrementing the
     * key level.
     */    
    for (cc = full_key_user; *cc != '\0'; cc += ind1, help_level++)
    {
        /*
         * Skip leading spaces
         */
        while (*cc == ' ')  cc++;

        /*
         * Get index past next word of key to look for (next space or EOL),
         * and save that portion
         */
        ind1 = !((char *)index(cc, ' ')) ? strlen(cc) : (char *)index(cc, ' ') - cc;
        strncpy(&key_user[0], cc, ind1);
        key_user[ind1] = '\0';

        /*
         * Search for that portion at the correct level
         */         
        if ((cd = unix_key_search(help_level, &key_user[0])) != NULL)
        {
            /*
             * If found that key word at the correct level, save the word
             * from the file (the user's input may have been abbreviated).
             * This will act as part of the current topic for the help text.
             */
            ind2 = !((char *)index(cd, ' ')) ? strlen(cd) : (char *)index(cd, ' ') - cd;
            *(cd+ind2) = '\0';
            if ((strlen(&current_topic[0]) + ind2 + 1) <= HELP_BUFFER_SIZE)
            {
                strcat(&current_topic[0], cd);
                strcat(&current_topic[0], " ");
            }
        }
        else
        {
            /*
             * Could not find the user's key in the file.  Just give the
             * options.
             */
            int  topic_length;
            int  topic_offset;

            topic_offset = 0;
            topic_length  = strlen(&current_topic[0]) + strlen(&key_user[0]);
            if (topic_length > number_columns)
            {
                topic_offset = topic_length - number_columns + 3;
                TTY_PUT("...");
            }

            if (key_user[0] != LIST_TOPICS)
            {
                TTY_PUT(str_inf_non);
                if (topic_offset < topic_length) {TTY_PUT(&current_topic[topic_offset]);}
                TTY_PUT(&key_user[0]);
            }
            else if (current_topic[0] != '\0')
            {
                TTY_PUT("\n");
                if (topic_offset < topic_length) {TTY_PUT(&current_topic[topic_offset]);}
            }

            TTY_PUT("\n\n");

            if (key_user[0] == LIST_TOPICS)
            {
                if (oops_string[0] != '\0')
                {
                    if (current_topic[0] == '\0') {TTY_PUT(" \n"); TTY_PUT(pnt_inf_mta);}
                    else                          {TTY_PUT(pnt_inf_sta);}
    
                    ind2 = strlen(oops_string) - 1;
                    if (oops_string[ind2] == '\n') {oops_string[ind2] = '\0';}
                    TTY_PUT(oops_string);
                    TTY_PUT("\n\n");
                }
                else
                {
                    if (current_topic[0] == '\0')  {TTY_PUT(pnt_inf_mtl);}
                    else if (*pnt_inf_stl != '\0') {TTY_PUT(pnt_inf_stl);}
                }
            }
#if 0
%%%
            if (print_topics == TRUE)
            {
                if (oops_string[0] != '\0')
                {
                    if (current_topic[0] == '\0') {TTY_PUT(" \n"); TTY_PUT(pnt_inf_mta);}
                    else                          {TTY_PUT(pnt_inf_sta);}
    
                    ind2 = strlen(oops_string) - 1;
                    if (oops_string[ind2] == '\n') {oops_string[ind2] = '\0';}
                    TTY_PUT(oops_string);
                    TTY_PUT("\n\n");
                }
                else
                {
                    if (current_topic[0] == '\0')  {TTY_PUT(pnt_inf_mtl);}
                    else if (*pnt_inf_stl != '\0') {TTY_PUT(pnt_inf_stl);}
                }
            }
#endif

            topic_result = RETURN_NONE;
            break;
        }
    }

    /*
     * Finished looking for user's key
     */
    if (topic_result == RETURN_SUCCESS)
    {
        /*
         * Found the key.  Print the help text if required
         */
        if (print_text)
        {
            /*
             * Print the keys from the file that got us here
             */
            if (TTY_LINCNT) {count_lines -= 2;}
            if (current_topic[0] != '\0')
            {
                int  topic_length;
                int  topic_offset;

                if ((TTY_LINCNT) && (count_lines <= 2))
                {
                    TTY_PUT(pnt_inf_mor);
                    tcsetattr(tty_fileno, TCSANOW, &tty_cbreak);
                    getchar();
                    tcsetattr(tty_fileno, TCSANOW, &tty_saved);
                    TTY_PUT("\r\233K");
                }

                TTY_PUT("\n");

                topic_offset = 0;
                topic_length  = strlen(&current_topic[0]);
                if (topic_length > number_columns)
                {
                    topic_offset = topic_length - number_columns + 3;
                    TTY_PUT("...");
                }
                TTY_PUT(&current_topic[topic_offset]);
            }
            TTY_PUT("\n\n");

            /*
             * Print the help text until the end of file, or until the next key
             * key definition line.
             */
            ind1 = 0;
            while (!isdigit(line_file[2]))
            {
                if (line_file[2] != '!')
                {
                    if (TTY_LINCNT)
                    {
                        count_lines -= 1;
                        if (count_lines <= 2)
                        {
                            TTY_PUT(pnt_inf_mor);
                            tcsetattr(tty_fileno, TCSANOW, &tty_cbreak);
                            inpchar = getchar();
                            tcsetattr(tty_fileno, TCSANOW, &tty_saved);
                            if ((inpchar == EOF) || (inpchar == 4) || (inpchar == 'q') || (inpchar == 'Q'))
                            {
                                count_lines = number_lines;
                                TTY_PUT(" \n");
                                break;
                            }
                            else if (inpchar == ' ') {count_lines = number_lines;}
                            else                     {count_lines += 1;}
                            TTY_PUT("\r\233K");
                        }
                    }
    
                    inpchar = strlen(&line_file[0]) - 1;
                    if (line_file[inpchar] != '\n')
                    {
                        line_file[inpchar++] = '\n';
                        line_file[inpchar] = '\0';
                    }
                    TTY_PUT(&line_file[0]);
                    ind1 = 1;
                }

                if ((fgets(&line_file[2], HELP_BUFFER_SIZE, help_file)) == NULL)
                {
                    break;
                }
            }

            if (ind1 != 0) {TTY_PUT("\n");}
        }

        /*
         * Collect information on sub-keys in the file, and tell the user.
         */
        if (print_topics)
        {
            while ((cd = unix_key_search(help_level, NULL)) != NULL) {}
            if (oops_string[0] != '\0')
            {
                ind2 = strlen(oops_string) - 1;
                if (oops_string[ind2] == '\n') {oops_string[ind2] = '\0';}
            }

            if (TTY_LINCNT)
            {
                if ( (oops_string[0] != '\0') && (count_lines <= 10) ||
                     (oops_string[0] == '\0') && (count_lines <=  5) )
                {
                    TTY_PUT(pnt_inf_mor);
                    tcsetattr(tty_fileno, TCSANOW, &tty_cbreak);
                    getchar();
                    tcsetattr(tty_fileno, TCSANOW, &tty_saved);
                    TTY_PUT("\r\233K");
                }
            }
    
            if (oops_string[0] != '\0')
            {
                if (current_topic[0] == '\0') {TTY_PUT(pnt_inf_mta);}
                else                          {TTY_PUT(pnt_inf_sta);}
                TTY_PUT(oops_string);
                TTY_PUT("\n\n");
            }
            else
            {
                if (current_topic[0] == '\0')  {TTY_PUT(pnt_inf_mtl);}
                else if (*pnt_inf_stl != '\0') {TTY_PUT(pnt_inf_stl);}
                else
                {
                    /*
                     * VMS "no subtopics" compatibility mode.  Fake things 
                     * out so the calling routine will automatically back 
                     * up to previous topic.
                     */
                    topic_result = RETURN_NONE;
                }
            }
        }
    }

    fclose(help_file);
    return(topic_result);
}

#endif

/*
 * unix_key_search --- Local routine
 *
 * Search for a help key
 *
 * Inputs:
 *      (int)    level - the key level number
 *      (char *) text  - the key text
 *
 *      If "text" is NULL, assume we already found the key, and we are now
 *      looking for all sub-keys.
 *
 * Outputs:
 *      (char *) - Non-NULL = Address of key from the file
 *                 NULL     = Key not found
 *
 * Notes:
 *      The "line_file" variable already contains the first line to check from
 *      the file.
 */

#ifndef VMS /* Only necessary on UNIX and DOS systems */

static char *unix_key_search(level, text)

int  level;
char *text;

{
    int  buffer_level;
    register int  len;
    register char *cc;
    register char *cd;

    if (level != save_level)
    {
        oops_string[0] = '\0';
        oops_cc = 0;
    }

    save_level = level;
    buffer_level = level;

    /*
     * Search through the file
     */
    do
    {
        /*
         * If the first character is a digit, then this is a KEY line
         */
        if (isdigit(line_file[2]))
        {
            /*
             * Convert the key line to key level number and key text
             */
            sscanf(&line_file[2], "%d %[^\n]\n", &buffer_level, &key_file[0]);
            cc = &key_file[0];

            /*
             * Check to see if this key is the same level as the one requested
             */
            if (level == buffer_level)
            {
                /*
                 * If no key was requested, use a default string
                 */
                if (text == NULL)   cd = &init_key[0];
                else                cd = text;

                /*
                 * Check the file key against the requested key
                 */
                while (*cd != '\0')
                {
                    if (istr_toupper(*cd) != istr_toupper(*cc++)) {break;}
                    cd++;
                }

                /*
                 * Check to see if they matched
                 */
                if (*cd != '\0')
                {
                    /*
                     * Keys did not match.  Remember the key from the file
                     * so it may possibly be printed, later.
                     */
                    len = strlen(&key_file[0]);
                    if (oops_cc + len > LINE_STOP)
                    {
                        oops_cc = 0;
                        strcat(oops_string, "\n");
                    }

                    if (oops_cc == 0)
                    {
                        oops_cc = 2;
                        strcat(oops_string, "  ");
                    }

                    strcat(oops_string, &key_file[0]);
                    oops_cc += len;

                    if (oops_cc > HELP_STOP)
                    {
                        oops_cc = 0;
                        strcat(oops_string, "\n");
                    }
                    else
                    {
                        while ((len -= HELP_TAB) > -1) {}
                        oops_cc += abs(len);
                        while(len++) {strcat(oops_string, " ");}
                    }
                    
                }

                /*
                 * Else, they did match.  In this case, get another line
                 * from the file (if no more lines, just return a NULL).
                 */
                else if ((fgets(&line_file[2], HELP_BUFFER_SIZE, help_file)) == NULL)
                {
                    return (NULL);
                }

                /*
                 * If they did match, and we did read a line from the file,
                 * then return a pointer to the key we found.
                 */
                else
                {
                    return (&key_file[0]);
                }
            }
        }

        /*
         * If we got here, then either we did not find a key line in the
         * file, or it was the wrong level, or they did not match.
         */
        if ((fgets(&line_file[2], HELP_BUFFER_SIZE, help_file)) == NULL)
        {
            return (NULL);
        }

    /*
     * Continue to loop through the file.  Stop if we wind up going down
     * a key level, since that means we are out of range for this key.
     */
    } while (level <= buffer_level);

    return (NULL);
}
            
#endif

/*
 * terminal_size --- Local routine
 *
 * Get the terminal size (characters and lines)
 *
 * Inputs:
 *      (char) type - 'c' to get number of characters per line
 *                    'l' to et number of lines
 *
 * Outputs:
 *      (int) - requested number
 *
 * Notes:
 *      If cannot get requested value, returns "0".
 */

static int terminal_size(type)

char type;

{

#ifdef VMS

    /*
     * Local structure definitions
     */
    struct item
    {
        short int buff_length;
        short int item_code;
        char *buff_address;
        short int *ret_length;
    };

    /*
     * System-specific local data
     */
    extern long unsigned SYS$GETDVI();
    extern long unsigned LIB$GET_EF();
    extern long unsigned LIB$FREE_EF();
    struct dsc$descriptor_s device_name;
    char *p_s_sysinput = "SYS$INPUT";
    long int i_event_flag;
    long int i_item_value;
    long int i_item_length;
    struct item item_list[2];

    /*
     * Check the argument
     */
    if ((type != 'l') && (type != 'L') && (type != 'c') && (type != 'C'))
    {
        return (0);
    }

    /*
     * In case something fails
     */
    i_item_value = 0;

    /*
     * Build the descriptor and the item list
     */
    device_name.dsc$w_length = strlen(p_s_sysinput);
    device_name.dsc$b_dtype = DSC$K_DTYPE_T;
    device_name.dsc$b_class = DSC$K_CLASS_S;
    device_name.dsc$a_pointer = p_s_sysinput;

    item_list[0].buff_length = 4;
    if ((type == 'l') || (type == 'L')) {item_list[0].item_code = DVI$_TT_PAGE;}
    if ((type == 'c') || (type == 'C')) {item_list[0].item_code = DVI$_DEVBUFSIZ;}
    item_list[0].buff_address = &i_item_value;
    item_list[0].ret_length = &i_item_length;

    item_list[1].buff_length = 0;
    item_list[1].item_code = 0;
    item_list[1].buff_address = NULL;
    item_list[1].ret_length = NULL;

    /*
     * Get an event flag, get the device information, and free the event flag
     */
    if (LIB$GET_EF(&i_event_flag) == SS$_NORMAL)
    {
        SYS$GETDVIW(i_event_flag, 0, &device_name, &item_list[0], 0, 0, 0, 0);
        LIB$FREE_EF(&i_event_flag);
    }

    /*
     * Return the requested information
     */
    return (i_item_value);

#else /* If not VMS */

#ifdef unix

    /*
     * System-specific local data
     */
    struct winsize tty_winsz;
    int            tty_fileno;

    if ((type != 'l') && (type != 'L') && (type != 'c') && (type != 'C'))
    {
        return (0);
    }

    tty_fileno = fileno(stdin);
    if (ioctl(tty_fileno, TIOCGWINSZ, &tty_winsz) >= 0)
    {
        if ((type == 'l') || (type == 'L')) {return(tty_winsz.ws_row);}
        if ((type == 'c') || (type == 'C')) {return(tty_winsz.ws_col);}
    }

    return(0);

#else /* If not VMS or UNIX */

    return(0);

#endif

#endif

}

/*
 * input_compress --- Local routine
 *
 * Compress extra spaces and tabs out of a string.  Remove trailing comment.
 * On VMS, this will also convert to uppercase.
 *
 * Inputs:
 *      (char *) p_s_buffer - character string
 *
 * Outputs:
 *      String is compressed in place.
 *
 * Notes:
 *      Tabs and other control characters will be converted to spaces.
 *      Leading and trailing spaces and tabs will be removed.  Multiple
 *      spaces will be converted to single spaces.
 *
 *      Strings enclosed in quotes are skipped over (not compressed and
 *      not upcased).
 */

#ifdef VMS
#define IF_VMS_UPCASE(chr) chr = istr_toupper(chr)
#else
#define IF_VMS_UPCASE(chr)
#endif

static int input_compress(p_s_buffer)

char *p_s_buffer;

{
    register char *p_s_work;
    register char *p_s_copy;

    /*
     * Search for comment and remove it if found (skip over quoted strings)
     * On VMS, this routine will also upcase the string.
     */
    p_s_work = p_s_buffer;
    while (TRUE)
    {
        if (*p_s_work == '"')
        {
            p_s_work++;
            while ( (*p_s_work != '"') && (*p_s_work != '\0') )
            {
                p_s_work++;
            }
        }
        if (*p_s_work == '\0')
        {
            break;
        }
        IF_VMS_UPCASE(*p_s_work);
        if ((*p_s_work == '!') || (*p_s_work == '#'))
        {
            *p_s_work = '\0';
            break;
        }
        else
        {
            p_s_work++;
        }
    }

    /*
     * Remove trailing spaces and control characters
     */
    p_s_work = p_s_buffer + strlen(p_s_buffer) - 1;
    while ( (p_s_work >= p_s_buffer) && (istr_isspace(*p_s_work) || istr_isnprnt(*p_s_work)) )
    {
        *p_s_work-- = '\0';
    }

    /*
     * Advance over leading spaces and control characters, and then compress
     * multiple spaces and/or control characters to single spaces.
     */
    p_s_work = p_s_buffer;
    p_s_copy = p_s_buffer;

    while ( (*p_s_work != '\0') && (istr_isspace(*p_s_work) || istr_isnprnt(*p_s_work)) )
    {
        p_s_work++;
    }

    while (TRUE)
    {
        if (*p_s_work == '"')
        {
            *p_s_copy++ = *p_s_work++;
            while ( (*p_s_work != '"') && (*p_s_work != '\0') )
            {
                *p_s_copy++ = *p_s_work++;
            }
        }

        if (*p_s_work == '\0')
        {
            break;
        }

        if ( istr_isprint(*p_s_work) && !istr_isspace(*p_s_work) )
        {
            *p_s_copy++ = *p_s_work++;
        }
        else
        {
            *p_s_copy++ = ' ';
            while ( (*p_s_work != '\0') && (istr_isspace(*p_s_work) || istr_isnprnt(*p_s_work)) )
            {
                p_s_work++;
            }
        }
    }

    *p_s_copy = '\0';
    return;
}

/*
 * get_message --- Local routine
 *
 * Get an error message for an error defined in the system message file.
 * This routine uses system-specific code.
 *
 * Inputs:
 *      (int) error_code - System error code (errno)
 *
 * Outputs:
 *      (char *) - Address of error message
 *                 Address of "" if can't find message
 */

#ifdef VMS
#define ERROR_FLAGS 0x1               /* Include text (not id, severity, or facility) */
static char get_message_error[256+1]; /* Buffer for message */
#else
static char get_message_error[1];     /* Buffer for null message */
#endif

static char *get_message(error_code)

int error_code;

{

#ifdef VMS

    /*
     * Local data
     */
    extern long unsigned SYS$GETMSG();          /* Get error message */
    struct dsc$descriptor_s dsc_vms_error;      /* String descriptor */
    short int error_length;

    /*
     * Copy the message into the local buffer
     */
    dsc_vms_error.dsc$a_pointer = &get_message_error[0];
    dsc_vms_error.dsc$w_length = 256;
    dsc_vms_error.dsc$b_dtype = DSC$K_DTYPE_T;
    dsc_vms_error.dsc$b_class = DSC$K_CLASS_S;

    error_length = 0;
    SYS$GETMSG(error_code, &error_length, &dsc_vms_error, ERROR_FLAGS, NULL);

    get_message_error[error_length] = '\0';
    return (&get_message_error[0]);

#else

    /*
     * Local data
     */
    char *pnt1;

    /*
     * Get the message and return it
     */
    pnt1 = (char *)strerror(error_code);

    if (pnt1 == NULL)
    {
        get_message_error[0] = '\0';
        pnt1 = &get_message_error[0];
    }

    return (pnt1);

#endif

}
