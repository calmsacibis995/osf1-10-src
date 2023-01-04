/* #module TEA_printer "X2.0-5" */
/*
 *  Title:	TEA_printer
 *
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1990, 1993  All Rights      |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  
 *  Module Abstract:
 *
 *	This module contains the DECterm application (TEA) code needed to
 *	implement printer port support.  It includes the callback routines
 *	for the Print menu and the DECterm widget.
 *
 *  Author:	Bob Messenger
 *
 *  Modification history:
 *
 *  Alfred von Campe    15-Oct-1993     BL-E
 *	- Change #includes to compile on OSF/1.
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Eric Osman		20-Jul-1993	VXT V2.1
 *	- If can't open printer, put up warning box so users without message
 *	  box process on vxt see warning anyway.
 *
 * Eric Osman		19-Jul-1993	VXT V2.1
 *	- Don't print 2 printer errors, 1 is enough
 *
 * Eric Osman		17-May-1993	VXT 2.1
 *	- Merge vxt and vms decterm sources
 *
 *  Alfred von Campe    04-Feb-1993     Ag/BL12 (MUP)
 *      - Use a pre-determined filename when printing to a queue.
 *
 *  Alfred von Campe    11-Dec-1992     Ag/BL11
 *      - Set the file ownership to the real uid & gid after printing to a file.
 *      - Remove obsolete priviledge handling code on VMS.
 *
 *  Eric Osman		 8-Oct-1992	VXT V1.2
 *	- Remove extraneous printf's
 *  Eric Osman		20-Aug-1992	VXT V1.2
 *	- Wait 3 seconds before commencing "print graphics" menu item, to let
 *	  menu disappear from screen.
 *
 *  Eric Osman		30-Jul-1992	VXT V1.2
 *	- When doing "finish" or "cancel" printing, set printer mode back to
 *	  normal (i.e. turn off autoprint and printer-controller mode) .
 *	- Don't loop saying "can't write to printer".  Fix is to wait 'til
 *	  end of finish_printing before calling start_input.
 *	- Put bells in "printer not available" message.
 *	- Say "printer not available" if printer is off.
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Aston Chan		15-Nov-1991	V3.1
 *	- Use XmStringFree() to free an XmString instead of XtFree().
 *
 *  Alfred von Campe    06-Oct-1991     Hercules/1 T0.7
 *      - Corrected one instance of variable file_name from filename.
 *      - Added #extern definition for *convert_widget_to_stream().
 *
 *  Michele Lien    11-Dec-1990	VXT X0.0 BL2
 *  - VXT can only direct printer output to a dedicated printer port, "printer
 *    destination" option in printer customize menu for DECterm V3.0 has been 
 *    taken out.  All the resources associated with printer destination and 
 *    port name can only be conditionally compiled for non-VXT systems.  
 *    Printer destination and printer port name has been hard coded to the 
 *    local printer port.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 *  Bob Messenger	20-Jul-1990	X3.0-5
 *	- Make sure the user has the right privileges before opening the
 *	  printer port or file.
 *	- First cut at Cancel Printing: just close the file.
 *
 *  Bob Messenger	30-Jun-1990	X3.0-5
 *	- Initial version.
 */

#include "mx.h"

#if !defined (VMS_DECTERM) && !defined (VXT_DECTERM)
#include <unistd.h>
#include <sys/file.h>		/* for symbols such as O_CREAT */
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef VXT_DECTERM
#include "vxtio.h"
#include "vxtprinter.h"
#include "msgboxconstants.h"
#include "vxtconfig.h"
#else
#include <errno.h>		/* get symbol errno */
#ifdef VMS_DECTERM
#include <file.h>		/* for symbols such as O_CREAT */
#include <perror.h>		/* vms sys_errlist. On unix, it's in errno.h */
#endif
#endif

globalref XtAppContext TEA_app_context;

/* 
 * External declarations 
 */

extern STREAM *convert_widget_to_stream();

#ifdef VXT_DECTERM
extern void VxtPrinterGetStatus();
#endif VXT_DECTERM

extern p_stop_read();
extern p_resume_read();

/*
 * Forward declarations.
 */

void start_printing(), finish_printing();
void cancel_printing_cb();
static void non_block_write();
static struct prt_buf_struct *search_for_end_of_link_list();
static void free_all_link_lists();
static void non_block_read();
void stop_read_data_from_prt();
void read_data_from_prt();

/* defines */
#define LAST_LINE_INDICATOR	-1
#define MAX_REPORT_QUEUE	4

/*
 * start_printing_handler
 *
 * This routine is called by the DECterm widget at the start of a text or
 * graphics print screen operation, or when entering auto print or controller
 * mode.  It is passed as the DECwNstartPrintingCallback resource.
 */

void start_printing_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    int *call_data;
{
    Arg arglist[1];
    int print_pending;

    if ( *call_data == DECwCRStartPrintScreen )
	start_printing( stm );

    if ( stm->printer.active )
    {
    	XtSetArg( arglist[0], DECwNprinterPending, &print_pending );
    	XtGetValues( stm->terminal, arglist, 1 );
	print_pending++;
    	XtSetArg( arglist[0], DECwNprinterPending, print_pending );
    	XtSetValues( stm->terminal, arglist, 1 );
    }
}

/*
 * stop_printing_handler
 *
 * This routine is called by the DECterm widget at the end of a text or
 * graphics print screen operation, or when exiting auto print or controller
 * mode.  It is passed as the DECwNstopPrintingCallback resource.
 */

void stop_printing_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    int *call_data;
{
        finish_printing( stm );
}

static struct prt_buf_struct *search_for_end_of_link_list( root )
struct prt_buf_struct *root;
{
struct prt_buf_struct *prev_buf;

	prev_buf = (struct prt_buf_struct *) root;
 	while( prev_buf->next )
	    prev_buf = (struct prt_buf_struct *) prev_buf->next;
	return( prev_buf);
}

/* This is used in the  non-blocking printing algorithm to
   (a) Stop reading data from the pty
   (b) Stop processing keyboard data.  Character-generating
       keys will be buffered in the input silo, but if the 
       user presses the F2 key (print page), DECterm will
       ring the bell and otherwise ignore the key.
   (c) Gray out the Print Screen commands in the Print menu.
*/

void stop_reading_input( w, stm )
Widget w;
STREAM *stm;
{

    stm->printer.stop_reading_input = True;
                  
    p_stop_read (stm);

    /* Gray out Print Screen option in the Print menu. */
}

void start_reading_input( w, stm )
Widget w;
STREAM *stm;
{

    stm->printer.stop_reading_input = False;

    p_resume_read (stm);

    /* Un-gray Print Screen option in the Print menu. */
}

/*
 * print_line_handler
 *
 * This routine is called by the DECterm widget to send a line (text or sixels)
 * to the printer port.  It is passed as the DECwNprintLineCallback resource.
 */

void print_line_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    DECwTermInputCallbackStruct *call_data;
{
    Arg arglist[1];
    char *data = call_data->data;
    int count = call_data->count;
    struct prt_buf_struct *current_buf, *prev_buf;
    int bytes_written;

    if ( !stm->printer.active )
	start_printing( stm );

    if ( count == 0 )
	return;

    if ( count >= 2 && data[count-2] == '\r' && data[count-1] == '\n' )
	{
	/*
	 * If the line ends with a carriage return / line feed, convert it to
	 * just a line feed for the benefit of the C library.
	 */
	data[count-2] = '\n';
	count--;
	}

    if ( stm->printer.active )
	{

	/* By using blocking writes, there was a problem that one DECterm
	   window could stall the rest of the DECterm windows if printing takes
	   a long  time or if the printer unplugs. "Clear Comm" would not cancel
	   printing because DECterm was stuck in the printer routine and 
	   never went back to the main event loop in order to execute "Clear 
	   Comm". The solution is to use non-blocking write routines.
	   Here is the printer port buffering algorithm for non-blocking 
	   write (printer buffer has threshold of 2k bytes) :

           1. Use non-blocking I/O when writing to the printer port.
           
              XtAppAddInput will be called when there is data to
              be printed.  Whenever the printer buffer is empty, 
              XtRemoveInput will be called.  XtAppAddInput and 
              XtRemoveInput may be called many times before exiting 
              out of a printer mode.
           
           2. If the DECterm widget has data to write to the printer but
              there is already a write in progress, the TEA will buffer 
              this data in malloc'ed memory and return from the callback.

           3. After reaching a threshold (e.g. 2048 bytes) of buffered
              data, DECterm will take the following actions:
              (a) Stop reading data from the pty
              (b) Stop processing keyboard data.  Character-generating
                  keys will be buffered in the input silo, but if the 
                  user presses the F2 key (print page), DECterm will
                  ring the bell and otherwise ignore the key.
              (c) Gray out the Print Screen commands in the Print menu.

           4. If DECterm can not allocate enough memory to buffer the
              data to be sent to the printer port, it will perform an
              implicit Cancel Printing (i.e. discard all the buffered
              data, cancel any pending I/O and close the printer port)
              and will then put up a warning box.

           5. When the printer port buffer empties, DECterm will reverse
              the action listed in (3).
        */

	if (stm->printer.wr_id <= 0 )
	    {
            stm->printer.wr_id = XtAppAddInput( TEA_app_context,
#ifdef VMS_DECTERM
				       stm->printer.wr_efn,
				       0,
#else
				       stm->printer.file,
				       (XtPointer)XtInputWriteMask,
#endif
				       non_block_write,
				       stm );
	    }
	if (stm->printer.prt_buf == NULL )
	{
	    current_buf = (struct prt_buf_struct *) stm->printer.prt_buf 
			= (struct prt_buf_struct *) 
			      XtMalloc( sizeof( struct prt_buf_struct) );
	}
	else
	{
	    prev_buf = (struct prt_buf_struct *) search_for_end_of_link_list
						      ( stm->printer.prt_buf );
	    current_buf = (struct prt_buf_struct *) prev_buf->next
			= (struct prt_buf_struct *) 
			  XtMalloc( sizeof( struct prt_buf_struct) );
	}	

	if ( current_buf == NULL )
	{
	    /* malloc has failed, perform Cancel Printing, and free all 
		the printer buffers */

	    log_error(" malloc failed, cancel printing\n");
	    cancel_printing_cb(w);
	    return;
	}
	current_buf->next = NULL;
	if ( (current_buf->data_buf = XtMalloc(count)) == NULL )
    	{
	    /* malloc has failed, perform Cancel Printing, and free all 
	   	the printer buffers */

	    log_error(" malloc failed, cancel printing\n");
	    cancel_printing_cb(w);
	    return;
	}
	memcpy (current_buf->data_buf, data, count);
	current_buf->count = count;
	current_buf->start_ptr = current_buf->data_buf;
	(stm->printer.depth_of_link_list)++;

	if ( stm->printer.depth_of_link_list >= MAX_DEPTH_OF_LINK_LIST )
	{
	    stop_reading_input( w, stm );
	}

	if ( stm->printer.status != DECwPrinterReady )
	    {
	    XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
	    XtSetValues( stm->terminal, arglist, 1 );
	    }
	}
}

/* Routine to write to printer in a non-blocking mode.
 *
 */
static void non_block_write( stm )
STREAM *stm;
{
	int bytes_written;
	struct prt_buf_struct *current_buf, *next_buf;
	Arg arglist[1];
	int print_pending;

	/* This routine only prints one line at a time, even though there might
	   be more lines in the link list of printer buffers.  One link list 
	   contains one line of buffer */

	current_buf = (struct prt_buf_struct *) stm->printer.prt_buf;

	if ( current_buf == NULL )
	{
	    /* no more buffered data */

	    XtRemoveInput(stm->printer.wr_id);
	    stm->printer.wr_id = NULL;
	    stm->printer.prt_buf = NULL;
	    stm->printer.depth_of_link_list = 0;
	    if ( stm->printer.stop_reading_input )
	    {
	        start_reading_input( stm->terminal, stm );
	    }
	    return;
	}

	if ( current_buf->start_ptr[0] == LAST_LINE_INDICATOR )
	{
    	    XtSetArg( arglist[0], DECwNprinterPending, &print_pending );
    	    XtGetValues( stm->terminal, arglist, 1 );
	    print_pending--;
	    if ( print_pending < 0 )
		print_pending = 0;
    	    XtSetArg( arglist[0], DECwNprinterPending, print_pending );
    	    XtSetValues( stm->terminal, arglist, 1 );
	    if (!print_pending )
	    {	    
	    	/* All the data has been written to the printer, now close
	       	   the printer */

	    	finish_printing( stm );
		return;
	    }
	}
	bytes_written =
			write ( stm->printer.file,
				current_buf->start_ptr,
				current_buf->count );
	/*
	 * On errors, we set count to buffer size so that we proceed as
	 * though this buffer was written.  Other possible action would be
	 * to close printer down immediately.
	 */
	if ( bytes_written < 0 )
	{
	    log_error(" write to printer failed, write returned %d\n",
		bytes_written);
	    bytes_written = current_buf->count;
	}

	if (bytes_written == 0 && current_buf->count != 0)
	{
	    log_error ("unexpectedly got 0 from printer write\n");
	    bytes_written = current_buf->count;
	}

	if ( ( current_buf->count -= bytes_written ) < 0 )
	{
	    current_buf->count = 0;
	    log_error ("error: printed too many bytes from DECterm\n");
	    return;
	}
	else if ( current_buf->count == 0 )
	{
	    /* finished printing the line, go to the next line */

	    next_buf = (struct prt_buf_struct *) current_buf->next;

	    /* free the current buffer */

	    XtFree(current_buf->data_buf);
	    XtFree((char *)stm->printer.prt_buf);
	    stm->printer.prt_buf = (struct prt_buf_struct *) next_buf;
	    (stm->printer.depth_of_link_list)--;
	}
	else
	{
	    current_buf->start_ptr += bytes_written;
	}

	/* DECterm would start reading input from PTY again after buffered 
	   data goes below the threshold. */

	if ( stm->printer.depth_of_link_list < MAX_DEPTH_OF_LINK_LIST && 
		stm->printer.stop_reading_input )
	{
	    start_reading_input( stm->terminal, stm );
	}

}

/*
 * printer_status_handler
 *
 * This routine is called by the DECterm widget to update the value of the
 * DECwNprinterStatus resource.  It is passed as the DECwNprinterStatusCallback
 * resource.
 */

void printer_status_handler( w, stm, call_data )
    Widget w;
    STREAM *stm;
    int *call_data;
{
    Arg arglist[1];

    XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
    XtSetValues( stm->terminal, arglist, 1 );
}

/*
 * The get_efn routine is for obtaining efn's used on vms, since the
 * XtAppAddInput routine requires an efn instead of a file channel.
 */

#if defined(VMS_DECTERM)

static int get_efn (result) int *result;
{
int status;
status = u_open_wake_channel (result);

if (!(status&1))
    {
    log_error (
"Decterm failed to get printer efn, u_open_wake_channel status = %d\n",
	status);
    return status;
    }

status = sys$setef (*result);

if (!(status&1)) log_error (
       "Decterm failed to set printer efn, sys$setef status = %d\n",
		status);

return status;
}

#endif

/*
 * start_printing
 *
 * This routine is called from within the TEA to initiate a printing operation.
 */

void start_printing( stm )
    STREAM *stm;
{
    Arg arglist[3];
    int n;
#ifdef VXT_DECTERM
    VxtPrinterStatus prt_status;
    unsigned int prt_mode;
#endif
    DECwPrintingDestination destination;
    char *port_name, *file_name;

#ifdef VMS_DECTERM
#define printer_file_protection 0  /* on vms, 0 means default protection */
#else
#define printer_file_protection S_IRUSR|S_IWUSR
#endif				   /* on unix, no default so owner read + wrt */

    if ( stm->printer.active )
	return;	/* already active */

#ifdef VXT_DECTERM	/* do VXT checking of printer availability */

/* Some hardware platforms do not have printer support, such as 
   the VT1300.  In those cases, send a warning message. */

    if( VxtSystemType() != VXTDWTII)
	    {
		/* disable all existing printing mode */

		stm->printer.status = DECwNoPrinter;
		stm->printer.active = False;
        	XtSetArg( arglist[0], DECwNprintMode, DECwNormalPrintMode );
                XtSetArg( arglist[1], DECwNprinterStatus, stm->printer.status );
                XtSetValues( stm->terminal, arglist, 2 );

	        vxt_msgbox_write( TM_MSG_WARNING, 1, k_dt_printer_not_supported,
		    0); 

		return;
            }

    /* send printer not present and printer not available status 
		to TM message box */

    VxtPrinterGetStatus (&prt_status, &prt_mode);

    switch( prt_status) {
	        case not_available_not_ready:
	        case not_available_ready:
		    vxt_msgbox_write( TM_MSG_WARNING, 1,
					k_dt_printer_not_available, 0);
		    goto bad_printer;
	        case no_printer:
		    vxt_msgbox_write( TM_MSG_WARNING, 1,
					k_dt_no_printer, 0);
		    goto bad_printer;
		default: goto good_printer;
	        } /* switch() */

    bad_printer:

    stm->printer.status = DECwNoPrinter;
    XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
    XtSetValues( stm->terminal, arglist, 1 );
    return;

    good_printer:

#endif VXT_DECTERM

    n = 0;
    XtSetArg( arglist[n], DECwNprintingDestination, &destination );	n++;
    XtSetArg( arglist[n], DECwNprinterPortName, &port_name );		n++;
    XtSetArg( arglist[n], DECwNprinterFileName, &file_name );		n++;
    XtGetValues( stm->terminal, arglist, n );
    stm->printer.destination = destination;

    switch(destination)
    {
        case DECwDestinationQueue:
#ifdef VMS_DECTERM
            strcpy(stm->printer.file_name, "SYS$SCRATCH:DECW$TERMINAL_PRINT.TMP");
#endif

#if !defined(VMS_DECTERM) && !defined(VXT_DECTERM)
            tmpnam(stm->printer.file_name);
#endif
            stm->printer.status = DECwPrinterReady;
            break;

        case DECwDestinationPort:
            if (port_name == NULL ||
                strlen(port_name) > MAX_PRINT_FILE_NAME_LENGTH)
                stm->printer.status = DECwNoPrinter;
            else
            {
                strcpy(stm->printer.file_name, port_name);
                stm->printer.status = DECwPrinterReady;
            }
            break;

        case DECwDestinationFile:
            if (file_name == NULL ||
                 strlen(file_name) > MAX_PRINT_FILE_NAME_LENGTH)
                stm->printer.status = DECwNoPrinter;
            else
            {
                strcpy(stm->printer.file_name, file_name);
                stm->printer.status = DECwPrinterReady;
            }
            break;

        case DECwDestinationNone:
        default:
            stm->printer.status = DECwNoPrinter;
            break;
    }

#ifdef VXT_DECTERM
    stm->printer.status = DECwPrinterReady;
#endif
                
    if (stm->printer.status == DECwPrinterReady)
    {
#if !defined (VMS_DECTERM) && !defined(VXT_DECTERM)
	/*
	 * Make sure the user has the privileges needed to open the file.
	 */
	if (!directory_writable(stm->printer.file_name))
	    stm->printer.status = DECwNoPrinter;
	else
#endif
        {
	    int status = 1;
#ifdef VXT_DECTERM
	    stm->printer.file = VxtFileOpen( VxtFileClassPrinter, NULL, 
				    (O_RDWR | O_NONBLOCK), 0 );
	    if (stm->printer.file <= 0)
		    {
		    vxt_msgbox_write( TM_MSG_WARNING, 1,
					k_dt_no_printer, 0);
		    warn_window (stm, "printer_open_warning",
			"VxtFileClassPrinter", "VxtFileOpen failed");
		    }
#else
	    stm->printer.file = open( stm->printer.file_name,
		O_WRONLY|O_CREAT|O_TRUNC, printer_file_protection );
	    if (stm->printer.file <= 0)
		warn_window (stm, "printer_open_warning",
		    stm->printer.file_name, sys_errlist[errno]);
#ifdef VMS_DECTERM
	    /*
	     * On unix-style systems, and on vxt, our strategy for knowing
	     * when it's time to print the next line is that we get an
	     * interrupt via our XtAppAddInput when the file is ready for
	     * more data.
	     *
	     * On vms, we use this wr_efn, which we always leave set.  Hence
	     * we get a constant stream of interrupts until we call
	     * XtRemoveInput, which we do right away if there's no more
	     * data yet to print.
	     */
	    status = get_efn (&stm->printer.wr_efn);
#endif
#endif
	    if ( stm->printer.file <= 0 || !(status&1))
		{
		stm->printer.status = DECwNoPrinter;
		}
	    else
		{
		stm->printer.active = True;
		}
        }
    }

    stm->printer.wr_id = NULL;
    stm->printer.stop_reading_input = False;
    stm->printer.prt_buf = NULL;
    stm->printer.depth_of_link_list = 0;

    XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
    XtSetValues( stm->terminal, arglist, 1 );
}

/*
 * finish_printing
 *
 * This routine is called from within the TEA to complete a printing operation.
 */

void finish_printing( stm )
    STREAM *stm;
{
    Arg arglist[2];
    int close_status;
    XmString cs_file_name;	/* file name in compound string form */
    int to_host = 0 ;

    if ( stm->printer.active )
	{

#if !defined (VMS_DECTERM) && !defined(VXT_DECTERM)

/* Change the uid of the file to the real uid if it isn't already set that way.
 * Starting in Ag/BL11, NFS was changed so that if a root NFS operation fails,
 * it is retried with the real uid if that is not root.  This means that files
 * written on an NFS mounted volume may already have the correct ownership.
 */
	struct stat fs;
	uid_t real_uid = getuid();

	stat(stm->printer.file_name, &fs);
	if (fs.st_uid != real_uid)
	    chown(stm->printer.file_name, real_uid, -1);
#endif
	/* Set normal mode, which prints last buffer and turns off auto and
	 * controller modes.  We must do this before clearing printer.active,
	 * lest the operation of printing last buffer implicitly call
 	 * start_printing.
	 */
	XtSetArg( arglist[0], DECwNprintMode, DECwNormalPrintMode );
    	XtSetValues( stm->terminal, arglist, 1 );

	stm->printer.active = False;

	/* Remove write call back */

	if ( stm->printer.wr_id )
	{
	    XtRemoveInput(stm->printer.wr_id);
	    stm->printer.wr_id = NULL;
        }

	/* Free all the buffer link lists */

	free_all_link_lists( stm->printer.prt_buf );
        stm->printer.depth_of_link_list = 0;

	/* set to no printing pending */
	XtSetArg( arglist[0], DECwNprinterPending, 0 );
    	XtSetValues( stm->terminal, arglist, 1 );

	/* close printer port */

	XtSetArg( arglist[0], DECwNprinterToHostEnabled, &to_host );
	XtGetValues( stm->terminal, arglist, 1 );

	/* can only close printer port if not in use by printer to host mode */
	close_status = -1;
	if ( stm->printer.file > 0 && !to_host)
	{
	    close_status = close( stm->printer.file );
#ifdef VMS_DECTERM
	    u_close_wake_channel (stm->printer.wr_efn);
#endif
	    stm->printer.file = -1;
	}

	start_reading_input( stm->terminal, stm );

	if ( close_status != 0 )
	    {
	    stm->printer.status = DECwNoPrinter;
	    XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
	    XtSetValues( stm->terminal, arglist, 1 );
	    }
	else if ( stm->printer.destination == DECwDestinationQueue )
	    {
	    /*
	     * Create the print widget unless it already exists.
	     */
	    if ( stm->setup.queued_printer_options_parent == 0 )
		if ( !create_queued_options_box( stm ))
		    return;
	    /*
	     * Submit the print job.
	     */
	    cs_file_name = XmStringCreate( stm->printer.file_name,
	      XmSTRING_DEFAULT_CHARSET );
	    DXmPrintWgtPrintJob( stm->setup.print_widget_id, &cs_file_name, 1 );
	    XmStringFree( cs_file_name );
	    }
	}
}

/*
 * UIL callbacks.
 */

void print_page_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM 	*stm;
    DECwPrintExtent extent;
    Arg		arglist[1];

    stm = convert_widget_to_stream(w);
    /*
     * Save the current print extent.
     */
    XtSetArg( arglist[0], DECwNprintExtent, &extent );
    XtGetValues( stm->terminal, arglist, 1 );
    /*
     * Temporarily set the extent to DECwFullPage.
     */
    if ( extent != DECwFullPage )
	{
	XtSetArg( arglist[0], DECwNprintExtent, DECwFullPage );
	XtSetValues( stm->terminal, arglist, 1 );
	}
    /*
     * Print the page (not including the transcript).
     */
    DECwTermPrintTextScreen( stm->terminal );
    /*
     * Restore the saved print extent.
     */
    if ( extent != DECwFullPage )
	{
	XtSetArg( arglist[0], DECwNprintExtent, extent );
	XtSetValues( stm->terminal, arglist, 1 );
	}
}

void print_selected_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM 	*stm;
    DECwPrintExtent extent;
    Arg		arglist[1];

    stm = convert_widget_to_stream(w);

    /*
     * Save the current print extent.
     */
    XtSetArg( arglist[0], DECwNprintExtent, &extent );
    XtGetValues( stm->terminal, arglist, 1 );
    /*
     * Temporarily set the extent to DECwSelectionOnly.
     */
    if ( extent != DECwSelectionOnly )
	{
	XtSetArg( arglist[0], DECwNprintExtent, DECwSelectionOnly );
	XtSetValues( stm->terminal, arglist, 1 );
	}
    /*
     * Print the selected text.
     */
    DECwTermPrintTextScreen( stm->terminal );
    /*
     * Restore the saved print extent.
     */
    if ( extent != DECwSelectionOnly )
	{
	XtSetArg( arglist[0], DECwNprintExtent, extent );
	XtSetValues( stm->terminal, arglist, 1 );
	}
}

void print_all_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM 	*stm;
    DECwPrintExtent extent;
    Arg		arglist[1];

    stm = convert_widget_to_stream(w);

    /*
     * Save the current print extent.
     */
    XtSetArg( arglist[0], DECwNprintExtent, &extent );
    XtGetValues( stm->terminal, arglist, 1 );
    /*
     * Temporarily set the extent to DECwFullPagePlusTranscript.
     */
    if ( extent != DECwFullPagePlusTranscript )
	{
	XtSetArg( arglist[0], DECwNprintExtent, DECwFullPagePlusTranscript );
	XtSetValues( stm->terminal, arglist, 1 );
	}
    /*
     * Print the page (including the transcript).
     */
    DECwTermPrintTextScreen( stm->terminal );
    /*
     * Restore the saved print extent.
     */
    if ( extent != DECwFullPagePlusTranscript )
	{
	XtSetArg( arglist[0], DECwNprintExtent, extent );
	XtSetValues( stm->terminal, arglist, 1 );
	}
}

void print_graphics_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM 	*stm = convert_widget_to_stream(w);

#ifdef VXT_DECTERM
/*
 * We use a timeout so that there isn't the print menu still on the screen,
 * which would cause a hole in the picture.  We don't use the vms decterm
 * method of using a pixmap, because we are concerned about memory restrictions
 * on the physical vxt.
 */
static void delayed_print_graphics ();
if (! stm->printer.graphics_delay_id)
    stm->printer.graphics_delay_id = XtAppAddTimeOut (
	XtWidgetToApplicationContext (w), 3000, delayed_print_graphics, stm);
}

/*
 * Come here after the delay, to actually commence printing.
 */
static void delayed_print_graphics (stm) STREAM *stm;
{
stm->printer.graphics_delay_id = NULL;
#endif VXT_DECTERM
DECwTermPrintGraphicsScreen( stm->terminal );
}

void finish_printing_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    STREAM 	*stm;

    stm = convert_widget_to_stream(w);
    finish_printing( stm );
}

static void free_all_link_lists( root )
struct prt_buf_struct *root;
{
struct prt_buf_struct *current_buf, *next_buf;

    current_buf = root;

    while ( current_buf != NULL )
    {
        next_buf = current_buf->next;

	/* free the current buffer */

	if ( current_buf->data_buf != NULL )
	    XtFree(current_buf->data_buf);
	XtFree((char *)current_buf);
	current_buf  = next_buf;
    }
}

void cancel_printing_cb(w, closure, call_data)
Widget w;
caddr_t closure;
XmAnyCallbackStruct *call_data;
{
    Arg arglist[2];
    int close_status;
    STREAM 	*stm;
    int to_host = 0;

    stm = convert_widget_to_stream(w);

    if ( stm->printer.active )
	{
	/* Set normal mode, which prints last buffer and turns off auto and
	 * controller modes.  We must do this before clearing printer.active,
	 * lest the operation of printing last buffer implicitly call
 	 * start_printing.
	 */
	XtSetArg( arglist[0], DECwNprintMode, DECwNormalPrintMode );
    	XtSetValues( stm->terminal, arglist, 1 );

	stm->printer.active = False;

	/* Remove write call back */

	if ( stm->printer.wr_id )
	{
	    XtRemoveInput(stm->printer.wr_id);
	    stm->printer.wr_id = NULL;
        }

	/* Free all the buffer link lists */

	free_all_link_lists( stm->printer.prt_buf );
        stm->printer.depth_of_link_list = 0;

	/* set to no printing pending */

	XtSetArg( arglist[0], DECwNprinterPending, 0 );
    	XtSetValues( stm->terminal, arglist, 1 );

	/* clear printer device driver buffer */

#ifdef VXT_DECTERM
	VxtFileCancel(stm->printer.file);
#endif

	/* close printer port */

	XtSetArg( arglist[0], DECwNprinterToHostEnabled, &to_host );
	XtGetValues( stm->terminal, arglist, 1 );

	/* can only close printer port if not in use by printer to host mode */
	close_status = -1;
	if ( stm->printer.file > 0 && !to_host)
	{
	    close_status = close( stm->printer.file );
#ifdef VMS_DECTERM
	    u_close_wake_channel (stm->printer.wr_efn);
#endif
	    stm->printer.file = -1;
	}

	start_reading_input( w, stm );

	if ( close_status != 0 )
	    {
	    stm->printer.status = DECwNoPrinter;
	    XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
	    XtSetValues( stm->terminal, arglist, 1 );
	    }
	}
}


/* Routine to get printer configuration.
   returns : READ_ALLOWED = 1 = printer port is serial
	     0		      = printer port is parallel
 */

#ifdef VXT_DECTERM

#include "vxtprinter.h"
int get_printer_config()
{
    VxtPrinterConfig config;

    VxtPrinterGetConfig( &config );
    if ( config == serial_printer)
	return READ_ALLOWED;  /* serial port */
    else
	return 0;	/* parallel port */
}

#endif

/*
 * enable_printer_read_input tells toolkit to interrupt us often so we
 * can read from printer port.
 */
enable_printer_read_input (stm) STREAM *stm;
{
stm->printer.rd_id = XtAppAddInput( TEA_app_context,
#if defined(VMS_DECTERM)
    stm->printer.rd_efn,
    0,
#else
    stm->printer.in_file,
    (XtPointer) XtInputReadMask,
#endif
    non_block_read,
    stm );
}

/*
	This is the design for implementing the printer to host mode. 

1. A user can enable the printer to host mode either from "printer" options 
   dialog box or from an escape sequence.  Default state is to disable the 
   printer to host mode.
   
2. After the printer to host mode is enabled, first check to see if printer
   configuration allows reading from the printer port.  This part is platform 
   dependent.  For example, in the VXT, either a serial port or a parallel
   port can be configured as a printer port.  A serial port is duplex, but
   the parallel port used in the VXT is a write only port.  Therefore, cannot
   allow printer to host port mode for a parallel port.  

   In the case which reading from the printer port is not allowed, "printer to
   host" button in the printer options would be grayed out.  The escape
   sequence to enable printer to host mode would do nothing.

3. DECterm application opens the printer port for read in a non-blocking 
   mode (if it hasn't been opened for write already).  Read and write from/to
   printer port shares the same file descriptor.  Open is platform dependent. 
   It is counting on open to return failure if the printer port is already in 
   used by another DECterm (for read or write).  Only one DECterm can be in 
   the printer to host mode at one time.  It doesn't seem to be useful to have 
   multiple DECterms reading data from the same printer port at the same time 
   because each DECterm would only get a piece of the data stream from the 
   printer.  It also does not make sense for one DECterm window to be in 
   printer to host mode (read only), and another DECterm to be doing print 
   screen (write only). 

   In the case when open fails, printer to host button in the printer options
   would be disabled, and the escape sequence to enable printer to host mode
   would do nothing.

4. DECterm uses XtAddInput for reading from the printer port.

5. DECterm application reads a chunck of data from the printer (in a 
   non-blocking mode), and puts it into the DECterm widget's input queue 
   (shares the same queue as reports to the host).  It first checks to see if 
   the queue is full before doing the read.  If the queue is full, it does a 
   XtRemoveInput of the file descriptor, so that no more data can be read from
   the printer.  

6. When the DECterm widget can accept more inputs, it calls the DECterm
   application to do XtAddInput again so that the DECterm application can 
   continue to read from the printer port.

7. When a user disables the printer to host button in the printer options or
   a disable printer to host escape sequence is sent, the DECterm application
   would do a XtRemoveInput and close the printer port (only if the port is 
   not in use for writing).

note: "Clear Comm" and "Cancel Printing" do NOT disable printer to host mode.

*/

/* This routine reads data from printer after printer to host mode is enabled.
   Read is done in a non-blocking mode.  This routine returns 0 if the printer
   port cannot be read.  It returns 1 if the printer port can be read.  
   Possible Conditions for return status of 0:

   1. If the printer port is physically not duplex (such as a parallel printer
      port on a VXT2000), then reading from the printer port is not allowed. 

   2. If other DECterms already enabled printer to host mode, open would fail.
      Thus this DECterm cannot read from printer port.  Only one DECterm
      can be in printer to host mode at a time.

   3. malloc fails, so cannot allocate any memory for the buffer.
*/

void read_data_from_prt( w, stm, ok_flag )
Widget w;
STREAM *stm;
Boolean *ok_flag;
{
    Arg arglist[3];
    int n;
    int status;
    DECwPrintingDestination destination;
    char *port_name, *file_name;

    *ok_flag = False;

    if (stm->printer.in_file > 0)   /* if already on, it's ok, so return */
	{
	*ok_flag = True;
	return;
	}

#ifdef VXT_DECTERM

    /* Some hardware platforms do not have printer support, such as 
       the VT1300.  In those cases, send a warning message. */

    if( VxtSystemType() != VXTDWTII)
    {
    	stm->printer.status = DECwNoPrinter;
    	stm->printer.active = False;
        XtSetArg( arglist[0], DECwNprinterStatus, stm->printer.status );
        XtSetValues( stm->terminal, arglist, 1 );

	vxt_msgbox_write( TM_MSG_WARNING, 1, k_dt_printer_not_supported, 0); 

	return;
    }

    if ( get_printer_config() != READ_ALLOWED )
    {
	vxt_msgbox_write( TM_MSG_WARNING, 1, k_dt_printer_wrong_config, 0);
	return;
    }

#endif

	/* need to check if printer is already opened for writing. */
	if ( stm->printer.file > 0)
	return;
/*
 * Now we ask the decterm widget whether we're supposed to read from the
 * port or the file.  (It's called "destination" because it's originally
 * used for writing to instead of reading from.)
 */
    n = 0;
    XtSetArg( arglist[n], DECwNprintingDestination, &destination );	n++;
    XtSetArg( arglist[n], DECwNprinterPortName, &port_name );		n++;
    XtSetArg( arglist[n], DECwNprinterFileName, &file_name );		n++;
    XtGetValues( stm->terminal, arglist, n );
/*
 * Now decide which name to use.  For now, if user selected "none" or a
 * queue, we don't attempt to read anything.
 */
    switch(destination)
    {
        case DECwDestinationPort:
            if (port_name == NULL ||
                strlen(port_name) > MAX_PRINT_FILE_NAME_LENGTH)
                return;
            else
            {
                strcpy(stm->printer.file_name, port_name);
            }
            break;

        case DECwDestinationFile:
            if (file_name == NULL ||
                 strlen(file_name) > MAX_PRINT_FILE_NAME_LENGTH)
 		return;
            else
            {
                strcpy(stm->printer.file_name, file_name);
            }
            break;

	case DECwDestinationQueue:
        case DECwDestinationNone:
        default:
	    return;
    }
#ifdef VXT_DECTERM
        stm->printer.in_file = VxtFileOpen( VxtFileClassPrinter, NULL, 
		                       ( O_RDWR | O_NONBLOCK), 0 );
#else
	stm->printer.in_file = open( stm->printer.file_name,
		O_RDONLY       /* | O_NDELAY */       , 0 );
/*
 * We really want "O_NDELAY" so that we can truly have asynchronous input,
 * but my observation on vms at least is that O_NDELAY causes the read to
 * fail with the "I/O stream is empty" error from the c library.
 */
#endif

#if defined (VMS_DECTERM)
	status = get_efn (&stm->printer.rd_efn);
#endif

	if ( stm->printer.in_file <= 0 )
	{
#ifdef VXT_DECTERM
	    vxt_msgbox_write( TM_MSG_WARNING, 1, k_dt_printer_open_read_failed,
				0);
	    warn_window (stm, "printer_open_warning",
		"VxtFileClassPrinter", "VxtFileOpen failed");
#else
	    warn_window (stm, "printer_open_warning",
		    stm->printer.file_name, sys_errlist[errno]);
#endif
	    stm->printer.rd_id = NULL;
	    return;
	}
	else
	{
	    if ( !stm->printer.rd_id ) enable_printer_read_input (stm);
	}

    *ok_flag = True;
    return;
}

/* Routine to read from the printer in a non-blocking mode. and send it to
   the host. */

static void non_block_read( stm )
STREAM *stm;
{
    int bytes_read = 0;
    char read_buf[CHAR_BUF_SIZE];
    Widget w = stm->terminal;
    extern Boolean chars_from_widget ();

    bytes_read = read( stm->printer.in_file, read_buf,
				CHAR_BUF_SIZE);

    if ( bytes_read < 0 )
    {
#if defined (VXT_DECTERM)
        log_error(
"read from printer failed, status=%d, exiting printer to host mode\n",
	    bytes_read);
#else
        log_error(
"read from printer failed.\nReason: %s\nexiting printer to host mode\n",
	    sys_errlist[errno]);
        stop_read_data_from_prt(w);
#endif
        return;
    }

    if (bytes_read == 0 )
    {
        log_message(
"end of file on read from printer, exiting printer to host mode\n",
	    bytes_read);
        stop_read_data_from_prt(w);
        return;
    }
/*
 * For now, assume that linefeeds ("\n" in C language) at the end of the
 * data represent end-of-records in a disk file, and hence send a carriage
 * return ("\r") instead.
 */
    if (read_buf[bytes_read-1] == '\n') read_buf[bytes_read-1] = '\r';

    {
    DECwTermInputCallbackStruct call_data;
    call_data.count = bytes_read;
    call_data.data = read_buf;
    if (!chars_from_widget (w, stm, &call_data))
	pause_printer_to_host (stm);
    }
}


/* routine to stop reading data from printer mode and to disable printer to 
   host mode */

void stop_read_data_from_prt(w)
Widget w;
{
    Arg arglist[1];
    STREAM 	*stm;
    int to_host = 0;

    stm = convert_widget_to_stream(w);
/*
 * Turn off the printer to host light.
 */
	XtSetArg( arglist[0], DECwNprinterToHostEnabled, 0 );
	XtSetValues( stm->terminal, arglist, 1 );

	/* Remove read call back */

	if ( stm->printer.rd_id )
	{
	    XtRemoveInput(stm->printer.rd_id);
	    stm->printer.rd_id = NULL;
        }

	/* close printer port read channel if open */

 	if ( stm->printer.in_file > 0)
	{
	    close( stm->printer.in_file );
	    stm->printer.in_file = -1;
#ifdef VMS_DECTERM
	    u_close_wake_channel (stm->printer.rd_efn);
#endif
	}
}

/*
 * pause_printer_to_host is called whenever decterm is stopping input.
 * This routine looks to see if we're in the middle of printer to host
 * mode, and if so, it tells toolkit to stop giving us interrupts so we
 * won't keep stuffing data into the input path with chars_from_widget, until
 * we later get the expected resume_printer_to_host call.
 */
pause_printer_to_host (stm) STREAM *stm;
{
if (stm->printer.in_file && stm->printer.rd_id)
    {
    XtRemoveInput (stm->printer.rd_id);
    stm->printer.rd_id = NULL;
    }
}

/*
 * resume_printer_to_host is called when decterm input is turned back on.
 */
resume_printer_to_host (stm) STREAM *stm;
{
if (stm->printer.in_file && !stm->printer.rd_id)
    enable_printer_read_input (stm);
}

