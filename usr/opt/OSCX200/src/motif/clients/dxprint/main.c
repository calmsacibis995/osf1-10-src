/*
**++
**  MODULE NAME:
**	main.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	 Print an image dump of part of the screen
**
**  ENVIRONMENT:
**
**	VMS V5.4, Ultrix V4.2, OSF/1 1.0, DW Motif 1.0
**
**  AUTHOR:
**      Ed Luwish
**
**  CREATION DATE:     
**	13-Jan-1992
**
**  MODIFICATION HISTORY:
**
**	 4-Feb-1992	Edward P Luwish
**		I18N modifications
**
**	13-Jan-1992	Edward P Luwish
**		Module created from parts of original Print Screen modules
**		main.c, prdw_c.c and printcb.c
**
**	17-Dec-1993	Dhiren M Patel
**		Added initialization of fit option and removed form_feed
**		option from the initialization list as it is obsolete.
**
**--
**/

#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /b5/aguws3.0/aguws3.0_rcs/src/dec/clients/print/main.c,v 1.2 91/12/30 12:48:20 devbld Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */

#define _MAIN_MODULE_ 1

#include "iprdw.h"
#include <stdio.h>
#include <stdlib.h>

#include "smdata.h"
#include "smconstants.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw_entry.h"

#ifdef VMS
#include "desc.h"
#include <jpidef.h>
#include <lnmdef.h>
typedef struct v_d_t
   {
    short string_length;
    unsigned char string_type;
    unsigned char string_class;
    unsigned char *string_address;           
   } vms_descriptor_type;

typedef struct sie
   {
    short buffer_length;
    short item_code;
    int *buffer_address;
    int *return_length_address;
   } set_itemlist_entry;

#define put_set_item(item_list, item_index, item_code_value, component)        \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].buffer_length = sizeof(component.value);             \
    item_list[item_index].item_code = item_code_value;                         \
    item_list[item_index].buffer_address = (int *)&component.value;            \
    item_list[item_index].return_length_address = (int *)&component.length;    \
   }

#define put_set_item_str(item_list, item_index, item_code_value, component)    \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].buffer_length = sizeof(component.value);             \
    item_list[item_index].item_code = item_code_value;                         \
    item_list[item_index].buffer_address = (int *)component.value;             \
    item_list[item_index].return_length_address = (int *)&component.length;    \
   }

#define end_set_itemlist(item_list, item_index)                                \
   {                                                                           \
    item_index++;                                                              \
    item_list[item_index].buffer_length = 0;                                   \
    item_list[item_index].item_code = 0;                                       \
    item_list[item_index].buffer_address = 0;                                  \
    item_list[item_index].return_length_address = 0;                           \
   }

#define start_set_itemlist(item_list, item_index)                              \
   {                                                                           \
    item_index = -1;                                                           \
    item_list[0].buffer_length = 0;                                            \
    item_list[0].item_code = 0;                                                \
    item_list[0].buffer_address = 0;                                           \
    item_list[0].return_length_address = 0;                                    \
   }
#else
#endif

unsigned int	global_debug_flag = 0;
int		clock_time;
char		tempFilename[256];
char		equiv_string[256];
int		equiv_string_length;
XtAppContext	applicationContext;

static void takeDownCopyright PROTOTYPE((Widget	titlewidget));

static int InitializeDebug PROTOTYPE ((void));

main
#if _PRDW_PROTO_
(
    int		argc,
    char	*argv[]
)
#else
(argc, argv)
    int		argc;
    char	*argv[];
#endif
{
    unsigned int	status;		/* subroutine return code	*/
    unsigned int	i;
    unsigned int	value, done = 0;
    int			dfs;
    Display		*dpy;		/* display id			*/
    Window		window;		/* window id of image		*/
    char		*str_value;
    XEvent		event;
    unsigned int	doneWithCopyright = 0;
    Command		tempCommand;

    /* initialize debug environment */

    status = InitializeDebug();

    /*
    ** initialize options block
    */
    /* dp:	added initialization of fit option and removed
     *		form_feed option from the initilization list
     *		(it is obsolete).
     */
    command.options.fit = dxPrscCrop;
    command.options.reverse_image = dxPrscDefault;
    command.options.print_color = dxPrscPrinterBW;	/* Black & White */
    command.options.storage_format = dxPrscDefault;	/* PostScript */
    command.options.print_widget = dxPrscQuickPrint;
    command.options.send_to_printer = dxPrscFile;
    command.options.aspect = dxPrscDefault;
    command.options.time_delay = 0;
    command.options.partial_capture = dxPrscFull;
    command.options.x_coord = 0;
    command.options.y_coord = 0;
    command.options.h_coord = 0;
    command.options.w_coord = 0;
    command.options.run_mode = dxPrscNormalCapture;
    command.options.command_mode = dxPrscShellmode;
    command.print_dest[0] = 0;

    status = ws_init (&argc, argv);
    if (!status) finish();

    dpy = XtDisplay(smdata.toplevel);
    dfs = XScreenNumberOfScreen(XtScreen(smdata.toplevel));
    window = RootWindowOfScreen (XtScreen(smdata.toplevel));

    parse (argc, argv);

    if ((command.options.run_mode & dxPrscRenderOnly) == 0)
    {
	if ((command.options.run_mode & dxPrscFromCallback) == 0)
	{
	    if (command.options.command_mode != dxPrscXmode)
	    {
		/*
		** dxprint [-afrt][-bcg][-hwxy][-dpsT][-S][-oF]
		**
		** command line entry from user shell, not batch job
		*/

		if (command.options.time_delay != 0)
		    sleep (command.options.time_delay);

		XGrabServer (display_id);
 
		if(command.options.partial_capture == dxPrscFull)
		{
		    status = dxPrscPrintScreenWindow
			(dpy, dfs, window, &command, smdata.toplevel); 
		}
		else 
		{
		    status =  dxPrscPrintScreenRect
		    (
			display_id,
			dfs,
			&command,
			(int)command.options.x_coord,
			(int)command.options.y_coord,
			(unsigned int)command.options.w_coord,
			(unsigned int)command.options.h_coord,
			smdata.toplevel
		    );
		}
 
		if (BAD(status)) exit (status); 

		exit(status = Normal);

	    } /* -X */
	    else
	    {
		/*
		** dxprint [-TD]-X
		**
		** Create Motif interface - all options other than T and D
		** are ignored
		*/
		smdata.err_window_id = 0;
		smdata.caution_id = 0;

		/*
		** create the control panel
		*/
		status = create_panel();
		if (!status) finish();

		/*
		** get events forever.  We get out by the quit button
		*/
		XtAppMainLoop (applicationContext);
	    } /* -X */
	} /* -I */
    } /* -R */

    exit(Normal);
} /* main */

static void takeDownCopyright (titlewidget)
Widget	titlewidget;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Remove the copyright notice from the title bar and replace it
**	with the title the user requested.
**
**  FORMAL PARAMETERS:
**
**	titlewidget - The widget to change to title on.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
*/
{
    Arg		arglist[3];
    char	*title_text;
    XmString	title_cs;

    title_cs = get_drm_message (k_sm_title_msg);

    DWI18n_SetTitle (titlewidget, title_cs);

}

int finish()
{
    exit(1);
}

int getEquiv
#if _PRDW_PROTO_
(
    char	*env,
    char	*equiv
)
#else
(env, equiv)
    char	*env;
    char	*equiv;
#endif
{
    char	*temp;
    int		len;
    /*
    ** Convert the logical or environment variable.
    */
    temp = getenv(env);

    /*
    ** If too big for the buffer or no value.
    */
    if ((temp == NULL) || ((len = strlen(temp)) >= 256) || (len == 0))
    {
	equiv[0]=0;
	return (False);
    }

    /*
    ** Uses a single buffer that must be cleared.
    */
    strcpy (equiv, temp);

    return (True);
}

int execShellCmd
#if _PRDW_PROTO_
(
    char	*command_string
)
#else
(command_string)
    char	*command_string;
#endif
{
#ifdef VMS
#pragma nostandard
    DCLDESC (dcl_command);
#pragma standard
    FILLDESC_R (dcl_command, command_string);
    return (lib$spawn (&dcl_command));
#else
    system (command_string);
    return (Normal);
#endif
}

/* InitializeDebug sets up global_debug_flags according to logical names or
   environmental variables specific to PRINTSCREEN.  It also gets the initial
   clock time. */

static int InitializeDebug ()
{
#ifdef VMS
    int debug_debugger, status;
#pragma nostandard
    DCLDESC(dcl_command);
    DCLDESC(name_table);
    DCLDESC(log_name);
#pragma standard

    FILLDESC_R(name_table,"LNM$FILE_DEV");
    FILLDESC_R(log_name,"DXPRINT_DBG_ON");
    status = sys$trnlnm(0,&name_table,&log_name,0,0) - 1;

    if (status == 0) global_debug_flag = DbgOn;

    FILLDESC_R(name_table,"LNM$FILE_DEV");
    FILLDESC_R(log_name,"DXPRINT_DBG_INIT_ONLY");
    debug_debugger = sys$trnlnm(0,&name_table,&log_name,0,0) - 1;


    if (debug_debugger == 0)
    {
	printf("\n==========================================\n\n");

        FILLDESC_R(dcl_command,"$ SHOW LOGICAL/PROCESS");
        status = lib$spawn(&dcl_command);

        printf("\n");

        FILLDESC_R(dcl_command,"$ SHOW LOGICAL/JOB");
        status = lib$spawn(&dcl_command);

        printf("Status of DXPRINT_DBG_INIT_ONLY is %d\n",status);
        global_debug_flag = DbgOn;
        Dprintf(("Trying out Dprintf from the init routine\n"), DbgMsgOnly );
        finish();
    };
#else
#endif
    return 0;
}

void PrintDebugInfo
#if _PRDW_PROTO_
(
    unsigned int	debug_flags
)
#else
(debug_flags)
    unsigned int	debug_flags;
#endif
/* PrintDebugInfo is typically called as a result of a macro expansion such
as:

Dprintf( (printf_string), flags);

which expands into:

if (global_debug_flag) {
	printf(printf string);
	PrintDebugInfo(flags);
}

The "printf_string" can be any legal argument to a printf, including variable
names.  Thus any program variable can be examined at any point where Dprintf
is called.

An initialization routine has already set global_debug_flag according to
logical names or environmental variables.  One of these is an "override" bit
that causes the global flags to XOR with the flags specified in the function
call.  The normal behavior is to OR the flags.

If no global debug flags are specified, the macro expansion ensures that this
function never gets called.  Therefore very little overhead is added to the
program.

*/
{
#ifdef VMS
#pragma nostandard
    DCLDESC(time_string_descriptor);
#pragma standard
    char time_string[30];
    jpi  process_info;
    set_itemlist_entry jpi_itemlist[32];
    int index, flags, status;

    /* 'OR' the flag argument with the global debug flags derived from logical
    ** names, or 'XOR' them, depending on the override bit set in the global
    ** debug flags
    */

    if ((global_debug_flag & DbgOverride) == 0)
	flags = global_debug_flag | debug_flags;
    else
	flags = global_debug_flag ^ debug_flags;

    /* Set up sys$getjpiw call */

    start_set_itemlist(jpi_itemlist, index);

    /* Process identification */

    put_set_item(jpi_itemlist, index, JPI$_PID, process_info.pid);
    put_set_item_str(jpi_itemlist, index, JPI$_PRCNAM, process_info.prcnam);
    put_set_item_str(jpi_itemlist, index, JPI$_USERNAME, process_info.username);

    /* Process characteristics and ownership */

    put_set_item(jpi_itemlist, index, JPI$_JOBTYPE, process_info.jobtype);
    put_set_item(jpi_itemlist, index, JPI$_MODE, process_info.mode);
    put_set_item(jpi_itemlist, index, JPI$_MASTER_PID, process_info.master_pid);
    put_set_item(jpi_itemlist, index, JPI$_OWNER, process_info.owner);
    put_set_item(jpi_itemlist, index, JPI$_UIC, process_info.uic);

    /* Process quotas */

    put_set_item(jpi_itemlist, index, JPI$_DFPFC, process_info.dfpfc);
    put_set_item(jpi_itemlist, index, JPI$_DFWSCNT, process_info.dfwscnt);
    put_set_item(jpi_itemlist, index, JPI$_PGFLQUOTA, process_info.pgflquota);
    put_set_item(jpi_itemlist, index, JPI$_WSAUTH, process_info.wsauth);
    put_set_item(jpi_itemlist, index, JPI$_WSAUTHEXT, process_info.wsauthext);
    put_set_item(jpi_itemlist, index, JPI$_WSEXTENT, process_info.wsextent);
    put_set_item(jpi_itemlist, index, JPI$_WSQUOTA, process_info.wsquota);

    /* Dynamic information */

    put_set_item(jpi_itemlist, index, JPI$_PRI, process_info.pri);
    put_set_item(jpi_itemlist, index, JPI$_CPUTIM, process_info.cputim);

    put_set_item(jpi_itemlist, index, JPI$_FREPTECNT, process_info.freptecnt);
    put_set_item(jpi_itemlist, index, JPI$_PAGEFLTS, process_info.pageflts);
    put_set_item(jpi_itemlist, index, JPI$_PAGFILCNT, process_info.pagfilcnt);
    put_set_item(jpi_itemlist, index, JPI$_PPGCNT, process_info.ppgcnt);
    put_set_item(jpi_itemlist, index, JPI$_VIRTPEAK, process_info.virtpeak);

    put_set_item(jpi_itemlist, index, JPI$_WSPEAK, process_info.wspeak);
    put_set_item(jpi_itemlist, index, JPI$_WSSIZE, process_info.wssize);

    end_set_itemlist(jpi_itemlist, index);

    /* call $GETJPIW */

    status = sys$getjpiw (0, 0, 0, jpi_itemlist, 0, 0, 0);

    /* get clock time */

    INITDESC_W(time_string_descriptor, time_string);

    status = lib$date_time(&time_string_descriptor);

    /* terminate time_string */

    time_string[time_string_descriptor.dsc$w_length] = 0;

    /* translate timestamp into milliseconds since initialization */
    /* translate mode and jobtype */
    /* put zero at end of strings (prcnam and username) */

    if ((flags & DbgMsgOnly) ==0)
    {

	/* print the header info, if selected */

	if ((flags & DbgProcStatic) !=0)
	{
	    printf("Process Identification\n");
	    printf("   Process Id     = %x\n", process_info.pid.value);
	    printf("Process characteristics and ownership\n");
	    printf("   Job Type       = %d\n", process_info.jobtype.value);
	    printf("   Process Mode   = %d\n", process_info.mode.value);
	    printf("   Master PID     = %x\n", process_info.master_pid.value);
	    printf("   Owner PID      = %x\n", process_info.owner.value);
	    printf("   Process UIC    = %x\n", process_info.uic.value);
	    printf("Process quotas\n");
	    printf("   Pg Flt Cluster = %d\n", process_info.dfpfc.value);
	    printf("   Dflt WS size   = %d\n", process_info.dfwscnt.value);
	    printf("   Pg File Quota  = %d\n", process_info.pgflquota.value);
	    printf("   Auth WS Size   = %d\n", process_info.wsauth.value);
	    printf("   Auth WS Extent = %d\n", process_info.wsauthext.value);
	    printf("   Curr WS Extent = %d\n", process_info.wsextent.value);
	    printf("   Curr WS Quota  = %d\n", process_info.wsquota.value);
	}
	/* print the dynamic info, including clock time */

	printf("Process Dynamic Information\n");
	printf("   Clock Time     = %s\n", time_string);
	printf("   Sched Priority = %d\n", process_info.pri.value);
	printf("   Accum CPU Time = %d\n", process_info.cputim.value);
	printf("   Free Proc VM   = %d\n", process_info.freptecnt.value);
	printf("   Page Faults    = %d\n", process_info.pageflts.value);
	printf("   PGFLQUOTA Left = %d\n", process_info.pagfilcnt.value);
	printf("   Pages in WS    = %d\n", process_info.ppgcnt.value);
	printf("   Peak VM alloc  = %d\n", process_info.virtpeak.value);
	printf("   Peak WS size   = %d\n", process_info.wspeak.value);
	printf("   Curr WS size   = %d\n", process_info.wssize.value);
	printf("\n==========================================\n\n");

	/* get additional info and print according to flag bits */

    }
#else
#endif
    return;
}
