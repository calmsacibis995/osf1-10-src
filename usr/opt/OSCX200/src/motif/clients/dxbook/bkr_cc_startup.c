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
static char *rcsid = "@(#)$RCSfile: bkr_cc_startup.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 15:37:17 $";
#endif
/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1992  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Program to startup the character cell server, window manager,
**	and Bookreader with the right environment.
**
**  AUTHORS:
**
**      David L Ballenger
**
**  CREATION DATE:     09-Aug-1992
**
**  MODIFICATION HISTORY:
**
**--
**/


/*
 * INCLUDE FILES
 */
#ifdef vms
#include <ssdef.h>
#include <psldef.h>
#include <descrip.h>
#include <chfdef.h>
#include <clidef.h>
#include <climsgdef.h>
#include <libclidef.h>
#include <dvidef.h>
#include <iodef.h>
#include <lnmdef.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <lmfdef.h>		/* VMS license checking literals */
#include "br_application_info.h"

                               
/**********************
 *  LOCAL STRUCTURES  *
 **********************/
typedef struct {
    short	buffer_length;
    short	item_code;
    char	*buffer_address;
    int		*ret_len_address;
} ITMLST;      

static void	check_license ();
static int	get_binary_time();


#define INIT_DESC(dsc,str,len) { \
    dsc.dsc$a_pointer = str; \
    dsc.dsc$w_length  = len; \
    dsc.dsc$b_dtype   = DSC$K_DTYPE_T; \
    dsc.dsc$b_class = DSC$K_CLASS_D; }

#define NO_ARGS ""

typedef struct {
    char *symbol;
    char *logical_name;
    unsigned long process_id;
} SpawnInfo;



static void enable_ctrl_y_ast();
static void enable_cli_ctrl_y();
static void cleanup_and_exit();
static void wakeup();
static void spawn_ast(SpawnInfo *info);
static void exit_subprocess(SpawnInfo *info);
static void set_logical(char *name, char *value, unsigned int attributes);
static char *get_logical(char *name);
static void startup(SpawnInfo *info, char *arguments);
static void find_file(char *filespec_string,
                      char *default_filespec_string,
                      char *result_filepec_buffer
                      );



volatile SpawnInfo bookreader     = { "BKR","DECW$BOOKREADER", 0 };
volatile SpawnInfo window_manager = { "XCWM","XCWM", 0 } ;
volatile SpawnInfo server         = { "XC","XC", 0 };
    
static char startup_procedure[256];
static char *server_number = NULL;
static struct dsc$descriptor reverse_video;
static char *log_directory = "NL";
static int access_mode = PSL$C_USER;
static char *tt;
static int cli_ctrl_mask;


main(argc,argv)
    int argc;
    char **argv;
{
    char bkr_arguments[250];
    char *bkr_log;
    int status;
    int previous_handler; 

    /* Establish a condition handler to exit the subprocesses,
     * if something is terribly wrong.
     */
    status = SYS$SETEXV(2,
                        cleanup_and_exit,
                        &access_mode,
                        &previous_handler);
    if ((status & 1) == 0) {
	fprintf(stderr,"Can't set last chance handler.\n");
	exit(status);
    }
    LIB$ESTABLISH(cleanup_and_exit);
    if ((status & 1) == 0) {
	fprintf(stderr,"Can't set condition handler.\n");
	exit(status);
    }

    /* Set up a control Y ast to to exit the suprocesses and
     * exit if the user types CTRL-Y.
     */
    enable_ctrl_y_ast();

    /* If we receieve any of these signals now then do the
     * cleanup and exit.
     */
    signal(SIGHUP,cleanup_and_exit);
    signal(SIGINT,cleanup_and_exit);
    signal(SIGQUIT,cleanup_and_exit);
    signal(SIGILL,cleanup_and_exit);
    signal(SIGTRAP,cleanup_and_exit);
    signal(SIGIOT,cleanup_and_exit);
    signal(SIGEMT,cleanup_and_exit);
    signal(SIGFPE,cleanup_and_exit);
    signal(SIGKILL,cleanup_and_exit);
    signal(SIGBUS,cleanup_and_exit);
    signal(SIGSEGV,cleanup_and_exit);
    signal(SIGSYS,cleanup_and_exit);
    signal(SIGTERM,cleanup_and_exit);

    /* First see if we can run Bookreader in character cell mode.
     */
    check_license();

    /* If BKR$LOG is defined we'll put log files in this dierctory,
     * otherwise they will go to NL:.
     */
    bkr_log = get_logical("BKR$LOG");
    if ((bkr_log != NULL) && (strlen(bkr_log) != 0))
    {
	log_directory = "BKR$LOG";
    }
	
    tt = get_logical("TT");

    /* Look for the DCL command procedure that is used to start
     * Bookreader, the character cell window manager and the character
     * cell server.
     */
    find_file("DECW$BOOKREADER_CC","SYS$SYSTEM:*.COM",startup_procedure);

    /* Now start the character cell server.  This has to happen last
     * because it attaches a mailbox to the terminal and after that
     * occurs we can no longer spawn subprocesses.
     *
     * Note also that the input and output for the subprocess is directed
     * to the terminal.  This is not the case for the window manager and
     * Bookreader.
     *
     * We also check for the /REVERSE_VIDEO qualifier to see it we
     * need to pass -rv to the server.
     */
    INIT_DESC(reverse_video,"REVERSE_VIDEO",strlen("REVERSE_VIDEO"));
    startup(&server,
            ((CLI$PRESENT(&reverse_video) == CLI$_PRESENT) ? "-rv" : NO_ARGS));

    /* The startup procedure for the server does a SET DISPLAY/CREATE ...
     * and then defines DECW$XC_DISPLAY in the job logical name table to
     * point to the created display device. We then wait for the server
     * to start up by looping on the attempt to open the display using
     * the DECW$XC_DISPLAY logical name.
     */
    while (XOpenDisplay("DECW$XC_DISPLAY") == NULL)
    {
	sleep(1);
    }

    /* We know that the XC server has started, now get the 
     * server number and we'll append that to the process
     * names for the Bookreader and WindowManager subprocesses.
     */
    server_number = get_logical("DECW$XC_SERVER_NUMBER");

    /* Startup the window manager in their own subprocesses.
     */
    startup(&window_manager,NO_ARGS);
    startup(&bookreader,"-c");

    /* Now if we receieve any of these signals, we'll just wakeup if
     * we're hibernating and cleanup on the way out.
     */
    signal(SIGHUP,wakeup);
    signal(SIGINT,wakeup);
    signal(SIGQUIT,wakeup);
    signal(SIGILL,wakeup);
    signal(SIGTRAP,wakeup);
    signal(SIGIOT,wakeup);
    signal(SIGEMT,wakeup);
    signal(SIGFPE,wakeup);
    signal(SIGKILL,wakeup);
    signal(SIGBUS,wakeup);
    signal(SIGSEGV,wakeup);
    signal(SIGSYS,wakeup);
    signal(SIGTERM,wakeup);

    /* Hibernate until one of the child processes exits. Normally this
     * will be bookreader.
     */
    SYS$HIBER();

    cleanup_and_exit();
}

static void enable_ctrl_y_ast()
{
	
    static char *tt_name_string = "SYS$INPUT";
    static int disable_mask = LIB$M_CLI_CTRLY;

    struct dsc$descriptor tt_name;
    long iosb[2];
    int status;
    int tt_channel;

    INIT_DESC(tt_name,tt_name_string,strlen(tt_name_string));
    status = LIB$DISABLE_CTRL(&disable_mask,&cli_ctrl_mask);
    if ((status & 1) == 0) {
	fprintf(stderr,"LIB$DISABLE_CTRL\n");
	exit(status);
    }
    status = SYS$ASSIGN(&tt_name,&tt_channel,access_mode,NULL);
    if ((status & 1) == 0) {
	fprintf(stderr,"SYS$ASSIGN\n");
	exit(status);
    }

    status = SYS$QIOW(0,tt_channel,IO$_SETMODE|IO$M_CTRLYAST,&iosb,0,0,
                      cleanup_and_exit,0,access_mode,0,0,0);
    if ((status & 1) == 0) {
	fprintf(stderr,"SYS$QIOW\n");
	exit(status);
    }
}

static void enable_cli_ctrl_y()
{
    int old_mask;
    int status;

    status = LIB$ENABLE_CTRL(&cli_ctrl_mask,&old_mask);
    if ((status & 1) == 0) {
	exit(status);
    }
}

void 
cleanup_and_exit()
{
    /* Exit the bookreader and window manger suprocesses,
     * if they are still running.
     */
    exit_subprocess(&bookreader);
    exit_subprocess(&window_manager);

    if (server.process_id != 0)
    {
	if (LIB$AST_IN_PROG())
        {
	    /* Sleep for a couple of second to let the server clear
             * the screen.
             */
            sleep(2);
	}
	else
	{
	    /* Give the server a chance to recognize that the clients have gone
	     * away so it can clear the screen.
	     */
	    while (bookreader.process_id || window_manager.process_id)
	    {
		sleep(1);
	    }
	}
    	/* Do a FORCEX on the server, instead of DELPRC, to let it
    	 * reset the terminal characteristics before it exits.
    	 */
    	exit_subprocess(&server);
    }

    enable_cli_ctrl_y();

    exit(1);
}

void 
wakeup()
{
    SYS$WAKE(0,0);
}

void 
exit_subprocess(SpawnInfo *info)
{
    if (info->process_id != 0)
    {
        SYS$FORCEX(&info->process_id,NULL,1);
    }
}

void 
spawn_ast(SpawnInfo *info)
{
    info->process_id = 0;
    SYS$WAKE(0,0);
}

void 
set_logical(char *name, char *value, unsigned int attributes)
{

    unsigned int status;
    struct dsc$descriptor logical_name;
    struct dsc$descriptor translation;

    INIT_DESC(logical_name,name,strlen(name));
    INIT_DESC(translation,value,strlen(value));

    status = LIB$SET_LOGICAL(&logical_name,&translation,NULL,&attributes,NULL);

    if ( (status & 1) == 0 )
    {
        fprintf(stderr,"\nError defining %s\n",name);
        exit(status);
    }

}

typedef struct _VMS_ITEM_LIST {
    unsigned short  length; 	/*  buffer length  */
    unsigned short  itemcode;	/*  symbolic item code  */
    char           *pointer;	/*  buffer address  */
    unsigned short *ret_len;	/*  length returned by $TRNLNM  */
} VMS_ITEM_LIST ;


static char *get_logical(char *name)
{
#pragma nostandard
    $DESCRIPTOR (tablename_dsc, "LNM$FILE_DEV");    /*  table to search  */
#pragma standard

    unsigned	status;
    unsigned	lognam_len = 0;
    char	namebuff[LNM$C_NAMLENGTH];/*  buffer for name*/
    char	lognam[LNM$C_NAMLENGTH];/*  buffer for translation  */
    unsigned    attr_mask;
#pragma nostandard
    struct dsc$descriptor name_dsc = 	/*  logical name dsc.        */
	{strlen (name), DSC$K_DTYPE_T, DSC$K_CLASS_S, name};
#pragma standard

    VMS_ITEM_LIST item_list[2];			/*  array of item list dsc   */

    item_list[0].length =   LNM$C_NAMLENGTH;
    item_list[0].itemcode = LNM$_STRING;
    item_list[0].pointer =  lognam;
    item_list[0].ret_len =  (unsigned short *) &lognam_len;
    item_list[1].length =   0;		/*  null terminate the item list  */
    item_list[1].itemcode = 0;		

    attr_mask = LNM$M_CASE_BLIND;	/* ignore case  */    

    status = SYS$TRNLNM (&attr_mask, &tablename_dsc, &name_dsc, 0, item_list);

    if (status == SS$_NORMAL)
    {
        char	*trans_name = malloc (lognam_len + 1);
        strncpy (trans_name, lognam, lognam_len);
        trans_name[lognam_len] = 0; 
        return trans_name;
    }
    return NULL;
}

void
find_file(char *filespec_string,
          char *default_filespec_string,
          char *result_filespec_buffer
          )
{
    unsigned int status;
    unsigned int flags = 1;
    unsigned int context = 0;
    unsigned int rms_status = 0;
    struct dsc$descriptor_s filespec;
    struct dsc$descriptor_s result_filespec;
    struct dsc$descriptor_s default_filespec;
    char *semicolon;

    INIT_DESC(filespec,filespec_string,strlen(filespec_string));
    INIT_DESC(default_filespec,default_filespec_string,strlen(default_filespec_string));
    INIT_DESC(result_filespec,NULL,0);

    status = LIB$FIND_FILE(&filespec,
                           &result_filespec,
                           &context,
                           &default_filespec,
                           NULL,
                           &rms_status,
                           &flags);
    LIB$FIND_FILE_END(&context);

    if ( (status & 1) == 0 )
    {
        fprintf(stderr,"\nCan't find %s\n",filespec_string);
        exit(status);
    }

    strncpy(result_filespec_buffer,result_filespec.dsc$a_pointer,result_filespec.dsc$w_length);
    result_filespec_buffer[result_filespec.dsc$w_length] = '\0';
    semicolon = strchr(result_filespec_buffer,';');
    if (semicolon) {
        *semicolon = '\0';
    }
    
}

static void
startup(SpawnInfo *info,
        char *arguments
        )
{
    int	    status;
    int	    flags;
    struct  dsc$descriptor_s command;
    struct  dsc$descriptor_s process_name;
    struct  dsc$descriptor_s input;
    struct  dsc$descriptor_s output;
    char    name_buf[16];
    char    command_line[1000];
    char    input_file[1000];
    char    output_file[1000];
    char    image_file[1000];

    find_file(info->logical_name,"SYS$SYSTEM:*.EXE",image_file);

    sprintf(command_line,
            "@%s %s %s %s %s",
            startup_procedure,
            info->symbol,
            image_file,
	    tt,
            arguments);

    sprintf(input_file,"NL:%s.INPUT",info->symbol);
    sprintf(output_file,"%s:%s.LOG",log_directory,info->symbol);

    INIT_DESC(command, command_line ,strlen(command_line));
    INIT_DESC(input, input_file, strlen(input_file));
    INIT_DESC(output, output_file, strlen(output_file));

    if (server_number)
    {
        sprintf(name_buf,"%s_%s",info->symbol,server_number);
        INIT_DESC(process_name, name_buf,strlen(name_buf));
    }

    flags = CLI$M_NOCLISYM | CLI$M_NOWAIT ;

    status = lib$spawn(&command, &input, &output, &flags,
                       (server_number == NULL) ? NULL : &process_name, 
                       &info->process_id, 0, 0, 
                       spawn_ast, info, 0, 0 );
    if ( (status & 1) == 0 )
    {
        fprintf(stderr,"\nError starting: \"%s\".\n",command_line);
        exit(status);
    }
}

/*
**++
**  ROUTINE NAME:
**     	check_license ()
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called at startup to set the global variables (yuck)
**	that indicate which functionality to allow or disallow based on 
**	whether the BookreaderPlus license is installed.
**    
**  FORMAL PARAMETERS:
**	None
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None     
**
**  SIDE EFFECTS:
**	None     
**--
**/
static void check_license ()
{
    long		  binary_time_quad [2];
    long		  flags;
    short		  product_version [2];
    struct dsc$descriptor date_dsc, product_name_dsc, producer_dsc;
    ITMLST		  item_list [3];
    int			  status = 0;

    /* Set up version number */
    product_version [0] = BKR_CC_MINOR_VERSION;
    product_version [1] = BKR_CC_MAJOR_VERSION;

    /* Convert the date to binary time */
    date_dsc.dsc$a_pointer = BKR_CC_PRODUCT_DATE;
    date_dsc.dsc$w_length = strlen (BKR_CC_PRODUCT_DATE);
    date_dsc.dsc$b_class = DSC$K_CLASS_S;
    date_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    status = get_binary_time (&date_dsc, binary_time_quad);
    if (status != SS$_NORMAL)
	return;

    /* Create product name descriptor */
    product_name_dsc.dsc$a_pointer = BKR_CC_PRODUCT_NAME;
    product_name_dsc.dsc$w_length = strlen (BKR_CC_PRODUCT_NAME);
    product_name_dsc.dsc$b_class = DSC$K_CLASS_S;
    product_name_dsc.dsc$b_dtype = DSC$K_DTYPE_T;

    /* Create producer descriptor */
    producer_dsc.dsc$a_pointer = BKR_CC_PRODUCER;
    producer_dsc.dsc$w_length = strlen (BKR_CC_PRODUCER);
    producer_dsc.dsc$b_class = DSC$K_CLASS_S;
    producer_dsc.dsc$b_dtype = DSC$K_DTYPE_T;

    /* Create item list to hold the date and version */
    item_list[0].buffer_length = sizeof (binary_time_quad);
    item_list[0].item_code = LMF$_PROD_DATE;
    item_list[0].buffer_address = (char *) binary_time_quad;
    item_list[0].ret_len_address = 0;    
    item_list[1].buffer_length = sizeof (product_version);
    item_list[1].item_code = LMF$_PROD_VERSION;
    item_list[1].buffer_address = (char *) product_version;
    item_list[1].ret_len_address = 0;    
    item_list[2].buffer_length = 0;
    item_list[2].item_code = 0;

    /* Initialize flags */
    flags = LMF$M_RETURN_FAILURES;

    /* Check for the license */
    status = SYS$LOOKUP_LICENSE (&product_name_dsc,
				 item_list,
		  		 &producer_dsc,
				 &flags,
				 0,
				 0);

    if (status != SS$_NORMAL) {
	fprintf(stderr,"%%BOOKRDR-E-NOLICENSE, character cell mode requires a BOOOKREADER-CC license.\n");
	exit(status);
    }
}




/*
**++
**  ROUTINE NAME:
**	bkr_get_binary_time (time_dsc, time_addr)
**
**  FUNCTIONAL DESCRIPTION:
**	This routine is called by the check license command to convert the
**	ASCII date string to binary time.
**    
**  FORMAL PARAMETERS:
**	struct dsc$descriptor	*time_dsc;	- A descriptor containing the ASCII date
**	struct dsc$descriptor	*time_addr;	- A pointer to a quadword to receive the binary time
**
**  IMPLICIT INPUTS:
**	None
**
**  IMPLICIT OUTPUTS:
**	None
**
**  FUNCTION VALUE:
**	None
**
**  SIDE EFFECTS:
**	None
**--
**/
static int get_binary_time (time_dsc, time_addr)
    struct dsc$descriptor *time_dsc;
    long		  *time_addr;
{
    int			  status = 0;

#ifdef VMS
    status = SYS$BINTIM (time_dsc, time_addr);
#endif

    return (status);
}
#endif /* VMS */
