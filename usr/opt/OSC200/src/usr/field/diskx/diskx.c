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

/*	@(#)$RCSfile: diskx.c,v $ $Revision: 4.2.3.12 $ (DEC) $Date: 1993/01/14 15:29:45 $ */

/*
 * Compilation Instructions: cc -o diskx diskx.c -lbinlog
 *
 * To compile on a V3.1 system: cc -o diskx diskx.c -lerrlog -DCOMP31
 *
 ************************************************************************/
/*
 * diskx - ULTRIX disk exercising utility.
 *
 * Author: Tim Burke
 * Date:   21-Dec-1989
 *
 * This program is used to test disk device functionality.  Test coverage
 * is more comprehensive than the older disk exerciser called dskx(8).
 * The advantages of diskx over dskx include:
 *	- use of n-buffered I/O (not in TIN).
 *	- tests the full range of a partition.
 *	- transfers random or sequentially.
 *	- transfers in a range of sizes.
 *	- allows test parameters and attributes to be easily changed through
 *	  both the command line and interactively.
 *	- disklabel testing.
 *      - performance testing.
 *
 * For a more complete description of the usage of this program see the
 * help messages which are displayed to the "-h" option.
 *
 * TODO:
 *
 *	- Allow preliminary output to be displayed at given time intervals.
 *	  This way the user gets an idea of how things are progressing for
 *	  tests that take a long time to complete.
 *
 *-----------------------------------------------------------------------
 *
 * Modification History
 *
 * 20-Nov-91	Tom Tierney
 *	Modified to always write enable disk label if we are writing on
 *	a partition which incorporates the label and the user gives the 
 *	OK to trash the label.
 * 
 * May 1991	Matthew Sacks
 *		Ported this utility to run on the OSF Tin release.
 *
 *	I)  	This requires using OSF/1 disk labels.
 *		Ultrix uses the pt (partition table) structure, defined 
 *		in fs.h, and an associated structure, disktab, defined
 *		in disktab.h  OSF uses, instead, the disklabel structure
 *		defined in disklabel.h.  The file system group intends
 *		to support fully disklabels in tin/silver and support
 *		the ultrix partition table structure readonly.  I have
 *		changed the utilities so that they use disklabel ioctls.
 *		So, disk exerciser functionality will work only on disks
 *		that have disklabels written onto them. 
 *
 *		References through the pt structure are replaced with
 *		references through the disklabel structure.  For example,
 *
 *		pt.pt_part[partition_number].pi_nblocks
 *
 *		is replaced with
 *
 *		(&disklabel).d_partitions[partition_number].p_size
 *
 *		To simplify this, I used a macro "Pt_tab."  pi_offset, in
 *		the pt struct, is replaced with p_offset in the disklabel
 *		struct.
 *
 * 		Wherever a utility used to acquire and use a disktab structure,
 *		it now uses disklabel fields.  There is not a perfect
 *		correspondence between the two structures.  This is how I
 *		mapped them
 *
 *		disktab field		disklabel field
 *		-------------		---------------
 *		d_name			d_packname
 *		d_type			d_typename
 *		p_bsize			<does not have equivalent>
 *		d_badsectforw		<does not have equivalent>
 *		d_type			d_typename
 *		bsize			<does not have equivalent>
 *
 *
 *
 *		II)  N-buff I/O is not supported by tin.  So,
 *		the -n option which disables n-buff I/O is always
 *		turned on.  The nbuf_read and nbuf_write routines
 *		are conditionally compiled out, and replaced with
 *		dummies, by the conditional TIN_NO_NBUF.
 *
 *		III) Tin does not have the creatediskbyname system
 *		call so the -d test, which compares the results of
 *		getdiskbyname() and creatediskname(), is not supported.
 *		This is easily accomplished by defining the existing
 *		COMP31 conditional.
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#if     BSD > 43
#include <ufs/fs.h>
#else
#include <sys/fs.h>
#endif
#include <sys/disklabel.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <io/common/devio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <dec/binlog/binlog.h>


/*-------------------------------------------------------------------------
 * Constant Definitions
 */
#define STATE_INITIALIZE 0			/* Initializes state      */
#define ERRORS_INITIALIZE { 0, 0, 0, '\0' }	/* Init error status      */
/*
 * Partition Masks
 */
#define PART_A		0x1			/* Test the "a" partition */
#define PART_B		0x2			/* Test the "b" partition */
#define PART_C		0x4			/* Test the "c" partition */
#define PART_D		0x8			/* Test the "d" partition */
#define PART_E		0x10			/* Test the "e" partition */
#define PART_F		0x20			/* Test the "f" partition */
#define PART_G		0x40			/* Test the "g" partition */
#define PART_H		0x80			/* Test the "h" partition */
#define PART_ALL       (PART_A|PART_B|PART_C|PART_D|PART_E|PART_F|PART_G|PART_H)
#define PART_BAD	0x100			/* Invalid partition mask */
#define MAX_PART	8			/* Maximum partitions     */
/*
 * Test States
 */
#define RUN_TEST	0x1			/* Run test state	  */
#define TEST_IP		0x2			/* Test is in progress	  */
/*
 * Output status for printf's
 */
#define LOGFILE		0x1			/* Send output to a logfile */
#define TTYPRINT	0x2			/* Send output to standard out*/
#define TTY_WIDTH	80			/* Assume 80 column display   */
#define MAX_OUTPUT	10			/* Max error messages per test*/
#define MAX_ERR_LINES	10000			/* Max error messages allowed */
#define MIN_ERR_LINES	1			/* Min error messages allowed */
#define OUT_STRING	"\nMaximum error output exceeded, terminate printing on this subtest.\n"
#define MIN_DEBUG	0			/* Lowest debug output level  */
#define MAX_DEBUG	10000			/* Highest debug output level */

#define SUCCESS		0			/* Success return status      */
#define FAILURE		1			/* Failure return status      */

#define NO_WRITE	0			/* No writes - read only      */
#define DO_WRITE	1			/* Writes will be done        */
#define NO_VALIDATE	0			/* No validation on reads     */
#define DO_VALIDATE	1			/* Do validation on reads     */
#define NO_SEEK0	0			/* No seek to 0 before xfers  */
#define DO_SEEK0	1			/* Seek to 0 before xfers     */

#define MAX_DISKNAME	40			/* Max length of disk name    */
#define MAX_TIMER	10080			/* Max time of 7 days in mins */
/*
 * Options to the devio_print routine.
 */
#define DEVIO_NAME	1			/* Print devname	      */
#define DEVIO_OFFLINE	2			/* Device offline	      */
#define DEVIO_WRTLCK	3			/* Device write protected     */
/*
 * Transfer size related parameters.
 */
#define MAXPHYS		(64 * 1024)		/* Copied from minphys routine*/
#define MIN_XFER	DEV_BSIZE		/* Smallest read or write     */
#define MAX_XFER	MAXPHYS			/* Largest read or write      */
#define MIN_NUM_XFER	1			/* Minimum number of xfers    */
#define MAX_NUM_XFER	100000			/* Maximum number of xfers    */
#define DEF_NUM_XFER	0			/* Default number of xfers    */
#define TIME_CHECK	500			/* Check timer per # xters    */
/*
 * N-buffered I/O parameters
 */
#define DEF_NUM_NBUF	7			/* Default # of bufs	      */
#define MIN_NUM_NBUF	1			/* Minimum number of bufs     */
#define MAX_NUM_NBUF	100			/* Maximum number of bufs     */
#define NO_ALLOC	0			/* Don't allocate buffer space*/
#define ALLOC_BUFFERS   1			/* Allocate buffer space      */
#define MAX_NBUF_ABORTS 20			/* N-buf error max	      */
#define XFER_IP		1			/* N-buf xfer in progress     */
/*
 * Defines related to the performance tests.
 */
#define PERF_SPLITS	10			/* Divides xfer range, default*/
#define MIN_PERF_SPLITS 1			/* Minimum num of calculations*/
#define MAX_PERF_SPLITS 50			/* Maximum num of calculations*/
/*
 * Strings which are sent to the error logger to represent test status.
 */
#define LOG_START		"Diskx - started testing"
#define LOG_STOP		"Diskx - completed testing"
#define LOG_ABORT		"Diskx - aborting testing"
#define LOG_TIMEUP		"Diskx - stop due to time expiration"
#define LOG_SIGNAL		"Diskx - stop due to signal receipt"
#define LOG_OFFLINE		"Diskx - aborting, device offline"
#define LOG_COMMAND		"Diskx - command line parameter error"
#define LOG_PARAM		"Diskx - parameter range error"
#define LOG_NBUF		"Diskx - repeated n-buf errors"
#define LOG_CACHE		"Diskx - cache flush errors"
#define LOG_SEEK		"Diskx - aborting due to seek errors"
#define LOG_READERR		"Diskx - aborting due to read errors"
#define LOG_TERM		"Diskx - terminating at operator request"
#define LOG_OPENFAIL		"Diskx - partition open failure"
#define LOG_IOCTLFAIL		"Diskx - partition ioctl failure"
#define LOG_MALLOC		"Diskx - aborting due to malloc failure"

#define TIN_NO_NBUF

/*---------------------------------------------------------------------------
 * Macros
 */
/*
 * Macros to facilitate logging to an output file.
 *
 * Print - printf with no args
 * Print1 - printf with 1 arg
 * Print2 - printf with 2 args
 * Print3 - printf with 3 args
 *
 * Error printfs; these are used with error messages to limit runaway output.
 * EPrint - printf with no args
 * EPrint1 - printf with 1 arg
 * EPrint2 - printf with 2 args
 * EPrint3 - printf with 3 args
 *
 * Log_print - used to echo interactive parameters to logfile.
 */
#define Log_print( string )						\
	{								\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string);		\
				fprintf(fd_logfile, "\n");		\
			}						\
	}
			
#define Print( string ) 						\
	{								\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string);		\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string);		\
			}						\
	}
			
#define Print1( string, arg1 ) 						\
	{								\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string, arg1);	\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string, arg1);		\
			}						\
	}
#define Print2( string, arg1, arg2 )					\
	{								\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string, arg1, arg2);\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string, arg1, arg2);	\
			}						\
	}
#define Print3( string, arg1, arg2, arg3 )				\
	{								\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string, arg1, arg2, arg3);\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string, arg1, arg2, arg3);\
			}						\
	}
#define EPrint( string ) 						\
	{								\
		if (lines_output <= max_err_lines) {			\
			if (lines_output == max_err_lines) {		\
				if (printstatus & LOGFILE) {            \
					fprintf(fd_logfile, OUT_STRING);\
				}					\
				if (printstatus & LOGFILE) {            \
					fprintf(stdout, OUT_STRING); 	\
				}					\
				flush_output();				\
			}						\
			lines_output++;					\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string);		\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string);		\
			}						\
		}							\
	}
			
#define EPrint1( string, arg1 ) 						\
	{								\
		if (lines_output <= max_err_lines) {			\
			if (lines_output == max_err_lines) {		\
				if (printstatus & LOGFILE) {            \
					fprintf(fd_logfile, OUT_STRING);\
				}					\
				if (printstatus & LOGFILE) {            \
					fprintf(stdout, OUT_STRING); 	\
				}					\
				flush_output();				\
			}						\
			lines_output++;					\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string, arg1);	\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string, arg1);		\
			}						\
		}							\
	}
#define EPrint2( string, arg1, arg2 )					\
	{								\
		if (lines_output <= max_err_lines) {			\
			if (lines_output == max_err_lines) {		\
				if (printstatus & LOGFILE) {            \
					fprintf(fd_logfile, OUT_STRING);\
				}					\
				if (printstatus & LOGFILE) {            \
					fprintf(stdout, OUT_STRING); 	\
				}					\
				flush_output();				\
			}						\
			lines_output++;					\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string, arg1, arg2);\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string, arg1, arg2);	\
			}						\
		}							\
	}
#define EPrint3( string, arg1, arg2, arg3 )				\
	{								\
		if (lines_output <= max_err_lines) {			\
			if (lines_output == max_err_lines) {		\
				if (printstatus & LOGFILE) {            \
					fprintf(fd_logfile, OUT_STRING);\
				}					\
				if (printstatus & LOGFILE) {            \
					fprintf(stdout, OUT_STRING); 	\
				}					\
				flush_output();				\
			}						\
			lines_output++;					\
			if (printstatus & LOGFILE) {			\
				fprintf(fd_logfile, string, arg1, arg2, arg3);\
			}						\
			if (printstatus & TTYPRINT) {			\
				fprintf(stdout, string, arg1, arg2, arg3);\
			}						\
		}							\
	}
			
/*-------------------------------------------------------------------------
 * Typedefs - data structure templates
 */

typedef struct _errors {		/* Test Error Summary		*/
	int	read_errors;		/* Read Error Status		*/
	int	write_errors;		/* Write Error Status		*/
	int	seek_errors;		/* Seek Error Status		*/
	char	*error_string;		/* Error String			*/
} ERRORS;

typedef struct _test {			/* Test Definition		*/
	char 	*option;		/* Command line specifier	*/
	char	*comment;		/* Test Comment string		*/
	u_int	state;			/* Test State			*/
	int	(*test_routine)();	/* Test Routine Name		*/
	ERRORS 	errors;			/* Test Error Summary		*/
	int	(*test_signal)();	/* Test Signal Handler		*/
} TEST;

typedef struct _attributes {		/* Test Attributes		*/
	char	*option;		/* Command line specifier       */
	char	*comment;		/* Attribute Description	*/
	u_int	attr_flag;		/* Attribute flag to enable	*/
} ATTRIBUTE;

typedef struct _results {		/* Results array for xfer tests */
	int	reads;			/* Number of reads		*/
	int	read_errs;		/* Number of read errors	*/
	double	read_bytes;		/* Number of bytes read		*/
	int	writes;			/* Number of writes		*/
	int	write_errs;		/* Number of write errors	*/
	double	write_bytes;		/* Number of bytes written	*/
	int	seeks;			/* Number of seeks		*/
	int	seek_errs;		/* Number of seek errors	*/
	int	val_errs;		/* Data validation errors	*/
} RESULTS;

typedef struct _perfstruct {		/* Results of performance tests */
	char	partition;		/* Partition name		*/
	int	xfer_size;		/* Transfer size		*/
	int	num_xfers;		/* Number of transfers		*/
	int	read_rate;		/* Observed read throughput	*/
	int	write_rate;		/* Observed write throughput	*/
	int	errors;			/* Errors encountered		*/
} PERFSTRUCT;
	
typedef struct _nbuf_struct {		/* Struct for n-buf I/O     	*/
	char *buffer;			/* Data transfer buffer pointer */
} NBUF_STRUCT;


/*-------------------------------------------------------------------------
 * Routine Names
 */
int readtest();
int writetest();
int seektest();
int performance_test();
int disklabel_test();
int badtest();
int signal_diskx();
int print_xfers();
int print_perf();
int no_sig_handler();
int help_message();

/*-------------------------------------------------------------------------
 * Test Attributes
 */
#define BAD_ATTRIBUTE	0x0		/* Unused attribute		*/
#define NO_NBUF		0x1		/* Do not use n-buf io		*/
#define INIT_BUF	0x2		/* Initialize buffer to zero	*/
#define OPEN_READONLY	0x4		/* Open the file read-only	*/
#define SEQUENTIAL	0x8		/* Read and write sequentially  */
#define FIXED_SIZE	0x10		/* Use a single transfer size   */
#define PRINT_HELP	0x20		/* Print a help message	        */
#define INTERACTIVE	0x40		/* Set params interactively     */
#define NO_READ_PERF	0x80		/* Skip read performance tests  */
#define TTY_AND_LOG	0x100		/* Print to both tty and logfile*/
#define NO_CONFIRM	0x200		/* Don't ask for write confirm  */

/*
 * Table of tests this exerciser performs.
 */
TEST testmatrix[] = {
    {
	"-d",
	"disklabel verification",
	STATE_INITIALIZE,
	disklabel_test,
	ERRORS_INITIALIZE,
	no_sig_handler,
    },
    {
	"-p",
	"performance measurement",
	STATE_INITIALIZE,
	performance_test,
	ERRORS_INITIALIZE,
	print_perf,
    },
    {
	"-r",
	"read test",
	STATE_INITIALIZE,
	readtest,
	ERRORS_INITIALIZE,
	print_xfers,
    },
    {
	"-w",
	"write and read validation",
	STATE_INITIALIZE,
	writetest,
	ERRORS_INITIALIZE,
	print_xfers,
    },
    {
	'\0',
	"Terminating element of the test matrix",
	STATE_INITIALIZE,
	badtest,
	ERRORS_INITIALIZE,
	no_sig_handler,
    }
};

/*
 * Table of test attributes that can be modified by command line options.
 */
ATTRIBUTE attribute_table[] = {
    {
	"-n",
	"Disable usage of n-buffered io",
	NO_NBUF,
    },
    {
	"-h",
	"Display help message",
	PRINT_HELP,
    },
    {
	"-i",
	"Set test parameters interactively",
	INTERACTIVE,
    },
    {
	"-Q",
	"Skip read phase of performance test, only do the write tests",
	NO_READ_PERF,
    },
    {
	"-R",
	"Open the disk read-only",
	OPEN_READONLY,
    },
    {
	"-S",
	"Test sequentially - not in random seek order",
	SEQUENTIAL,
    },
    {
	"-T",
	"Direct output to both a log file and the terminal",
	TTY_AND_LOG,
    },
    {
	"-F",
	"Perform fixed size transfers",
	FIXED_SIZE,
    },
    {
	"-Y",
	"Do not ask for permission to destroy file systems",
	NO_CONFIRM,
    },
    {
	"-z",
	"Initialize buffers to zero",
	INIT_BUF,
    },
    {
	'\0',
	"Terminating element of the attributes array ",
	BAD_ATTRIBUTE,
    },
};

/*---------------------------------------------------------------------------
 * Global Variables
 */
u_int attributes = 0;			/* Initialize test attributes null */
char *disk_cmdline = '\0';		/* Device special file name	   */
char diskname[MAX_DISKNAME];		/* Full device special file name   */
char disk_base[MAX_DISKNAME];		/* Device file name no partition   */
char raw_diskname[MAX_DISKNAME];	/* Raw device special file name    */
char raw_base[MAX_DISKNAME];		/* Raw device name no partition    */
int debug = 0;				/* Debug output devel for printf   */
int test_pnumber;			/* Number of partition to test	   */
u_int test_partitions;			/* Mask of which partitions to test*/
u_int overlap_partitions;		/* Mask of which partitions overlap*/
u_int fs_partitions;			/* Mask of partitions with fs      */
FILE *fd_logfile;               	/* File to log to - if requested.  */
int fd_disk = 0;			/* File descriptor of disk file    */
int is_block_device = 0;		/* Set true when block device      */
struct disklabel disklabel;
struct disklabel *lp = &disklabel;
#define Pt_tab(partition_num) (lp->d_partitions[partition_num])
/* struct pt pt;			Disks partition table in Ultrix	   */
int printstatus = TTYPRINT;     	/* Where to log output to 	   */
int lines_output = 0;           	/* Number of error messages generated */
int max_err_lines = MAX_OUTPUT;		/* Default value for max error output */
int min_xfer = MIN_XFER;		/* Smallest transfer size	   */
int max_xfer = MAX_XFER;		/* Largest transfer size	   */
int xfer_size = 0;			/* Transfer size presently in use  */
int  num_xfer = DEF_NUM_XFER;		/* Default num of xfers / iteration*/
int  num_nbuf = DEF_NUM_NBUF;		/* Default number of bufs for n-buf*/
int  nbuf_aborts = 0;			/* N-buf error counter		   */
NBUF_STRUCT *nbuf_area;			/* Pointer to n-buf structs	   */
char *xfer_buf;				/* Transfer data buffer		   */
int test_minutes = 0;			/* Number of minutes to test       */
int stop_time = 0;			/* Number of seconds to test	   */
RESULTS xfer_results[MAX_PART];		/* Results of xfer tests	   */
PERFSTRUCT perf_results[MAX_PART * MAX_PERF_SPLITS];	/* Performance     */
int perf_number = 0;			/* How many performance tests run  */
int perf_splits = PERF_SPLITS;		/* Default # of performance tests  */
int perf_min_xfer = MIN_XFER;		/* Smallest xfer size perf tests   */
int perf_max_xfer = MAX_XFER;		/* Largest xfer size perf tests	   */
int perf_xfers = DEF_NUM_XFER;		/* Default # of perf transfers	   */
int seed = 0;				/* Data validation seed		   */
char logstr[256];			/* String to pass to the error log */

/*
 * Program Routines:
 */

main(argc, argv)
        int argc;
        char *argv[];

{
    TEST	*present_test;

    /*
     * Parse the command line.
     */
    parse(argc, argv);
    if (attributes & TTY_AND_LOG) {
	printstatus |= TTYPRINT;
    }

    /*
     * Make sure that some tests have been specified.
     */
    for (present_test = testmatrix; present_test->option; present_test++) {
        if (present_test->state)
           break;
    }
    if (present_test->option == '\0' & !(attributes & PRINT_HELP)) {
       Print(" No tests have been selected. Type \"diskx -h\" for help. \n");
       exit(1);
    }

#ifdef TIN_NO_NBUF
    /* kluge - n-buf I/O is not supported for tin, so always set
	NO_NUBUF
     */
    attributes |= NO_NBUF;
#endif /* TIN_NO_NBUF */

    /*
     * Print out a header.
     */
#ifdef OSF
    Print("\nDISKX - DEC OSF/1 Disk Exerciser.\n");
#else  /* !OSF */
    Print("\nDISKX - ULTRIX Disk Exerciser.\n");
#endif  /* OSF */
    Print("\n");
    /*
     * Print out a help message and exit if requested.
     */
    if (attributes & PRINT_HELP) {
	help_message();
	exit(0);
    }
    Print1("Testing disk device %s.\n",diskname);
    /*
     * Prompt for test parameters interactively if requested.
     */
    if (attributes & INTERACTIVE) {
	get_test_params();
    }
    /*
     * Initialize the xfer_results array to zero.
     */
    init_results();
    nbuf_area = (NBUF_STRUCT *) 0;
    /*
     * Establish a signal handler.
     */
    signal(SIGTERM, (caddr_t) signal_diskx);
    signal(SIGINT, (caddr_t) signal_diskx);
    /*
     * If this program is not run as root user then there will be no start /
     * stop messages sent to the error logger.  
     */
    if (geteuid()) {
	    Print("\nNOTE: No messages will be sent to the error logger\n");
	    Print("because this program is not being run as user root.\n\n");
    }
    else {
	    do_errlog(LOG_START);
    }
    /*
     * Allocate space for transfer buffers.
     */
    alloc_buffers();
    /*
     * Start timer so that ending time interval is established.
     */
    start_timer();

    if (debug) {
	Print1("Program output level is %d.\n",debug)
    }

    for (present_test = testmatrix; present_test->option; present_test++) {
	if (present_test->state & RUN_TEST) {
	    print_time();
	    flush_output();
	    print_line();
	    sprintf(logstr, "Diskx - starting test: ");
	    strcat(logstr, present_test->comment);
	    do_errlog(logstr);
	    present_test->state |= TEST_IP;
	    present_test->test_routine();
	    present_test->state &= ~TEST_IP;
	    flush_output();
	    sprintf(logstr, "Diskx - completed test: ");
	    strcat(logstr, present_test->comment);
	    do_errlog(logstr);
	}
    }
    print_line();
    print_time();
    do_errlog(LOG_STOP);
    Print("Disk exercising completed.\n");
    Print("\n");
}

/*----------------------------------------------------------------------------
 * readtest
 *
 * Test the disk in read-only mode.  This test will read a partition
 * sequentially or at random locations (through seeks).  The data read is
 * not validated or examined in any way.  The only check is to insure that
 * the value returned by the read system call is equal to the number of
 * bytes requested.  Transfer (reads) will be done in either fixed sizes, or
 * random sizes within a specified range.  The reads can be done in either
 * n-buffered mode or without n-buf.
 */
int
readtest()
{
    int proceed;
    int part;

    Print("\nRead-only Transfer Testing\n\n");
    Print("This test verifies that reads will succeed.  The data is not\n");
    Print("verified in any way.  Since this test does not write to the disk\n");
    Print("there is no possibility of file system damage.\n");
    if (attributes & FIXED_SIZE) {
	Print1("All reads will be of size %d bytes.\n",min_xfer);
    }
    else {
	Print("Reads will be done using random size transfers.  The read\n");
	Print2("size will be randomly selected from the range %d to %d bytes.\n",min_xfer, max_xfer);
    }
    if (test_partitions == PART_ALL) {
	Print("Reads will be done to all partitions of this disk.\n");
    }
    if (attributes & SEQUENTIAL) {
	Print("Reads will be done on a sequential basis.\n");
	if ((num_xfer == 0) && (test_partitions == PART_ALL)) {
	    Print("Enough reads will be issued to entirely read a partition\n");
	    Print("before alternating testing to a different partition.\n");
	}
	if ((num_xfer != 0) && (test_partitions == PART_ALL)) {
	    Print1("%d reads will be done before alternating testing to \n",num_xfer);
	    Print("different partition.\n");
	}
    }
    else {
	Print("Reads will be issued to random locations on the disk.  ");
	Print("To accomplish this\n");
	Print("a seek will be issued before each read ");
	Print("to force a read of a different\n");
	Print("disk region.\n");
    }

#ifndef TIN_NO_NBUF
    if (attributes & NO_NBUF) {
	Print("This read test will not utilize n-buffered I/O.\n");
    }
    else {
	Print1("This read test will utilize n-buffered I/O with %d buffers.\n", num_nbuf);
    }
#endif /* TIN_NO_NBUF */

    Print("\n");
    if (test_minutes) {
	Print1("Testing will be done for %d minutes.\n",test_minutes);
    }
    else {
	Print("Testing will continue until an interrupt signal is received.\n");
    }
    flush_output();
    /*
     * Before reading from the disk call diskx_fsck to make sure that the
     * partitions specified for testing are readable.
     */
    if (diskx_fsck(NO_WRITE) == FAILURE) {
        return(FAILURE);
    }
    /*
     * Now that the disk has been opened see if n-buf is requested to the
     * block special file.
     */
    if ((is_block_device) && ((attributes & NO_NBUF) == 0)) {
        Print("\n");
        Print("NOTE: Usage of n-buffered I/O is not supported on the\n");
        Print("block special file.  To run this test on the block device\n")
        Print("the -n option must be specified on the command line to\n");
	Print("disable the usage of n-buffered I/O.  N-buffered I/O is only\n");
        Print("supported on the character special file.\n")
        Print("\n");
        return;
    }
    /*
     * Clear the results array to insure that all stats are relevant to this
     * test.
     */
    init_results();
    /*
     * At this point the disk is ready for testing.
     */
    proceed = 1;
    while ( proceed ) {
	part = random() % MAX_PART;
	if (test_partitions & (1 << part)) {
	    set_xfer_size();
	    if (debug > 10) {
		Print1("Testing on the %c partition.\n",'A'+part);
		Print1("Set read size to %d bytes.\n",xfer_size);
	    }
	    if (do_reading(part, NO_VALIDATE, DO_SEEK0) != SUCCESS) {
		if (debug > 10) {
		    Print("The do_reading routine returned failure status.\n");
		}
	    }
	}
	flush_output();
        if ((test_minutes) && (time_up())) {
	    proceed = 0;
	}
    }
    /*
     * Print out the results of the read test.
     */
    print_xfers();
    return(SUCCESS);
}

/*----------------------------------------------------------------------------
 * writetest
 *
 * Test disk writes.  This test will write to the disk and then read back
 * for data validation.  The test operates as follows:
 *
 *	1) Set a new seed value.  This is done to insure that the data written
 *	   during each pass is different; otherwise you could be passing data
 *	   from a previous test iteration.
 *	2) Sequentially write the whole partition (or specified # of xfers).
 *	3) Sequentially read the whole partition.  Validate that the whole
 *	   partition has been successufly written.
 *	4) Perform random writes.
 *	5) Perform random reads and validate the data.  Note that step 2 (which
 *	   involves writing the whole partition) is necessary for the random
 *	   test to be done.  If only random writes were performed it would not
 *	   be possible to guarantee that the whole partition has been written.
 *	6) Go back to step 1 with a possibly different transfer size.
 *
 * Transfers (writes) will be done in either fixed
 * sizes, or random sizes within a specified range.  The writes can be done in 
 * either n-buffered mode or without n-buf.
 */
int
writetest()
{
    int proceed;
    int part;
    int save_seq = 0;		/* Save original attributes because they */
    int test_bad = 0;		/* Test continue variable.		 */

    if (attributes & OPEN_READONLY) {
	Print("Skipping the write test, the readonly attribute is set.\n");
	return;
    }

    Print("\nWrite Transfer Testing\n\n");
    Print("This test verifies that writes will succeed.  The data is first\n");
    Print("written to disk. After all writes have completed the data will be\n")
    Print("read back for validation.  Since this test writes to the disk \n");
    Print("there is potential for file system corruption if a file system \n");
    Print("exists on the disk that is being tested.\n");
    if (attributes & FIXED_SIZE) {
	Print1("All writes will be of size %d bytes.\n",min_xfer);
    }
    else {
	Print("Writes will be done using random size transfers.  The write\n");
	Print2("size will be randomly selected from the range %d to %d bytes.\n",min_xfer, max_xfer);
    }
    if (test_partitions == PART_ALL) {
	Print("Writes will be done to all partitions of this disk.\n");
    }
    if (attributes & SEQUENTIAL) {
	save_seq = 1;
	Print("Writes will be done on a sequential basis.\n");
	if ((num_xfer == 0) && (test_partitions == PART_ALL)) {
	    Print("Enough writes will be issued to entirely write a partition\n");
	    Print("before alternating testing to a different partition.\n");
	}
	if ((num_xfer != 0) && (test_partitions == PART_ALL)) {
	    Print1("%d writes will be done before alternating testing to \n",num_xfer);
	    Print("different partition.\n");
	}
    }
    else {
	Print("Writes will be issued to random locations on the disk.  ");
	Print("To accomplish this\n");
	Print("a seek will be issued before each write ");
	Print("to force a write of a different\n");
	Print("disk region.\n");
    }
#ifndef TIN_NO_NBUF
    if (attributes & NO_NBUF) {
	Print("This write test will not utilize n-buffered I/O.\n");
    }
    else {
	Print1("This write test will utilize n-buffered I/O with %d buffers.\n", num_nbuf);
    }
#endif /* TIN_NO_NBUF */

    Print("\n");
    if (test_minutes) {
	Print1("Testing will be done for %d minutes.\n",test_minutes);
    }
    else {
	Print("Testing will continue until an interrupt signal is received.\n");
    }
    Print("\n");
    flush_output();
    /*
     * Before writing from the disk call diskx_fsck to make sure that the
     * partitions specified for testing are writeable.
     */
    if (diskx_fsck(DO_WRITE) == FAILURE) {
        return(FAILURE);
    }
    /*
     * Now that the disk has been opened see if n-buf is requested to the
     * block special file.
     */
    if ((is_block_device) && ((attributes & NO_NBUF) == 0)) {
        Print("\n");
        Print("NOTE: Usage of n-buffered I/O is not supported on the\n");
        Print("block special file.  To run this test on the block device\n")
        Print("the -n option must be specified on the command line to\n");
	Print("disable the usage of n-buffered I/O.  N-buffered I/O is only\n");
        Print("supported on the character special file.\n")
        Print("\n");
        return;
    }
    /*
     * If this is a character device then we can tell if it is write protected.
     */
    if (is_block_device == 0) {
  	if (devio_print(DEVIO_WRTLCK)) {
	    Print("\n");
	    Print("ERROR: Aborting write testing because the disk is write protected.\n");
	    Print("\n");
	    return;
	}
    }
    /*
     * Clear the results array to insure that all stats are relevant to this
     * test.
     */
    init_results();
    /*
     * At this point the disk is ready for testing.
     */
    seed = 0;
    proceed = 1;
    while ( proceed ) {
	part = random() % MAX_PART;
	if (test_partitions & (1 << part)) {
	    set_xfer_size();
	    if (debug > 10) {
		Print1("Testing on the %c partition.\n",'A'+part);
		Print1("Set write size to %d bytes.\n",xfer_size);
	    }
	    test_bad = 0;
	    /*
	     * Sequentially write the whole partition, then read back for data
	     * validation.
	     */
	    attributes |= SEQUENTIAL;
	    if (debug) {
		Print1("Sequentially write to partition %c.\n",'A'+part);
	    }
	    if (do_writing(part, DO_VALIDATE, DO_SEEK0) != SUCCESS) {
		test_bad = 1;
		if (debug > 10) {
		    Print("The do_writing routine returned failure status.\n");
		}
	    }
	    if (debug) {
		Print1("Sequentially read verify partition %c.\n",'A'+part);
	    }
	    if ((test_bad == 0) && 
		(do_reading(part, DO_VALIDATE, DO_SEEK0) != SUCCESS)) {
		test_bad = 1;
		if (debug > 10) {
		    Print("The do_reading routine returned failure status.\n");
		}
	    }
	    if (debug) {
		Print("The initial write and read verification has ");
		if (test_bad) {
		    Print("failed");
		}
		else {
		    Print("succeeded");
		}
		Print(".\n");
	    }
	    /*
	     * Now that the initial writes have been validated it is
	     * possible to perform random tests.
	     */
	    if ((save_seq == 0) && (test_bad == 0)) {
		attributes &= ~SEQUENTIAL;
		if (debug) {
		    Print1("Perform random writes to partition %c\n",'A'+part);
		}
		if (do_writing(part, DO_VALIDATE, DO_SEEK0) != SUCCESS) {
		    test_bad = 1;
		    if (debug > 10) {
			Print("do_writing returns error on random write.\n");
		    }
		}
		else if (debug) {
		    Print("Random writes completed without error.\n");
		}
		if (debug) {
		    Print1("Perform random reads to partition %c\n",'A'+part);
		}
		if ((test_bad == 0) &&
		    (do_reading(part, DO_VALIDATE, DO_SEEK0) != SUCCESS)) {
		    test_bad = 1;
		    if (debug > 10) {
			Print("do_reading returns error on random read.\n");
		    }
		}
		if ((test_bad == 0) && (debug)) {
		    Print("Random reads completed without error.\n");
		}
	
	    }
	    /*
	     * Increment the seed value.  This will insure that the next
	     * iteration will cause different data to be written to the disk.
	     * if this wasn't done I suppose it would be possible for old
	     * data written to cause later reads to succeed.
	     */
	    seed++;
	}
	flush_output();
        if ((test_minutes) && (time_up())) {
	    proceed = 0;
	}
    }
    /*
     * Restore attributes to original values.
     */
    if (save_seq) {
        attributes |= SEQUENTIAL;
    }
    /*
     * Print out the results of the write test.
     */
    print_xfers();
}

/*---------------------------------------------------------------------------
 * do_writing
 *
 * This routine is just a dispatcher
 * based on whether or not n-buffered I/O is being used.
 * Open the device special file before writing, close the file after writing.
 * This routine contains code that is common to both nbuf_write and
 * non_nbuf_write.
 */
do_writing(partition, validate, seek_to_zero)
    int partition;			/* Partition to write to	*/
    int validate;			/* Specifies validate status    */
    int seek_to_zero;			/* Seek to 0 if set		*/
{
    int fd_write;
    char dsk_name[MAX_DISKNAME];
    char partchar[10];
    int max_seek;
    int numwrite;

    /*
     * Open the disk device.
     */
    sprintf(partchar, "%c", 'a' + partition);
    strcpy(dsk_name, disk_base);
    strcat(dsk_name, partchar);
    if ((fd_write  = open(dsk_name, O_RDWR)) < 0) {
	Print1("Cannot open %s.\n", dsk_name)
	if (debug > 10)
		Print1("Open failed, errno = %d\n",errno);
	return(FAILURE);
    }
    if (seek_to_zero == DO_SEEK0) {
        if (lseek(fd_write, (off_t)0, L_SET) < 0) {
            Print("Seek back to 0 failed.\n");
            xfer_results[partition].seek_errs++;
        }
	else {
	    xfer_results[partition].seeks++;
	}
    }
    /*
     * Determine how many multiples of the xfer_size variable this
     * partition is.  This number will determine how far onto the
     * partition seeks can be done to.
     */
    max_seek = (Pt_tab(partition).p_size * DEV_BSIZE) / xfer_size;
    /*
     * If a fixed number of transfers have not been specified then set
     * the transfer count to be sufficient to write the whole partition.
     * By writing in the whole partition, better test coverage of the disk
     * can be acheived.  If this was not done, then testing would just write
     * in the same number off the beginning of each partition and never end up
     * testing the full range of lbn's in each partition.
     */
    if (num_xfer == 0) {
	numwrite = max_seek;
    }
    else {
	numwrite = num_xfer;
    }
    max_seek = numwrite;
    if (debug > 10) {
	Print2("The maximum seek is %d with a size of %d\n",max_seek,xfer_size);
    }

    if (attributes & NO_NBUF) {
	non_nbuf_write(fd_write, validate, max_seek, numwrite, partition);
    }
    else {
	nbuf_write(fd_write, validate, max_seek, numwrite, partition);
    }
    close(fd_write);
    flush_output();
    return(SUCCESS);
}

/*---------------------------------------------------------------------------
 * non_nbuf_write
 *
 * Perform the actual writing off the disk.  This routine does not use
 * n-buffered I/O.
 */
non_nbuf_write(fd_write, validate, max_seek, xfer_count, partition)
    int fd_write;			/* File descriptor of disk      */
    int validate;			/* Specifies validate status    */
    register int max_seek;		/* Max seek multiple on partition */
    register int xfer_count;		/* Number of writes to do	*/
    register int partition;		/* Which partition is testing   */
{
    register int xfer_number;
    register off_t seek_location = 0;
    register int write_ret;

    if (debug > 50) {
	Print1("non_nbuf_write: do %d transfers.\n",xfer_count);
    }
    for(xfer_number = 0; xfer_number < xfer_count; xfer_number++) {
	/*
	 * When sequentially writing through a partition it is not 
	 * necessary to do an lseek to "reposition" between writes.  Examine
	 * the location to wrap-around back to the beginning of the partition.
	 * This will only hold true if the writes are in sizes which are an
	 * even multiple of DEV_BSIZE.
	 */
	if (attributes & SEQUENTIAL) {
	    if (seek_location > max_seek) {
		xfer_results[partition].seeks++;
		if (lseek(fd_write, (off_t)0, L_SET) < 0) {
		    Print("Seek back to 0 failed.\n");
		    xfer_results[partition].seek_errs++;
		    return;
		}
		seek_location = 0;
	    	if (debug > 20) {
		    Print("Seek back to 0 succeeded.\n");
	    	}
	    }
	    /*
	     * If validation is to be performed setup the transfer buffer to
	     * have known contents.  These contents will be compared to the
	     * contents of a read buffer at a later time.  In this case each
	     * disk block will only be written once.  Set the buffer pattern
	     * based on the location.  If no validation is
	     * to be performed it doesn't matter what the data is.
	     */
	    if (validate == DO_VALIDATE) {
	        setup_writebuf(seek_location, xfer_size, xfer_buf);
  	    }
	}
	/*
	 * Write to random locations, determine a new location and
	 * seek to that area.
	 */
	else {
	    seek_location = random() % max_seek;
	    seek_location *= xfer_size;
	    xfer_results[partition].seeks++;
	    if (lseek(fd_write, seek_location, L_SET) < 0) {
	        Print1("Seek to %d failed.\n", seek_location);
		xfer_results[partition].seek_errs++;
	        return;
	    }
	    if (debug > 90) {
	        Print1("Seek to %ld succeeded.\n", seek_location);
	    }
	    /*
	     * If validation is to be performed setup the transfer buffer to
	     * have known contents.  These contents will be compared to the
	     * contents of a read buffer at a later time.  In this case each
	     * disk block could be written more than once.  To allow the
	     * contents of the buffer to differ each time it is written, set
	     * the buffer pattern based on the present transfer number.
	     * If no validation is
	     * to be performed it doesn't matter what the data is.
	     */
	    if (validate == DO_VALIDATE) {
	        setup_writebuf(seek_location/xfer_size, xfer_size, xfer_buf);
  	    }
	}
	/*
	 * Now that the "reposition" has been done, perform the actual write.
	 */
	write_ret = write(fd_write, xfer_buf, xfer_size);
	xfer_results[partition].writes++;
	xfer_results[partition].write_bytes += xfer_size;
	seek_location++;	/* Sequential write moves pointer */
	if (write_ret != xfer_size) {
	    xfer_results[partition].write_errs++;
	    Print2("Write error, requested %d, returned %d\n", xfer_size, write_ret);
	}
	else {
	    if (debug > 90) {
	        Print1("Successfully wrote %d bytes.\n",write_ret);
	    }
	}
	/*
	 * Occasinally check to see if the time interval has expired.
	 * How often this is checked gives a certian granluarity to the
	 * specified time interval.
	 */
	if ((xfer_number % TIME_CHECK) == 0) {
            if ((test_minutes) && (time_up())) {
		break;
	    }
	}
    }	/* end of for(number of transfers) loop */
}

/*----------------------------------------------------------------------------
 * badtest
 *
 * badtest is specified in the testmatrix as the last test which should
 * never be called.  This is just a stub routine for non-existant tests.
 */
int
badtest()
{
	Print("Call to badtest\n");
	Print("Program error, this test should never be called.\n");
	do_errlog(LOG_ABORT);
	exit(1);
}

/*----------------------------------------------------------------------------
 * parse
 *
 *	Parse the command line.  
 *	Set flags which dictate the test to be performed.
 */
parse(argc, argv)
	int	argc ;
	char	*argv[] ;
{
    int 	i;
    TEST        *present_test;
    ATTRIBUTE   *chk_attr;

    if (argc < 2) {
	Print("diskx must be called with arguments.\n");
	do_errlog(LOG_COMMAND);
	exit(1);
    }
    for(i = 1; i < argc; i++) {
	/*
	 * First see if the argument specifies a test to run.
	 */
        for (present_test = testmatrix; present_test->option; present_test++) {
	    if( strcmp(argv[i], present_test->option) == 0 ) {
		present_test->state |= RUN_TEST;
		break;
	    }
	}
	/*
	 * If the argument does not specify a test it may specify an
	 * attribute.  Failing that it may specify a test parameter.
	 */
	if (present_test->test_routine == badtest ) {
          for (chk_attr = attribute_table; chk_attr->option; chk_attr++) {
	    if( strcmp(argv[i], chk_attr->option) == 0 ) {
		attributes |= chk_attr->attr_flag;
		break;
	    }
	  }
	  if (chk_attr->attr_flag == BAD_ATTRIBUTE) {
		/*
		 * Now see if the argument is a test parameter.  This was
		 * too messy to readily make it table driven.
		 */
		if ( strcmp(argv[i], "-f") == 0 ) {
		    /* Specifies the device special file name */
		    if( argc == ++i ) 
			need_word("-f");
		    else
			disk_cmdline = argv[i];
		}
		else if ( strcmp(argv[i], "-o") == 0 ) {
                    /* Specify a log file name */
		    if( argc == ++i )
			need_word("-o");		
		    else {
			setup_logfile(argv[i]);
		    }
		}
		else if( strcmp(argv[i], "-min_xfer") == 0 ) {
		    if( argc == ++i )
			need_word("-min_xfer");
		    else
			min_xfer = convert_value(argv[i]);
		    bounds_check(min_xfer,MIN_XFER,MAX_XFER);
		}
		else if( strcmp(argv[i], "-max_xfer") == 0 ) {
		    if( argc == ++i )
			need_word("-max_xfer");
		    else
			max_xfer = convert_value(argv[i]);
		    bounds_check(max_xfer,min_xfer,MAX_XFER);
		}
		else if( strcmp(argv[i], "-num_xfer") == 0 ) {
		    if( argc == ++i )
			need_word("-num_xfer");
		    else
			num_xfer = convert_value(argv[i]);
		    bounds_check(num_xfer,MIN_NUM_XFER,MAX_NUM_XFER);
		}
#ifndef TIN_NO_NBUF
		else if( strcmp(argv[i], "-num_nbuf") == 0 ) {
		    if( argc == ++i )
			need_word("-num_nbuf");
		    else
			num_nbuf = convert_value(argv[i]);
		    bounds_check(num_nbuf,MIN_NUM_NBUF,MAX_NUM_NBUF);
		}
#endif
		else if( strcmp(argv[i], "-perf_splits") == 0 ) {
		    if( argc == ++i )
			need_word("-perf_splits");
		    else
			perf_splits = convert_value(argv[i]);
		    bounds_check(perf_splits,MIN_PERF_SPLITS,MAX_PERF_SPLITS);
		}
		else if( strcmp(argv[i], "-perf_xfers") == 0 ) {
		    if( argc == ++i )
			need_word("-perf_xfers");
		    else
			perf_xfers = convert_value(argv[i]);
		    bounds_check(perf_xfers,MIN_NUM_XFER,MAX_NUM_XFER);
		}
		else if( strcmp(argv[i], "-perf_min") == 0 ) {
		    if( argc == ++i )
			need_word("-perf_min_xfer");
		    else
			perf_min_xfer = convert_value(argv[i]);
		    bounds_check(perf_min_xfer,MIN_XFER,MAX_XFER);
		}
		else if( strcmp(argv[i], "-perf_max") == 0 ) {
		    if( argc == ++i )
			need_word("-perf_max_xfer");
		    else
			perf_max_xfer = convert_value(argv[i]);
		    bounds_check(perf_max_xfer,perf_min_xfer,MAX_XFER);
		}
		else if( strcmp(argv[i], "-debug") == 0 ) {
		    if( argc == ++i )
			need_word("-debug");
		    else
			debug = convert_value(argv[i]);
		    bounds_check(debug,MIN_DEBUG,MAX_DEBUG);
		}
		else if( strcmp(argv[i], "-minutes") == 0 ) {
		    if( argc == ++i )
			need_word("-debug");
		    else
			test_minutes = convert_value(argv[i]);
		}
		else if( strcmp(argv[i], "-err_lines") == 0 ) {
			if( argc == ++i )
				need_word("-err_lines");	
			else
				max_err_lines = convert_value(argv[i]) ;
			bounds_check(max_err_lines,MIN_ERR_LINES,MAX_ERR_LINES);
		}
		else {
		    Print1("Invalid argument %s \(ignoring\).\n",argv[i]);
		}
	  }
	}
    }
    /*
     * Make sure the disk name looks reasonable.
     * The only time you don't need a disk name is if the help message is to
     * be printed.
     */
    if ((attributes & PRINT_HELP) == 0) {
        examine_disk_cmdline();
    }
}
/*
 *	The option expected another argument, which wasn't there.
 */
need_word(string)
	char *string;
{
	Print1("%s option needs additional argument.\n",string);
	do_errlog(LOG_COMMAND);
	exit(1);
}
/*
 * setup_logfile
 *
 * Open a logfile for writing.
 */
setup_logfile(logname)
	char *logname;
{
	if (debug)
		Print1("Enabling logging to file %s\n",logname);

	fd_logfile = fopen(logname, "w");
	if (fd_logfile == NULL) {
		Print("Unable to open log file; logging disabled.\n");
	}
	else {
		if (debug > 5)
			Print1("Successfully opened %s as a logfile.\n",
				logname);
		printstatus &= ~TTYPRINT;	/* Don't send to stdout */
		printstatus |= LOGFILE;		/* Enable logging */
	}
}	
/*
 *	Reads in a number from the command line and converts it by the
 *	appropriate scaling factor.
 *
 *		k or K - Kilobyte
 *		b or B - Block (512 bytes)
 *		s or S - Sector (512 bytes)
 *		m or M - Megabyte (1024 * 1024 = 1048576 bytes)
 *
 */
convert_value(s)
char	*s ;
{
	int	length, factor ;
	char	*p ;

	if((length = strlen(s)) == 0 )
		return(0);

	p = s + length - 1;

	if( isdigit(*p))			/* return value */
		return(atoi(s));
	else if( *p == 'k' || *p == 'K' )	/* kilobyte */
		factor = 1024;
	else if( *p == 'b' || *p == 'B' )	/* "block" */
		factor = 512;
	else if( *p == 's' || *p == 'S' )	/* sector */
		factor = 512;
	else if( *p == 'm' || *p == 'M' )	/* Megabyte */
		factor = 1024 * 1024;
	else
		return(atoi(s));			/* return value */

	*p = '\0' ;

	return(atoi(s) * factor);
}
/*
 * See if the disk name looks reasonable.  Also determine based on the
 * name which partitions to test.  If the name ends with a number then test
 * all partitions.  There can only be one letter following the unit number.
 * If present this one character must be between a and h.
 */
examine_disk_cmdline()
{
    char c;
    int i;
    int length;
    char *dname = disk_cmdline;
    int found_digit = 0;

    if ((disk_cmdline == 0) || (*disk_cmdline == '\0')) {
	Print("Command line error: no device name has been specified.\n");
	Print("Use the -f option to specify the device name.\n");
	do_errlog(LOG_COMMAND);
	exit(1);
    }
    test_partitions = PART_BAD;
    strcpy(diskname, disk_cmdline);
    /* 
     * Skip out to the unit number.
     */
    length = strlen(disk_cmdline);
    if (length > MAX_DISKNAME) {
	Print("The disk name exceeds the maximum length.\n");
	Print2("The specified name is %d characters, the maximum is %d.\n",length, MAX_DISKNAME);
    }
    for (i = 0; i < length; i++) {
	c = *dname++;
	if (isdigit(c)) {
	    found_digit++;
	    if (i == (length-1)) {
		test_partitions = PART_ALL;
		test_pnumber = 'c' - 'a';
		strcat(diskname, "c");
	    }
	}
	else if (found_digit) {
	    if (i != (length-1)) {
		Print("Too many characters specified as the partition.\n");
		do_errlog(LOG_COMMAND);
		exit(1);
	    }
	    else {
		if ((c < 'a') || (c > 'h')) {
		    Print1("Invalid partition: %c\n",c);
		    do_errlog(LOG_COMMAND);
		    exit(1);
		}
		else {
		    test_pnumber = c - 'a';
		    switch (c) {
			case 'a': test_partitions = PART_A; break;
			case 'b': test_partitions = PART_B; break;
			case 'c': test_partitions = PART_C; break;
			case 'd': test_partitions = PART_D; break;
			case 'e': test_partitions = PART_E; break;
			case 'f': test_partitions = PART_F; break;
			case 'g': test_partitions = PART_G; break;
			case 'h': test_partitions = PART_H; break;
		    }
		}
	    }
	}
    }
    strncpy(disk_base, diskname, strlen(diskname) - 1);
    /*
     * We shouldn't ge getting here without specifying any partitions.
     */
    if (test_partitions == PART_BAD) {
	Print("No partitions have been specified.\n");
	do_errlog(LOG_COMMAND);
	exit(1);
    }
}

/*
 * Flush output to standard out.  Also flush to logfile if logging is
 * being done.
 */
flush_output()
{
	fflush(stdout);
	if (printstatus & LOGFILE) {
		fflush(fd_logfile);
	}
}
/*
 * Bounds check
 * Insures that an user specified parameter falls within the acceptable
 * range.  Exit if the value is out of range.
 */
bounds_check(val,min,max)
	int val,min,max;
{
	if (val < min) {
		Print2("User specified parameter %d is less than the minimum %d\n",val,min);
		do_errlog(LOG_PARAM);
		exit(1);
	}
	else if (val > max) {
		Print2("User specified parameter %d is greater than the maximum %d\n",val,max);
		do_errlog(LOG_PARAM);
		exit(1);
	}
}

#define COMP31
/*----------------------------------------------------------------------------
 * disklabel_test
 *
 * This test compares the disktab entry in /etc/disktab to a dynamically
 * generated entry which is derived by calling creatediskbyname.  To do
 * this test use deviocget to determine the disk type to pass to 
 * getdiskbyname.
 */
int
disklabel_test()
{
    struct disklabel *get_p;  /* Fill in with call to getdiskbyname */
    struct disklabel *cre_p;  /* Fill in with call to creatediskbyname */
    struct devget dev_st;

    Print("\nDisktab Testing\n\n");
    Print("This test verifies the disklabel entry associated with this\n");
    Print("disk device.  The verification involves comparing the entry\n");
    Print("retruned by getdiskbyname\(3x\) to the entry dynamically\n");
    Print("generated by creatediskbyname\(3x\).\n");
    Print("\n");

#ifdef COMP31
    /*
     * The V3.1 variants do not support creatediskbyname.  In order to allow
     * this exerciser to compile on V3.1 systems the code that references 
     * creatediskbyname is being conditionally compiled out. 
     */
    Print("This operating system release does not support the\n");
    Print("creatediskbyname\(3x\) library routine.  For this reason\n");
    Print("it is not possible to perform the disklabel test.\n");
    Print("\n");
#else COMP31

    if (opendisk(diskname, O_RDONLY | O_NDELAY) != SUCCESS) {
	Print("\nERROR: Skipping this test due to open failure.\n\n");
	return;
    }
    if (is_block_device) {
	Print("\nERROR: This test does not work for the block device.\n\n");
	return;
    }
    /*
     * Call deviocget to acquire the disk type.
     */
    if ((ioctl(fd_disk,DEVIOCGET,&dev_st)) < 0) {
	Print("DEVIOCGET failed\n")
	Print("Aborting this test due to error in status aquisition.\n");
	return;
    }
    /*
     * Call routies to return 2 disklabel entries for the same disk.
     * One gets the info from /dec/disktab (getdiskbyname); the other
     * dynamically creates a disktab entry (creatediskbyname).
     */
    get_p = getdiskbyname(dev_st.device);
    cre_p = creatediskbyname(diskname);
    if (debug) {
    	/*
     	 * Print out the partition table entries.
     	 */
    	Print("Results of getdiskbyname:\n");
    	print_info(get_p);
    	Print("Results of creatediskbyname:\n");
    	print_info(cre_p);
    }

    /*
     * Compare the 2 structures.
     */
    if ((get_p != 0) && (cre_p != 0)) {
    	comp_disklabel(get_p, cre_p);
    }
    else {
	if (get_p == 0) {
	    Print("ERROR: Unable to obtain disklabel entry using getdiskbyname.\n");
	    Print1("The %d disk may not be in the disklabel file.\n",dev_st.device);
	    Print("\n");
	}
	if (cre_p == 0) {
	    Print("ERROR: Unable to obtain disklabel entry using creatediskbyname.\n");
	    Print("The creatediskbyname library call is not supported for\n");
	    Print("all disk types.\n");
	    Print("\n");
	}
    }
#endif COMP31
}

/*
 * opendisk
 *
 * Open the device special file.  To do this figure out the full device
 * path name in accordance with the partition mask.  Setup the global file
 * descriptor fd_disk to the open file descriptor.  Use fstat to determine if
 * this is a block or character special file.
 */
int
opendisk(dsk_name, mode_modifier)
	char *dsk_name;			/* Name of device special file */
	u_int mode_modifier;		/* Modifications to open mode  */
{
    int openmode;
    struct devget dev_st;
    struct stat stat_st;

    if (attributes & OPEN_READONLY) {
	if (debug > 5) {
		Print("Opening the device special file read-only.\n");
	}
	openmode = O_RDONLY;
    }
    else {
	openmode = O_RDWR;
    }

    openmode |= mode_modifier;

    if ((fd_disk  = open(dsk_name,openmode)) < 0) {
	Print1("Cannot open %s.\n", dsk_name)
	if (debug > 10)
		Print1("Open failed, errno = %d\n",errno);
	return(FAILURE);
    }

    if (fstat(fd_disk,&stat_st) < 0) {
	Print1("Cannot stat %s.\n", dsk_name);
	return(FAILURE);
    }
    stat_st.st_mode &= S_IFMT;	/* Mask off protections. */
    if (stat_st.st_mode == S_IFCHR) {
	is_block_device = 0;
    }
    else if (stat_st.st_mode == S_IFBLK) {
	is_block_device = 1;
    }
    else {
	Print1("%s is not a character or block special file.\n", dsk_name);
	return(FAILURE);
    }

    if (is_block_device == 0) {
        /*
         * Use devio to insure that this is in fact a disk device.
 	 * Devio is not supported for the block device.
         */
        if ((ioctl(fd_disk,DEVIOCGET,&dev_st)) < 0) {
	    Print("DEVIOCGET failed\n")
	    Print("Aborting open due to error in status aquisition.\n");
	    closedisk(dsk_name);
	    return(FAILURE);
        }
        if (dev_st.category != DEV_DISK) {
	    Print("DEV_DISK is not set in the category field.\n");
	    Print("Aborting open because this is not a disk device.\n");
	    closedisk(dsk_name);
	    return(FAILURE);
        }
	if (dev_st.stat & DEV_OFFLINE) {
	    Print("Aborting testing because the disk is offline.\n");
	    closedisk(dsk_name);
	    return(FAILURE);
	}
    }
    return(SUCCESS);
}

/*
 * closedisk
 *
 * Close the device special file.
 */
int 
closedisk(dsk_name)
	char *dsk_name;
{
    if (close(fd_disk) < 0) {
	Print1("Cannot close %s\n", dsk_name)
	if (debug > 10)
		Print1("Close failed, errno = %d\n",errno);
	return(FAILURE);
    }
    return(SUCCESS);
}

/*
 * Print_info: prints out the contents of a disklabel structure.
 */
print_info(dp)
    struct disklabel *dp;
{
    int i;
    
    if (dp == 0)  {
    	Print("Unable to print contents of disklabel struct\n");
    	printf("because pointer is 0.\n");
    }
    else {
    	Print("Display of disklabel structure:\n");
    	Print1("d_packname \t\t %s\n",dp->d_packname);
    	Print1("d_typename \t\t %s\n",dp->d_typename);
    	Print1("d_secsize \t %d\n",dp->d_secsize);
    	Print1("d_ntracks \t %d\n",dp->d_ntracks);
    	Print1("d_nsectors \t %d\n",dp->d_nsectors);
    	Print1("d_ncylinders \t %d\n",dp->d_ncylinders);
    	Print1("d_rpm \t\t %d\n",dp->d_rpm);
    	Print("Part\tp_size\tp_bsize\tp_fsize\n");
    	for (i=0; i < 8; i++) {
    		Print1("%c\t",'a'+i);
    		Print1("%d\t",dp->d_partitions[i].p_size);
/*
	Do not have a bsize field in the OSF disklabel
    		Print1("%d\t",dp->d_partitions[i].p_bsize);
*/
		Print1("%s",  "unknown");
    		Print1("%d\t",dp->d_partitions[i].p_fsize);
    		Print("\n");
    	}
    	Print("\n");
    }
}

/*
 * Compare the 2 disklabel structs.
 *
 * This routine is passed 2 disklabel structs.  The first disklabel is obtained
 * by getdiskbyname which gets the info from the /etc/disktab file.  The
 * second disklabel struct is obtained by calling creatediskbyname which derives
 * the information by asking the device driver.  The routine then compares the
 * two structs and displays the differences.
 */
comp_disklabel(get_p, cre_p)
	struct disklabel *get_p, *cre_p;
{
    int i;
    int error = 0;

    if ((get_p == 0) || (cre_p == 0)) {
    	Print("ERROR: comp_disklabel: 0 valued disklabel pointer.\n");
    	return;
    }

    Print("Structure Field      getdiskbyname       creatediskbyname\n");
    /*
     * Change to lower case the name field.  This gives a common base of
     * comparison.
     */
    make_lower(get_p->d_packname);
    make_lower(cre_p->d_packname);

    if ((strcmp(get_p->d_packname,cre_p->d_packname)) != 0) {
    	error++;
    	Print2("d_packname  \t\t%s \t\t\t%s\n",get_p->d_packname,cre_p->d_packname);
    }
    if ((strcmp(get_p->d_typename,cre_p->d_typename)) != 0) {
    	error++;
    	Print2("d_typename  \t\t%s \t\t\t%s\n",get_p->d_typename,cre_p->d_typename);
    }
    if (get_p->d_secsize != cre_p->d_secsize) {
    	error++;
    	Print2("d_secsize  \t\t%d \t\t\t%d\n",get_p->d_secsize, cre_p->d_secsize);
    }
    if (get_p->d_ntracks != cre_p->d_ntracks) {
    	error++;
    	Print2("d_ntracks  \t\t%d \t\t\t%d\n",get_p->d_ntracks, cre_p->d_ntracks);
    }
    if (get_p->d_nsectors != cre_p->d_nsectors) {
    	error++;
    	Print2("d_nsectors  \t\t%d \t\t\t%d\n",get_p->d_nsectors, cre_p->d_nsectors);
    }
    if (get_p->d_ncylinders != cre_p->d_ncylinders) {
    	error++;
    	Print2("d_ncylinders  \t\t%d \t\t\t%d\n",get_p->d_ncylinders, cre_p->d_ncylinders);
    }
    if (get_p->d_rpm != cre_p->d_rpm) {
    	error++;
    	Print2("d_rpm  \t\t%d \t\t\t%d\n",get_p->d_rpm, cre_p->d_rpm);
    }
    for (i=0; i < 8; i++) {
    	if (get_p->d_partitions[i].p_size != cre_p->d_partitions[i].p_size) {
    		error++;
    		Print3("partition %c p_size  \t%d \t\t\t%d\n",
    			i+'a',get_p->d_partitions[i].p_size,
    			cre_p->d_partitions[i].p_size);
    	}

/*	OSF disklabel does not have a bsize field
    	if (get_p->d_partitions[i].p_bsize != cre_p->d_partitions[i].p_bsize) {
    		error++;
    		Print3("partition %c p_bsize  \t%d \t\t\t%d\n",
    			i+'a',get_p->d_partitions[i].p_bsize,
    			cre_p->d_partitions[i].p_bsize);
    	}
*/
    	if (get_p->d_partitions[i].p_fsize != cre_p->d_partitions[i].p_fsize) {
    		error++;
    		Print3("partition %c p_fsize  \t%d \t\t\t%d\n",
    			i+'a',get_p->d_partitions[i].p_fsize,
    			cre_p->d_partitions[i].p_fsize);
    	}
    }

    Print("\n");
    if (error) {
    	Print1("ERROR: %d comparison errors occured.\n",error);
    }
    else {
    	Print("SUCCESS: No comparison errors.\n");
    }
}

/*
 * Convert a string to contain all lower case.
 */
make_lower(string)
    char *string;
{
    char c;

    while (( c = *string ) != '\0') {
    	if (isupper(c)) {
    		*string = tolower(c);
    	}
    	string++;
    }
}
/*
 * part_readable
 *
 * This routine is called to deterine if a partition is readable.  Testing
 * involves reading the first and last block of the partition.  This 
 * marginaly insures that all the blocks in the partition should exist.
 * Make sure the partition is defined (has some length).
 *
 * Testing is only performed on partitions which have been selected for
 * testing.
 */
int
part_readable(writemode) 
	int writemode;
{
    int i;
    int t_part;
    int fd_read;
    off_t last_block;
    char dsk_name[MAX_DISKNAME];
    char partchar[10];
    int readcount;
    char readbuf[DEV_BSIZE];
    struct fs superblock;
    char user_char;

    for (i = 0; i < MAX_PART; i++) {
	if (Pt_tab(i).p_size == 0) {
	    if (debug > 5) {
		Print1("Removing partition %c from the list of\n",'a'+i);
		Print("testable partitions because the partition\n");
		Print("size is zero.\n");
	    }
	    test_partitions &= ~(1 << i);
	    overlap_partitions &= ~(1 << i);
	}
    }
    if (test_partitions == 0) {
	Print("part_readable: Aborting testing because there are no\n");
	Print("testable partitions specified.\n");
	do_errlog(LOG_ABORT);
	exit(1);
    }
    fs_partitions = 0;
    /*
     * If read-only testing is being done then don't be concerned with
     * overlaps.  Overlaps are only a consideration when writes are being
     * done which could result in file system damage.
     */
    if ((writemode & DO_WRITE) == 0) {
	overlap_partitions = 0;
    }
    for (i = 0; i < MAX_PART; i++) {
	t_part = (1 << i);
	if ((test_partitions & t_part) || (overlap_partitions & t_part)) {
	    sprintf(partchar, "%c", 'a'+i);
	    strcpy(dsk_name, disk_base);
	    strcat(dsk_name, partchar);
	    fd_read = open(dsk_name, O_RDONLY);
	    if (fd_read < 0) {
		Print("part_readable: Aborting testing due to open failure\n");
		Print1("on %s.  Unable to determine if this \n",dsk_name);
		Print("partition is readable.\n");
		do_errlog(LOG_ABORT);
		exit(1);
	    }
	    /*
	     * Calculate the byte number for the last block of the partition.
   	     * First set last_block to be the last lbn.
	     * For the case of block devices reads are done in BLKDEV_IOSIZE
	     * units (2K).  For this reason, to read in the last block the
	     * last lbn is set back by 4 blocks to allow the 2K read.
	     * Finally multiply the last lbn by 512 to get the byte offset of
	     * where to begin reading in the last block.
	     * Note that all these calculations are done assuming the relative
	     * partition base offset of 0.  The disk driver itself will adjust
	     * for the actual partition base offset.
	     */
	    last_block = Pt_tab(i).p_size - 1;
	    if (is_block_device)
		last_block -= (BLKDEV_IOSIZE / DEV_BSIZE);
	    last_block *= DEV_BSIZE;
	    /*
	     * Read in the first block.
	     */
	    readcount = read(fd_read, readbuf, DEV_BSIZE);
	    if (readcount != DEV_BSIZE) {
		Print("part_readable: Unable to read the first block of\n");
		Print2("%s. The read returned %d.\n",dsk_name, readcount);
		do_errlog(LOG_ABORT);
		exit(1);
	    }
	    /*
	     * "Reposition" disk to the last block.
	     */
	    if (lseek(fd_read, (off_t)last_block, L_SET) < 0) {
		Print1("part_readable: Unable to seek to %d on\n",last_block);
		Print1("%s.\n",dsk_name);
		do_errlog(LOG_SEEK);
		exit(1);
	    }
	    /*
	     * Read in the last block.
	     */
	    readcount = read(fd_read, readbuf, DEV_BSIZE);
	    if (readcount != DEV_BSIZE) {
		Print("part_readable: Unable to read the last block of\n");
		Print2("%s. The read returned %d.\n",dsk_name, readcount);
		do_errlog(LOG_READERR);
		exit(1);
	    }
            /*
	     * See if there is a file system on the partition.  To do this
	     * first "reposition" to the super block location.
             */
            if (lseek(fd_read, (off_t)(SBLOCK * DEV_BSIZE) , L_SET) < 0) {
                Print("part_readable: Unable to seek to superblock\n");
                Print1("on %s.\n",dsk_name);
		do_errlog(LOG_SEEK);
                exit(1);
            }
	    /*
	     * Read in the superblock.
	     */
	    readcount = read(fd_read, (char *)&superblock, SBSIZE);
	    if (readcount != SBSIZE) {
		Print("part_readable: Unable to read the superblock of\n");
		Print2("%s. The read returned %d.\n",dsk_name, readcount);
		do_errlog(LOG_READERR);
		exit(1);
	    }
	    /*
	     * See if it is a valid superblock representing a file system.
	     */
	    if (superblock.fs_magic == FS_MAGIC) {
		fs_partitions |= (1 << i);
	    }
	    /*
	     * Close the device.
	     */
	    close(fd_read);
	}
    }
    /*
     * If there are existing file systems prompt the operator
     * to see if they want to continue before trashing them. 
     * There should be no chance of damage for the read-only test.  For
     * this reason, don't bother to prompt in the read-only case.  It is still
     * not a bad idea to read in the superblock above for the read-only test,
     * consider it just another aspect of the test itself.
     */
     if ((fs_partitions) && (writemode & DO_WRITE)) {
	Print("\n");
	Print("WARNING: Testing has been requested on partitions which\n");
	Print("contain file systems or the requested partitions overlap\n");
	Print("partitions which contain file systems.  These file systems\n");
	Print("will be destroyed if tests continue.  The following \n");
	Print("partitions contain file systems which may be destroyed:\n\n");
	for (i = 0; i < MAX_PART; i++) {
	    if (fs_partitions & (1 << i)) {
	        sprintf(partchar, "%c", 'a'+i);
	        strcpy(dsk_name, disk_base);
	        strcat(dsk_name, partchar);
		Print1("%s, ",dsk_name);
	    }
	}
	Print("\n\n");
	if (attributes & NO_CONFIRM) {
	    Print("Testing will proceed without prompting for confirmation\n");
	    Print("because the -Y attribute has been specified in the\n");
	    Print("command line.\n");
	    Print("\n");
	}
	else {
	    Print("Continue testing? [y,n]> ");
	    fflush(stdout);
	    scanf("%c",&user_char);
	    Print("\n");
	    if ((user_char != 'y') && (user_char != 'Y')) {
	        Print("Aborting testing at operator request.\n");
		do_errlog(LOG_READERR);
	        exit(1);
	    }
	}
     }
}

/*
 * part_overlap
 *
 * This routine sees if the partitions specified for testing overlap with any
 * other partitions.  This is used as part of the pre-test check to verify that
 * file systems will not be accidentally destroyed by the exerciser.  The
 * partition specified for testing may not have a file system, but an
 * overlapping partition does.  For this reason overlaps must be examined.
 * Overlap is determined by getting the disk's partition table by calling
 * DIOCGDINFO and comparing ranges of LBN's.
 */
int
part_overlap() 
{
    int fd_ioctl;
    int part_bottom, part_top;
    int test_bottom, test_top;
    int i;

    overlap_partitions = 0;
    if ((fd_ioctl = open(raw_diskname, O_RDONLY)) < 0) {
	Print1("part_overlap: Cannot open %s.\n", raw_diskname)
	if (debug > 10)
		Print1("Open failed, errno = %d\n",errno);
	do_errlog(LOG_OPENFAIL);
	exit(1);	
    }
    /*
     * Call diocgdinfo to acquire the disk label.
     */
    if ((ioctl(fd_ioctl,DIOCGDINFO,lp)) < 0) {
	Print("part_overlap: DIOCGDINFO failed\n")
	Print("Aborting this test due to failure in obtaining\n");
	Print1("partition layout (disk label) from %s.\n", raw_diskname);
	do_errlog(LOG_IOCTLFAIL);
	exit(1);	
    }

    part_bottom = Pt_tab(test_pnumber).p_offset;
    part_top = part_bottom + Pt_tab(test_pnumber).p_size - 1;
    /*
     * See if the regions intersect.  Of course the partition overlaps itself,
     * so skip the test for that case.
     */
    for (i = 0; i < MAX_PART; i++) {
	if (i != test_pnumber) {
	    test_bottom = Pt_tab(i).p_offset;
	    test_top = Pt_tab(i).p_offset + Pt_tab(i).p_size - 1;
	    if ((part_bottom <= test_top) && (part_top >= test_bottom)) {
	       overlap_partitions |= (1 << i);
	    }
	}
    }
    close(fd_ioctl);
}

/*
 * check_label()
 *
 * This routine is only called if write tests are requested on a partition
 * (or partitions).  Here we tread through the partitions to be tested and
 * check to see if the disk label sector resides on a partition we may
 * potentially write on.  If the label may get trashed, we first query the
 * user to see if they really want to trash it (and if they do, let the
 * trashing begin!).
 */ 
int
check_label() 
{
    int fd_ctl;
    int i,label_sector_found=0;
    int write_enable=1;
    char us_char=NULL;

    if ((fd_ctl = open(raw_diskname, O_RDWR)) < 0) {
	Print1("check_label: Cannot open %s.\n", raw_diskname)
	if (debug > 10)
		Print1("Open failed, errno = %d\n",errno);
	do_errlog(LOG_OPENFAIL);
	exit(1);	
    }
    /*
     * Attempt to read the disk label.
     */
    if ((ioctl(fd_ctl,DIOCGDINFO,lp)) < 0) {
	Print("check_label: DIOCGDINFO failed\n")
	Print("Aborting this test due to failure in obtaining\n");
	Print1("partition layout (disk label) from %s.\n", raw_diskname);
	do_errlog(LOG_IOCTLFAIL);
	exit(1);	
    }

    /*
     * For each of the test partitions being used, check to see if 
     * the disk label sector falls within the partition requested to
     * be written.
     */
    for (i = 0; i < MAX_PART; i++) {
        if (test_partitions & (1 << i)) {
            if ((LABELSECTOR >= Pt_tab(i).p_offset) &&
                (LABELSECTOR <= (Pt_tab(i).p_offset + Pt_tab(i).p_size-1))) {
                label_sector_found = 1;
            }
        }
    }

    if (label_sector_found) {
        Print("\n");
	Print("WARNING: Testing has been requested on partitions which\n");
	Print("contain the disk label of the disk to be tested.\n");
	Print("The disk label may be destroyed if tests continue.\n");

	Print("\n");
	if (attributes & NO_CONFIRM) {
	    Print("Testing will proceed without prompting for confirmation\n");
	    Print("because the -Y attribute has been specified in the\n");
	    Print("command line.\n");
	    Print("\n");
	}
	else {
            fflush(stdin);
	    Print("Continue testing? [y,n]> ");
	    fflush(stdout);
	    scanf("%c",&us_char);
	    Print("\n");
	    if ((us_char != 'y') && (us_char != 'Y')) {
	        Print("Aborting testing at operator request.\n");
		do_errlog(LOG_READERR);
	        exit(1);
	    }
	}
        /*
         * Here we attempt to write-enable the disk label on the
         * specified disk.  This allows diskx to later zonk the
         * label during write tests.
         */
        if (ioctl(fd_ctl, DIOCWLABEL, &write_enable) < 0) {
	    Print("check_label: DIOCWLABEL failed\n")
	    Print("Aborting this test due to failure in write\n");
	    Print1("enabling disk label on %s.\n", raw_diskname);
	    do_errlog(LOG_IOCTLFAIL);
	    exit(1);	
        }
    }
    flush_output();
    close(fd_ctl);
}

/*
 * diskx_fsck
 *
 * This routine is used to examine the disk to see which partitions have
 * existing file systems on them.  This routine should be called by any test
 * which writes to the disk to provide the operator a chance to abort testing
 * before any file systems are accidentaly destroyed.
 */
int
diskx_fsck(writemode) 
    int writemode;
{
    if (opendisk(diskname, 0) != SUCCESS) {
	Print("\nERROR: Skipping this test due to open failure.\n\n");
	return(FAILURE);
    }
    /*
     * Get the name of the character special file.
     */
    get_rawname(diskname);
    /*
     * Determine if any partitions overlap the test partition.
     */
    part_overlap();
    /*
     * Insure that the partitions of interest are non-zero in length and are
     * readable so that we can check for file system presence.  Prompt user
     * if any file systems exist.
     */
    part_readable(writemode);

    /*
     * Check if we are attempting to write over the partition which
     * contains the disk label.  Prompt the user if a disk label exists
     * and the test will write over it.
     */
    if (writemode & DO_WRITE) {
        check_label();
    }

    return(SUCCESS);
}

/*
 * get_rawname
 *
 * Setup the raw_diskname variable to specify the equivalent name
 * of the character special device associated with this block device.
 * This will be used later in cases where the raw device is needed to
 * do ioctls.
 *
 * This assumes that the raw device is formed by putting an "r" before
 * the disk name.  This "r" is placed before the first "r" that is
 * encountered in the disk name.  Another assumption is that the partition
 * will be specified by only 1 character and that character is the last
 * character in the device name.
 *
 * It is probably not a good to assume that these will always hold true.
 */
get_rawname(dsk_name)
    char *dsk_name;
{
    int length;
    int i;
    int offset;
    char c;

    if (opendisk(dsk_name, O_RDONLY | O_NDELAY) != SUCCESS) {
	Print("\nERROR: Skipping this test due to open failure.\n\n");
	return;
    }
    length = strlen(diskname);
    if (is_block_device) {
	/*
	 * Insert an "r" before the first "r" in the block name to get the
	 * character name.
	 */
	offset = 0;
	for (i = 0; i < length; i++) {
		c = dsk_name[i];
		if (c == 'r') {			/* Stuff in an "r" */
		    offset=1;
		    raw_diskname[i] = 'r';
		}
		raw_diskname[i+offset] = c;
	}
	raw_diskname[i+offset] = '\0';
	if (offset = 0) {
	    Print("\n");
	    Print("ERROR: aborting testing.\n");
	    Print("This exerciser only supports disks which have the\n");
	    Print("letter r in their name.  This is used to determine\n");
	    Print("the corresponding character special device which\n");
	    Print1("corresponds to this block special device %s.\n", dsk_name);
	    Print("\n");
	    do_errlog(LOG_COMMAND);
	    exit(1);
	}
    }
    else {
	/*
	 * This is already the character device, just copy the given name.
	 */
	strcpy(raw_diskname, dsk_name);
    }
    /*
     * Chop off the partition name for the raw_base variable.
     */
    strcpy(raw_base, raw_diskname);
    raw_base[length] = '\0';
}

/*---------------------------------------------------------------------------
 * start_timer
 *
 * Setup a global variable to be the ending time.  This is used when testing
 * is being done for a specific period of time.  The value will later be
 * compared in the time_up routine to see if it is time to stop.
 */
int
start_timer()
{
    if (test_minutes) {
	stop_time = (test_minutes * 60) + time(0);
	if (debug) {
	    Print1("Testing will continue for %d minutes.\n",test_minutes);
	}
    }
}

/*---------------------------------------------------------------------------
 * time_up
 *
 * Compare the present time to specified end time.  
 * Returns: 0 - timer not expired, 1 - timer expired.
 */
int
time_up()
{
    if (test_minutes) {
	if (stop_time <= time(0)) {
	    if (debug)
		Print1("time_up: Time interval of %.1f minutes has expired.\n",test_minutes);
	    return(1);
	}
    }
    return(0);
}

/*---------------------------------------------------------------------------
 * set_xfer_size
 *
 * This routine sets the global variable xfer_size to be a specific
 * transfer (read or write) size.  For the case of fixed size testing, the
 * assignment is always the same.  If it is not fixed sized transfers then
 * the transfer size is random. 
 */
int
set_xfer_size()
{
    int block_offset;

    if (attributes & FIXED_SIZE) {
	xfer_size = min_xfer;
    }
    else {
	/*
	 * Granted, the srandom could return a more random distribution.  
	 * What is good about the random routine is that the paterns may be
	 * more deterministic.  This may make failure reproduction more
	 * likely.
	 */
	xfer_size = random() % max_xfer;
    }
    /*
     * Transfers to the character special file should be done in multiples
     * of DEV_BSIZE.  This is because the file offset will be divided modulo
     * DEV_BSIZE to determine which lbn within the partition the data resides
     * on; as a consequence, reads will always start with the first byte of an
     * lbn.
     * Block devices on the other hand can seek to anywhere within the 
     * partition range and read in any number of bytes.
     */
    if (is_block_device == 0) {
	block_offset = xfer_size % DEV_BSIZE;
     	xfer_size -= block_offset;
	if (xfer_size <= 0) {
	    xfer_size = DEV_BSIZE;
	}
    }
}

/*---------------------------------------------------------------------------
 * do_reading
 *
 * This routine is just a dispatcher
 * based on whether or not n-buffered I/O is being used.
 * Open the device special file before reading, close the file after reading.
 * This routine contains code that is common to both nbuf_read and
 * non_nbuf_read.
 */
do_reading(partition, validate, seek_to_zero)
    int partition;			/* Partition to read from	*/
    int validate;			/* Specifies validate status    */
    int seek_to_zero;			/* Seek to 0 befire reads if set*/
{
    int fd_read;
    char dsk_name[MAX_DISKNAME];
    char partchar[10];
    int max_seek;
    int numread;

    /*
     * Open the disk device.
     */
    sprintf(partchar, "%c", 'a' + partition);
    strcpy(dsk_name, disk_base);
    strcat(dsk_name, partchar);
    if ((fd_read  = open(dsk_name, O_RDONLY)) < 0) {
	Print1("Cannot open %s.\n", dsk_name)
	if (debug > 10)
		Print1("Open failed, errno = %d\n",errno);
	return(FAILURE);
    }
    /*
     * Get the block pinter to the beginning of the partition before doing
     * any transfers.
     */
    if (seek_to_zero == DO_SEEK0) {
        if (lseek(fd_read, (off_t)0, L_SET) < 0) {
            Print("Seek back to 0 failed.\n");
            xfer_results[partition].seek_errs++;
        }
	else {
	    xfer_results[partition].seeks++;
	}
    }
    /*
     * Determine how many multiples of the xfer_size variable this
     * partition is.  This number will determine how far onto the
     * partition seeks can be done to.
     */
    max_seek = (Pt_tab(partition).p_size * DEV_BSIZE) / xfer_size;
    if (debug > 10) {
	Print2("The maximum seek is to %d with a transfer size of %d.\n",max_seek, xfer_size);
    }
    /*
     * If a fixed number of transfers have not been specified then set
     * the transfer count to be sufficient to read the whole partition.
     * By reading in the whole partition, better test coverage of the disk
     * can be acheived.  If this was not done, then testing would just read
     * in the same number off the beginning of each partition and never end up
     * testing the full range of lbn's in each partition.
     */
    if (num_xfer == 0) {
	numread = max_seek;
    }
    else {
	numread = num_xfer;
    }
    max_seek = numread;

    if (attributes & NO_NBUF) {
	non_nbuf_read(fd_read, validate, max_seek, numread, partition);
    }
    else {
	nbuf_read(fd_read, validate, max_seek, numread, partition);
    }
    close(fd_read);
    flush_output();
    return(SUCCESS);
}

/*---------------------------------------------------------------------------
 * non_nbuf_read
 *
 * Perform the actual reading off the disk.  This routine does not use
 * n-buffered I/O.
 */
non_nbuf_read(fd_read, validate, max_seek, xfer_count, partition)
    int fd_read;			/* File descriptor of disk      */
    int validate;			/* Specifies validate status    */
    register int max_seek;		/* Max seek multiple on partition */
    register int xfer_count;		/* Number of reads to do	*/
    register int partition;		/* Which partition is testing   */
{
    register int xfer_number;
    register off_t seek_location = 0;
    register int read_ret;
    int i;
    int compval = -1;
    int valerr = 0;
    unsigned char valchar;

    if (debug > 50) {
	Print1("non_nbuf_read: do %d transfers.\n",xfer_count);
    }
    for(xfer_number = 0; xfer_number < xfer_count; xfer_number++) {
	/*
	 * When sequentially reading through a partition it is not 
	 * necessary to do an lseek to "reposition" between reads.  Examine
	 * the location to wrap-around back to the beginning of the partition.
	 * This will only hold true if the reads are in sizes which are an
	 * even multiple of DEV_BSIZE.
	 */
	if (attributes & SEQUENTIAL) {
	    if (seek_location > max_seek) {
		xfer_results[partition].seeks++;
		if (lseek(fd_read, (off_t)0, L_SET) < 0) {
		    Print("Seek back to 0 failed.\n");
		    xfer_results[partition].seek_errs++;
		    return;
		}
		seek_location = 0;
		compval = 0;
	    	if (debug > 20) {
		    Print("Seek back to 0 succeeded.\n");
	    	}
	    }
	    else {
		compval++;
	    }
	}
	/*
	 * Read from random locations, determine a new location and
	 * seek to that area.
	 */
	else {
	    seek_location = random() % max_seek;
	    compval = seek_location;
	    seek_location *= xfer_size;
	    xfer_results[partition].seeks++;
	    if (lseek(fd_read, seek_location, L_SET) < 0) {
	        Print1("Seek to %d failed.\n", seek_location);
		xfer_results[partition].seek_errs++;
	        return;
	    }
	    if (debug > 90) {
	        Print1("Seek to %d succeeded.\n", seek_location);
	    }
	}
	/*
	 * Now that the "reposition" has been done, perform the actual read.
	 */
	read_ret = read(fd_read, xfer_buf, xfer_size);
	xfer_results[partition].reads++;
	xfer_results[partition].read_bytes += xfer_size;
	if (read_ret != xfer_size) {
	    xfer_results[partition].read_errs++;
	    Print2("Read error, requested %d, returned %d\n", xfer_size, read_ret);
	}
	else {
	    if (debug > 90) {
	        Print1("Successfully read %d bytes.\n",read_ret);
	    }
	    if (validate == DO_VALIDATE) {
		valerr = 0;
		for (i = 0; i < xfer_size; i++) {
		    valchar = (compval + seed + i) & 0x7f;
		    if (xfer_buf[i] != valchar) {
			valerr++;
			if (debug > 150) {
			    Print3("ERROR: read[%d] = 0x%x, val = 0x%x\n",i,xfer_buf[i], valchar);
			}
		    }
		    else {
			if (debug > 200) {
			    Print3("OK: read[%d] = 0x%x, val = 0x%x\n",i,xfer_buf[i], valchar);
			}
		    }
		}
		/*
		 * Increment the data validation error count if any errors
	         * occured.  Don't increment this value by the number of bytes
		 * in error because the count would grow too quickly.  Chances
		 * are that if one byte is off the whole transfer will be off.
		 */
		if (valerr) {
		    if (debug) {
			Print1("%d validation errors with this transfer.\n",valerr);
		    }
		    xfer_results[partition].val_errs++;
		}
	    }
	}
	seek_location++;	/* Sequential read moves pointer */
	/*
	 * Occasinally check to see if the time interval has expired.
	 * How often this is checked gives a certian granluarity to the
	 * specified time interval.
	 */
	if ((xfer_number % TIME_CHECK) == 0) {
	    if ((test_minutes) && (time_up())) {
		break;
	    }
	}
    }
}


#ifndef	TIN_NO_NBUF
/*---------------------------------------------------------------------------
 * nbuf_read
 *
 * Perform the actual reading off the disk.  This routine does use
 * n-buffered I/O.
 *
 * Note that n-buffered I/O can not be used on the block device.
 */
nbuf_read(fd_read, validate, max_seek, xfer_count, partition)
    int fd_read;			/* File descriptor of disk      */
    int validate;			/* Specifies validate status    */
    int max_seek;			/* Max seek multiple on partition */
    register int xfer_count;		/* Number of reads to do	*/
    register int partition;		/* Which partition is testing   */
{
    int i;				/* Loop counter			*/
    int read_pending[MAX_NUM_NBUF];	/* N-buf status			*/
    register int read_number;		/* Which xfer is in progress    */
    int readmore;			/* Continue to read flag	*/
    register int bufnum;		/* Which particular buffer      */
    int read_ret;			/* Return value of read syscall	*/
    register int rc;			/* Read count from FIONBDONE	*/
    int failed_reads;			/* Consecutive read error count */
    int failure_errno;			/* Read failure status		*/
    int same_errno;			/* Distinguish error patterns   */
    register int completed_xfer;	/* # of completed reads	        */
    off_t seek_location;		/* Location within partition    */
    int compvals[MAX_NUM_NBUF];		/* Value of first byte of xfer  */
    int valerr;				/* Count of validation errors   */
    unsigned char valchar;		/* Expected value of read byte  */

    /*
     * Initialize routine variables.
     * Initialize n-buf status as all buffers free.
     */
    for(i = 0; i < MAX_NUM_NBUF; i++) {
	read_pending[i] = 0;
	compvals[i] = 0;
    }
    read_number = 0;
    readmore = 1;
    failed_reads = 0;
    failure_errno = 0;
    same_errno = 0;
    completed_xfer = 0;
    seek_location = 0;
    valerr = 0;
    /*
     * Clear out any leftover I/O.
     * Enable n-buffered I/O operation using num_nbuf buffers.
     */
    nbuf_setup(fd_read, num_nbuf, ALLOC_BUFFERS);
    if (debug > 2) {
        Print("Performing read number: ");
        flush_output();
    }
    /* 
     * Perform the actual n-buffered reads.
     */
    while (readmore) {
        for (bufnum=0; bufnum < num_nbuf; bufnum++){
	    /*
	     * Occasinally check to see if the time interval has expired.
	     * How often this is checked gives a certian granluarity to the
	     * specified time interval.
	     */
	    if ((read_number % TIME_CHECK) == 0) {
	        if ((test_minutes) && (time_up())) {
		    readmore = 0;
		    break;
	        }
	    }
            /*
             * Start a read using this buffer if it isn't already
             * busy.  Only issue the number of transfers which is
             * specified in the xfer_count parameter.
             */
nbufread:
            if (read_pending[bufnum] != XFER_IP) {
              if (read_number < xfer_count) {
            	/*
 		 * Perform a seek operation if necessary.
		 *
            	 * When sequentially reading through a partition it is not 
            	 * necessary to do an lseek to "reposition" between reads.  
            	 * Examine the location to wrap-around back to the beginning of
            	 * the partition.  This will only hold true if the reads are in
	         * sizes which are an even multiple of DEV_BSIZE.
            	 */
            	if (attributes & SEQUENTIAL) {
            	    if (seek_location > max_seek) {
            		xfer_results[partition].seeks++;
            		if (lseek(fd_read, (off_t)0, L_SET) < 0) {
            		    Print("Seek back to 0 failed.\n");
            		    xfer_results[partition].seek_errs++;
            		    return;
            		}
            		seek_location = 0;
            	    	if (debug > 20) {
            		    Print("Seek back to 0 succeeded.\n");
            	    	}
            	    }
		    compvals[bufnum] = seek_location;
            	}
            	/*
            	 * Read from random locations, determine a new location and
            	 * seek to that area.
            	 */
            	else {
            	    seek_location = random() % max_seek;
		    compvals[bufnum] = seek_location;
            	    seek_location *= xfer_size;
            	    xfer_results[partition].seeks++;
            	    if (lseek(fd_read, seek_location, L_SET) < 0) {
            	        Print1("Seek to %d failed.\n", seek_location);
            		xfer_results[partition].seek_errs++;
            	        return;
            	    }
            	    if (debug > 90) {
            	        Print1("Seek to %d succeeded.\n", seek_location);
            	    }
            	}
                read_ret = read(fd_read, nbuf_area[bufnum].buffer,xfer_size);
		xfer_results[partition].reads++;
		xfer_results[partition].read_bytes += xfer_size;
		seek_location++;	/* Sequential read moves pointer */
                read_number++;
                if (read_ret != xfer_size) {
		    xfer_results[partition].read_errs++;
                    if (debug) {
                        Print1("nbuffered read %d failed\n",bufnum);
                        if (debug > 10)
                            Print1("nbuf read error, errno = %d\n",errno);
                    }
                    /*
                     * To avoid getting in an infinite loop if
                     * the read system call is failing, count
                     * the number of CONSECUTIVE read failures
                     * in failed_reads.  If this value exceeds
                     * twice the number of n-buffers then abort
                     * the reading.  Check for patterns of the
                     * same error number.
                     */
                    if (failed_reads == 0) {
                        failure_errno = errno;
                        same_errno = 1;
                    }
                    else if (errno == failure_errno) {
                        same_errno++;
                    }
                    failed_reads++;
                    if (failed_reads > (num_nbuf * 2)) {
                        Print1("ERROR: Aborting nbuf_read after %d consecutive read errors.\n",failed_reads);
                        if (same_errno == failed_reads) {
                            Print1("The errno was %d every time.\n",failure_errno);
                        }
                        else {
                            Print2("The errno was %d in %d failures.\n",failure_errno, same_errno);
                        }
                        readmore = 0;
                        /*
                         * Only put up with these n-buf errors
                         * for so long then bail out.
                         */
                        nbuf_aborts++;
                        if (nbuf_aborts > MAX_NBUF_ABORTS) {
                            Print("\nERROR: aborting diskx exerciser due to repeated n-buf failures.\n");
                            Print1("%d n-buf aborts have occured.\n",nbuf_aborts);
	    		    do_errlog(LOG_NBUF);
                            exit(1);
                        }
                        break;
                      }
                }
                /*
                 * A successful read has been started.  Set in progress
                 * flag so that the status of this read will be
                 * examined later. The failed_reads variable is a 
                 * counter of CONSECUTIVE read failures.  Since this
                 * read did not fail, clear this failure counter.
                 */
                else {
                    read_pending[bufnum] = XFER_IP;
                    failed_reads = 0;
                    if (debug > 200) {
                        Print1("start of read buffer %d\n",bufnum);
                    }
                }
                if (debug)
                    flush_output();
                  }
	    }
            /*
             * The buffer is busy.  Check the status of a previous
             * read operation.
             */
            else  if (read_pending[bufnum] == XFER_IP) {
                if ((rc = ioctl(fd_read, FIONBDONE,
                    &nbuf_area[bufnum].buffer)) != -1) {
                    if (rc == 0) {
                        if (debug > 10)
                            Print("nbuf read: read returned 0.\n");
                        readmore = 0;
                    }
                    /*
                     * A partial read.
                     */
                    else if (rc != xfer_size) {
                        if (debug)
                            Print2("PARTIAL: nbuf_read, read %d, requested %d bytes.\n",rc, xfer_size);
		        xfer_results[partition].read_errs++;
                        /*
                         * Free up the buffer for later use.
                         */
                        if (debug > 200) {
                            Print1("Free buffer %d\n",bufnum);
                            flush_output();
                        }
                        read_pending[bufnum] = 0;
                        completed_xfer++;
                        if (completed_xfer >= xfer_count) {
                            readmore = 0;
                        }    
                    }
                    /*
                     * A full transfer has been read.
		     * (rc == xfer_size)
                     */
                    else {
			/*
			 * Perform data validation if requested.
			 * compvals[bufnum] was setup when the read was issued
			 * to be the expected value of the first byte read.
			 */
        		if (validate == DO_VALIDATE) {
        		    valerr = 0;
        		    for (i = 0; i < xfer_size; i++) {
        		        valchar = (compvals[bufnum] + seed + i) & 0x7f;
        		        if (nbuf_area[bufnum].buffer[i] != valchar) {
        			    valerr++;
        			    if (debug > 150) {
        			        Print3("ERROR: read[%d] = 0x%x, val = 0x%x\n",i,nbuf_area[bufnum].buffer[i], valchar);
        			    }
        		        }
        		        else {
        			    if (debug > 200) {
        			        Print3("OK: read[%d] = 0x%x, val = 0x%x\n",i,nbuf_area[bufnum].buffer[i], valchar);
        			    }
        		        }
        		    }
        		    /*
        		     * Increment the data validation error count if any
        		     * errors occured.  Don't increment this value by 
        		     * the number of bytes in error because the count 
			     * would grow too quickly.  Chances are that if
        		     * one byte is off the whole transfer will be off.
        		     */
        		    if (valerr) {
        		        if (debug) {
        			    Print1("%d validation errors with this transfer.\n",valerr);
        		        }
        		        xfer_results[partition].val_errs++;
        		    }
			}
                        /*
                         * Free up the buffer for later use.
                         */
                        completed_xfer++;
                        if (debug > 2) {
                            Print1("%d ",completed_xfer);
                            if ((completed_xfer%20) == 0)
                                Print("\n");
                            flush_output();
                        }
                        read_pending[bufnum] = 0;
                        if (completed_xfer >= xfer_count) {
                            readmore = 0;
                        }    
                        /*
                         * Start up a new read with this
                         * freed buffer.
                         */
                        goto nbufread;
                    }
                }
		/*
		 * FIONBDONE returned -1
		 */
                else {
                    if (debug > 5) {
                        Print1("nbuf_read #%d, returned -1\n", bufnum);
                        if (debug > 10)
                            Print1("FIONBDONE error, errno = %d\n",errno);
                    }
		    xfer_results[partition].read_errs++;
                    readmore = 0;	/* Terminate reading */
                    /*
                     * See if the disk went offline.  
                     */
                    if (devio_print(DEVIO_OFFLINE)) {
		        Print("\nERROR: Aborting testing because the disk went offline.\n");
	    		do_errlog(LOG_OFFLINE);
		        exit(1);
		    }
                }
            }		/* End of XFER_IP section 	*/
        }		/* End of for num_nbuf loop 	*/
    }			/* End of while(readmore)	*/
    /*
     * Free up allocated buffer space and turn off usage of n-buf.
     */
    nbuf_disable(fd_read, num_nbuf);
    if (debug > 2) {
        Print("\n");
        flush_output();
    }
}

/*---------------------------------------------------------------------------
 * nbuf_write
 *
 * Perform the actual writing to the disk.  This routine does use
 * n-buffered I/O.
 *
 * Note that n-buffered I/O can not be used on the block device.
 */
nbuf_write(fd_write, validate, max_seek, xfer_count, partition)
    int fd_write;			/* File descriptor of disk      */
    int validate;			/* Specifies validate status    */
    int max_seek;			/* Max seek multiple on partition */
    register int xfer_count;		/* Number of writes to do	*/
    register int partition;		/* Which partition is testing   */
{
    int i;				/* Loop counter			*/
    int write_pending[MAX_NUM_NBUF];	/* N-buf status			*/
    register int write_number;		/* Which xfer is in progress    */
    int writemore;			/* Continue to write flag	*/
    register int bufnum;		/* Which particular buffer      */
    int write_ret;			/* Return value of write syscall*/
    register int wc;			/* Write count from FIONBDONE	*/
    int failed_writes;			/* Consecutive write error count*/
    int failure_errno;			/* Write failure status		*/
    int same_errno;			/* Distinguish error patterns   */
    register int completed_xfer;	/* # of completed writes	*/
    off_t seek_location;		/* Location within partition    */

    /*
     * Initialize routine variables.
     * Initialize n-buf status as all buffers free.
     */
    for(i = 0; i < MAX_NUM_NBUF; i++) {
	write_pending[i] = 0;
    }
    write_number = 0;
    writemore = 1;
    failed_writes = 0;
    failure_errno = 0;
    same_errno = 0;
    completed_xfer = 0;
    seek_location = 0;
    /*
     * Clear out any leftover I/O.
     * Enable n-buffered I/O operation using num_nbuf buffers.
     */
    nbuf_setup(fd_write, num_nbuf, ALLOC_BUFFERS);
    if (debug > 2) {
        Print("Performing write number: ");
        flush_output();
    }
    /* 
     * Perform the actual n-buffered writes.
     */
    while (writemore) {
        for (bufnum=0; bufnum < num_nbuf; bufnum++){
	    /*
	     * Occasinally check to see if the time interval has expired.
	     * How often this is checked gives a certian granluarity to the
	     * specified time interval.
	     */
	    if ((write_number % TIME_CHECK) == 0) {
	        if ((test_minutes) && (time_up())) {
		    writemore = 0;
		    break;
	        }
	    }
            /*
             * Start a write using this buffer if it isn't alwritey
             * busy.  Only issue the number of transfers which is
             * specified in the xfer_count parameter.
             */
nbufwrite:
            if (write_pending[bufnum] != XFER_IP) {
              if (write_number < xfer_count) {
            	/*
 		 * Perform a seek operation if necessary.
		 *
            	 * When sequentially writing through a partition it is not 
            	 * necessary to do an lseek to "reposition" between writes.  
            	 * Examine the location to wrap-around back to the beginning of
            	 * the partition.  This will only hold true if the writes are in
	         * sizes which are an even multiple of DEV_BSIZE.
            	 */
            	if (attributes & SEQUENTIAL) {
            	    if (seek_location > max_seek) {
            		xfer_results[partition].seeks++;
            		if (lseek(fd_write, (off_t)0, L_SET) < 0) {
            		    Print("Seek back to 0 failed.\n");
            		    xfer_results[partition].seek_errs++;
            		    return;
            		}
            		seek_location = 0;
            	    	if (debug > 20) {
            		    Print("Seek back to 0 succeeded.\n");
            	    	}
            	    }
	    	    /*
		     * If data validation is to be done setup the write buffer
		     * to have known contents; otherwise it doesn't matter what
		     * is being written.
	     	     */
	    	    if (validate == DO_VALIDATE) {
	        	setup_writebuf(seek_location, xfer_size, 
				nbuf_area[bufnum].buffer);
  	    	    }
            	}
            	/*
            	 * Write to random locations, determine a new location and
            	 * seek to that area.
            	 */
            	else {
            	    seek_location = random() % max_seek;
            	    seek_location *= xfer_size;
            	    xfer_results[partition].seeks++;
            	    if (lseek(fd_write, seek_location, L_SET) < 0) {
            	        Print1("Seek to %d failed.\n", seek_location);
            		xfer_results[partition].seek_errs++;
            	        return;
            	    }
            	    if (debug > 90) {
            	        Print1("Seek to %d succeeded.\n", seek_location);
            	    }
	    	    /*
		     * If data validation is to be done setup the write buffer
		     * to have known contents; otherwise it doesn't matter what
		     * is being written.
	     	     */
	    	    if (validate == DO_VALIDATE) {
	        	setup_writebuf(seek_location/xfer_size, xfer_size, 
				nbuf_area[bufnum].buffer);
  	    	    }
            	}
                write_ret = write(fd_write, nbuf_area[bufnum].buffer,xfer_size);
		xfer_results[partition].writes++;
		xfer_results[partition].write_bytes += xfer_size;
		seek_location++;	/* Sequential write moves pointer */
                write_number++;
                if (write_ret < 0) {
		    xfer_results[partition].write_errs++;
                    if (debug) {
                        Print1("nbuffered write %d failed\n",bufnum);
                        if (debug > 10)
                            Print1("nbuf write error, errno = %d\n",errno);
                    }
                    /*
                     * To avoid getting in an infinite loop if
                     * the write system call is failing, count
                     * the number of CONSECUTIVE write failures
                     * in failed_writes.  If this value exceeds
                     * twice the number of n-buffers then abort
                     * the writing.  Check for patterns of the
                     * same error number.
                     */
                    if (failed_writes == 0) {
                        failure_errno = errno;
                        same_errno = 1;
                    }
                    else if (errno == failure_errno) {
                        same_errno++;
                    }
                    failed_writes++;
                    if (failed_writes > (num_nbuf * 2)) {
                        Print1("ERROR: Aborting nbuf_write after %d consecutive write errors.\n",failed_writes);
                        if (same_errno == failed_writes) {
                            Print1("The errno was %d every time.\n",failure_errno);
                        }
                        else {
                            Print2("The errno was %d in %d failures.\n",failure_errno, same_errno);
                        }
                        writemore = 0;
                        /*
                         * Only put up with these n-buf errors
                         * for so long then bail out.
                         */
                        nbuf_aborts++;
                        if (nbuf_aborts > MAX_NBUF_ABORTS) {
                            Print("\nERROR: aborting diskx exerciser due to repeated n-buf failures.\n");
                            Print1("%d n-buf aborts have occured.\n",nbuf_aborts);
	    		    do_errlog(LOG_NBUF);
                            exit(1);
                        }
                        break;
                      }
                }
                /*
                 * A successful write has been started.  Set in progress
                 * flag so that the status of this write will be
                 * examined later. The failed_writes variable is a 
                 * counter of CONSECUTIVE write failures.  Since this
                 * write did not fail, clear this failure counter.
                 */
                else {
                    write_pending[bufnum] = XFER_IP;
                    failed_writes = 0;
                    if (debug > 200) {
                        Print1("start of write buffer %d\n",bufnum);
                    }
                }
                if (debug)
                    flush_output();
                  }
	    }
            /*
             * The buffer is busy.  Check the status of a previous
             * write operation.
             */
            else  if (write_pending[bufnum] == XFER_IP) {
                if ((wc = ioctl(fd_write, FIONBDONE,
                    &nbuf_area[bufnum].buffer)) != -1) {
                    if (wc == 0) {
                        if (debug > 10)
                            Print("nbuf write: write returned 0.\n");
                        writemore = 0;
                    }
                    /*
                     * A partial write.
                     */
                    else if (wc != xfer_size) {
                        if (debug)
                            Print2("PARTIAL: nbuf_write, write %d, requested %d bytes.\n",wc, xfer_size);
		        xfer_results[partition].write_errs++;
                        /*
                         * Free up the buffer for later use.
                         */
                        if (debug > 200) {
                            Print1("Free buffer %d\n",bufnum);
                            flush_output();
                        }
                        write_pending[bufnum] = 0;
                        completed_xfer++;
                        if (completed_xfer >= xfer_count) {
                            writemore = 0;
                        }    
                    }
                    /*
                     * A full transfer has been written.
		     * (wc == xfer_size)
                     */
                    else {
                        /*
                         * Free up the buffer for later use.
                         */
                        completed_xfer++;
                        if (debug > 2) {
                            Print1("%d ",completed_xfer);
                            if ((completed_xfer%20) == 0)
                                Print("\n");
                            flush_output();
                        }
                        write_pending[bufnum] = 0;
                        if (completed_xfer >= xfer_count) {
                            writemore = 0;
                        }    
                        /*
                         * Start up a new write with this
                         * freed buffer.
                         */
                        goto nbufwrite;
                    }
                }
		/*
		 * FIONBDONE returned -1
		 */
                else {
                    if (debug > 5) {
                        Print1("nbuf_write #%d, returned -1\n", bufnum);
                        if (debug > 10)
                            Print1("FIONBDONE error, errno = %d\n",errno);
                    }
		    xfer_results[partition].write_errs++;
                    writemore = 0;	/* Terminate writing */
                    /*
                     * See if the disk went offline.  
                     */
                    if (devio_print(DEVIO_OFFLINE)) {
		        Print("\nERROR: Aborting testing because the disk went offline.\n");
	    		do_errlog(LOG_OFFLINE);
		        exit(1);
		    }
                }
            }		/* End of XFER_IP section 	*/
        }		/* End of for num_nbuf loop 	*/
    }			/* End of while(writemore)	*/
    if (debug > 2) {
        Print("\n");
        flush_output();
    }
    /*
     * Free up allocated buffer space and turn off usage of n-buf.
     */
    nbuf_disable(fd_write, num_nbuf);
}

/*---------------------------------------------------------------------------
 * nbuf_disable
 * Setup for n-buffered I/O.  Declare the number of buffers used and init
 * data structures.
 */
nbuf_setup(fd, n_buffers, action)
    int fd;
    int n_buffers;
    int action;
{
    int zero = 0;

    if (n_buffers < 1) {
        Print("Error: running n-buffered I/O with less than 1 buffer\n");
	do_errlog(LOG_COMMAND);
        exit(1);
    }

    /*
     * Disable n-buffering to flush out any pending operations.
     * I found that this hack is necessary in order to get n-buf I/O to work.
     * You can't just go in and declare that you want to do n-buf using
     * the specified number of buffers.  You first have to "jump start" n-buf
     * by saying that you don't want to do nbuf then you say you do want to
     * do n-buf using the specified number of buffers.
     */
    if (ioctl(fd,FIONBUF,&zero) < 0) {
        Print("nbuf_setup: FIONBUF 0 failed");
	do_errlog(LOG_IOCTLFAIL);
        exit(1);
    }
    /*
     * Declare the number of n-buffers to be used.
     */
    if (ioctl(fd,FIONBUF,&n_buffers) < 0) {
        Print("nbuf_setup: FIONBUF failed");
	do_errlog(LOG_IOCTLFAIL);
        exit(1);
    }
    else if ((debug > 2) && (action == ALLOC_BUFFERS)) {
        Print1("n-buffered I/O enabled with %d buffers\n",n_buffers);
    }
}

/*---------------------------------------------------------------------------
 * nbuf_disable
 *
 * Turn off n-buffered I/O.
 */
nbuf_disable(fd, n_buffers)
    int fd;
    int n_buffers;
{
    int zero = 0;
    int i;
    /*
     * Disable n-buffering to flush out any pending operations.
     */
    if (ioctl(fd,FIONBUF,&zero) < 0) {
        Print("nbuf_disable: FIONBUF 0 failed");
	do_errlog(LOG_IOCTLFAIL);
        exit(1);
    }
    /*
     * Free associated data structs.
     */
    for (i = 0; i < n_buffers; i++) {
        free(nbuf_area[i].buffer);
    }
    free(nbuf_area);
}
#else
   /* These are dummy routine headers so we can compile in TIN */
nbuf_read(fd_read, validate, max_seek, xfer_count, partition)
    int fd_read;			/* File descriptor of disk      */
    int validate;			/* Specifies validate status    */
    int max_seek;			/* Max seek multiple on partition */
    register int xfer_count;		/* Number of reads to do	*/
    register int partition;		/* Which partition is testing   */
{}
nbuf_write(fd_write, validate, max_seek, xfer_count, partition)
    int fd_write;			/* File descriptor of disk      */
    int validate;			/* Specifies validate status    */
    int max_seek;			/* Max seek multiple on partition */
    register int xfer_count;		/* Number of writes to do	*/
    register int partition;		/* Which partition is testing   */
{}
#endif  /* TIN_NO_NBUF */

/*---------------------------------------------------------------------------
 * print_xfers
 *
 * Display the results of transfer testing.  This involves displaying the
 * contents of the xfer_results array.
 */
print_xfers()
{
    int i;

    Print("\n");
    Print("Disk Transfer Statistics\n");
    Print("\n");
    Print("Part ");
    Print("Seeks  ");
    Print("Seek_Er   ");
    Print("Writes ");
    Print("Writ_Er  ");
    Print("MB_Write   ");
    Print("Reads ");
    Print("Read_Er  ");
    Print("MB_Read  ");
    Print("Data_Er");
    Print("\n");

    for (i = 0; i < MAX_PART; i++) {
	Print1("%c",i + 'a');
	Print1("%9d", xfer_results[i].seeks);
	Print1("%9d", xfer_results[i].seek_errs);
	Print1("%9d", xfer_results[i].writes);
	Print1("%8d", xfer_results[i].write_errs);
	Print1("%10.1f", xfer_results[i].write_bytes/(1024*1024));
	Print1("%8d", xfer_results[i].reads);
	Print1("%8d", xfer_results[i].read_errs);
	Print1("%9.1f", xfer_results[i].read_bytes/(1024*1024));
	Print1("%9d", xfer_results[i].val_errs);
	Print("\n");
    }

    Print("\n");
}

/*---------------------------------------------------------------------------
 * init_results
 *
 * Initialize the results array to 0.  This is done when the exerciser is
 * first started and also when more than one test is run to allow re-usage
 * of the data structure.
 */
int
init_results()
{
    bzero(xfer_results, sizeof(RESULTS) * MAX_PART);
}

/*---------------------------------------------------------------------------
 * signal_diskx
 *
 * This routine is called in response to the receipt of a kill or interrupt
 * signal.  Print out test results and quit.
 */
int
signal_diskx()
{
    TEST	*a_test;

    Print("\n");
    Print("Stopping testing due to receipt of a termination signal.\n");
    Print("\n");
    for (a_test = testmatrix; a_test->option; a_test++) {
	if (a_test->state & TEST_IP) {
	    a_test->test_signal();
	}
    }
    print_line();
    print_time();
    Print("Terminating disk exerciser.\n");
    do_errlog(LOG_SIGNAL);
    exit(0);
}

/*---------------------------------------------------------------------------
 * no_sig_handler
 *
 * This routine is called when a signal has been received and the specific 
 * test does not have a routine to print out intermediate results.  This will
 * typically be the case for tests that do not have meaningful intermediate
 * results or the test itself is very short and is not expected to be 
 * interrupted.
 */
int
no_sig_handler()
{
    if (debug) {
	Print("This test does not have any intermediate results to display\n");
	Print("in response to an interrupt signal.\n");
    }
}

/*---------------------------------------------------------------------------
 * help_message
 *
 * Prints out a help message.  
 */
int
help_message()
{
Print("\n\ndiskx - disk exerciser program\n\n");
Print("This program is intended to test various aspects of disk driver functionality.\n");
Print("These tests provide more comprehensive functional coverage than the\n");
Print("dskx(8) utility.  ");
Print("Main functional areas which are tested include:\n\n");
Print("\t* Read testing.\n");
Print("\t* Write testing.\n");
Print("\t* Seek testing.\n");
Print("\t* Performance analysis.\n");
Print("\t* Disktab entry verification.\n");
Print("\n");
Print("Some of the tests involve writing to the disk.  For this reason\n");
Print("the exerciser should be used with caution on disks which may contain\n");
Print("useful data which could be over-written.  Tests which write to the\n");
Print("disk will first check for the existence of file systems on the test\n");
Print("partitions and partitions which overlap the test partitions.  If a\n");
Print("file system is found on these partitions, a prompt will appear \n");
Print("asking if testing should continue.\n");
Print("\n");
Print("There are a number of options that diskx accepts.  These\n");
Print("options control which tests are performed and also specify\n");
Print("parameters of test operation.\n");
Print("\n");
Print("\nThe following is a description of the available options:\n");
Print("\n");
Print("\nThe following options specify tests to be run.\n");
Print("\n");
Print("-d\tDisktab entry test.  The disktab(5) entry for the test disk\n");
Print("\tis obtained by using the getdiskbyname(3x) library routine.\n");
Print("\tAnother disktab(5) entry is dynamically generated using the\n");
Print("\tcreatediskbyname(3x) library routine.  These two disktab\n");
Print("\tstructures are then compared.  This test will only work if the\n");
Print("\tspecified disk is a character special file.\n");
Print("\n");
Print("-h\tHelp.\tDisplays a help message describing test options and\n");
Print("\tfunctionality.\n");
Print("\n");
Print("-p\tPerformance test.  Read and write transfers will be timed to\n");
Print("\tmeasure device throughput.  No data validation is performed as\n");
Print("\tpart of this test.  Testing will be done using a range of transfer\n");
Print("\tsizes if the -F option is not specified.  The transfer size used\n");
Print("\tfor testing a range of record sizes will start at the minimum value\n")
Print("\tand be incremented by the reciprocal of the number of specified\n");
Print("\t\"splits\".  For example if the number of splits is set to 10, the\n");
Print("\ttransfer size will start at the minimum value, the next transfer\n");
Print("\tsize will be the minimum value added to 1/10th of the range of\n");
Print("\tvalues, similarly the next transfer size will increase by 1/10th of\n")
Print("\tthe range during each testing interval.  If a specific number of\n");
Print("\ttransfers are not specified, the transfer count will be set to\n");
Print("\tallow the entire partition to be read or written; this number will\n");
Print("\tvary depending on the transfer size and the partition size. \n"); 
Print("\tThe performance test will run until completed or interrupted\n");
Print("\tand is not time limited by the -minutes parameter.  This test may\n");
Print("\ttake a long time to complete depending on the test parameters.\n");
Print("\n");
Print("\tIn order to acheive maximum throughput the -S attribute should\n");
Print("\tbe specified on the command line.  This will cause sequential\n");
Print("\ttransfers.  If the sequential attribute is not specified, transfers\n")
Print("\twill be done to random locations.  This may slow down the observed\n");
Print("\tthroughput due to associated head seeks on the device.\n");
Print("\n");
#ifndef TIN_NO_NBUF
Print("\tIn order to acheive maximum throughput when testing the character\n");
Print("\tspecial file, n-buffered I/O should be enabled.  For this reason\n");
Print("\tfor better measurements the -n attribute should not be used when\n");
Print("\ttesting the character special file.  On the other hand since\n"); 
Print("\tn-buffered I/O is not supported on the block special file the -n\n");
Print("\tattribute must be specified when testing the block special file.\n");
Print("\n");
#endif
Print("-r\tRead-only test.  Reads from the specified partitions.\n");
Print("\tTo run this test on the block special file the -n attribute must\n");
Print("\tbe specified on the command line.\n");
Print("\n");
Print("\tThis test is useful for generating system I/O activity.  Because \n");
Print("\tit is a read-only test it is possible to run more than one instance\n");
Print("\tof the exerciser on the same disk.\n");
Print("\n");
Print("-w\tWrite test.  The purpose of this test is to verify that data can\n");
Print("\tbe written to the disk and read back for validation.  Seeks are\n");
Print("\talso done as part of this test.  This test provides the most\n");
Print("\tcomprehensive coverage of disk transfer functionality due to\n");
Print("\tthe usage of reads, write, and seeks.  The test also combines\n");
Print("\tsequential and random access patterns.\n");
Print("\n");
Print("\tThe test performs the following operations using a range of\n");
Print("\ttransfer sizes.  \(A single transfer size will be utilized if the\n");
Print("\t-F attribute is specified.\)  The first step is to sequentially\n");
Print("\twrite the entire test partition \(unless the number of transfers\n");
Print("\thas been specified using -num_xfer\).  Next the test partition\n");
Print("\tis sequentially read.  The data read from disk is examined to\n");
Print("\tinsure that it is the same as what was originaly written.  At \n");
Print("\tthis point if random transfer testing has not been disabled\n");
Print("\t\(using the -S attribute\) then writes will be issued to randon\n");
Print("\tlocations on the partition.  After completion of the random writes,\n")
Print("\treads will be issued to random locations on the partition.  The\n");
Print("\tdata read in from random locations will be examined for validity.\n");
Print("\n");
Print("The following options are testing attributes which modify how\n");
Print("tests are to be run.\n");
Print("\n");
Print("-F\tPerform fixed size transfers.  If this option is not specified\n");
Print("\ttransfers will be done using random sizes.\n");
Print("\tThis attribute is associated with the following tests: -p, -r,  -w.\n");
Print("\n");
Print("-i\tInteractive mode.  Under this mode the user will be prompted\n");
Print("\tfor various test parameters.  Typical parameters include the \n");
Print("\ttransfer size and the number of transfers.\n");
Print("\tThe following scaling factors are allowed:\n");
Print("\t\tk or K\tkilobyte (1024 * n)\n");
Print("\t\tb or B\tblock (512 * n)\n");
Print("\t\tm or M\tmegabyte (1024 * 1024 * n)\n");
Print("\tFor example 10k would specify 10240 bytes.\n");
Print("\n");
#ifndef TIN_NO_NBUF
Print("-n\tDisable usage of n-buffered transfers.\n");
Print("\tThis attribute is associated with the following tests: -r, -w, -p.\n");
Print("\tThe default mode of operation is to enable the usage of n-buffered\n");
Print("\tI/O.  Usage of n-buffered I/O is not supported on the block special\n")
Print("\tfile.  For this reason it is required that the -n attribute be\n");
Print("\tspecified on the command line when requesting testing on the block\n");
Print("\tspecial file involves transfer operations \(reads or writes\).\n");
Print("\n");
#endif
Print("-Q\tDo not perform performance analysis of read transfers.  This\n");
Print("\twill cause only write performance testing to be performed.  To\n");
Print("\tperform only read testing and to skip the write performance tests\n");
Print("\tthe -R attribute must be specified.\n");
Print("\tThis attribute is associated with the following tests: -p.\n");
Print("\n");
Print("-R\tOpens the disk in read-only mode.\n");
Print("\tThis attribute is associated with all tests.\n");
Print("\n");
Print("-S\tPerform sequential transfers.  Transfers will be performed\n");
Print("\tto sequential disk locations.  If this option is not specified\n");
Print("\ttransfers will be done to random disk locations.\n");
Print("\tThis attribute is associated with the following tests: -p, -r, -w.\n");
Print("\n");
Print("-T\tDirects output to the terminal.  This attribute is useful when\n");
Print("\toutput is directed to a logfile using the -o option.  By also\n");
Print("\tspecifying this parameter after the -o filename options will cause\n");
Print("\toutput to be directed to both the termanal and the log file.\n");
Print("\tThis attribute is associated with all tests.\n");
Print("\n");
Print("-Y\tIf any of the selected tests write to the disk, the disk will\n");
Print("\tbe examined for any existing file systems.  If it appears that file\n")
Print("\tsystems exist, the exerciser will prompt for confirmation before\n");
Print("\tproceeding.  When this attribute is specified the exerciser will\n");
Print("\tNOT prompt for confirmation before proceeding.\n");
Print("\n");
Print("The following options are used to specify test parameters.  These\n");
Print("options are followed by an associated parameter specification.\n");
Print("Test parameters may also be modified in an interactive manner.\n");
Print("Refer to the description of the \"-i\" test attribute for details.\n");
Print("To specify a numerical value, type the parameter name followed by a \n");
Print("space and then the number.  For example \"-perf_min 512\".\n");
Print("\tThe following scaling factors are allowed:\n");
Print("\t\tk or K\tkilobyte (1024 * n)\n");
Print("\t\tb or B\tblock (512 * n)\n");
Print("\t\tm or M\tmegabyte (1024 * 1024 * n)\n");
Print("\tFor example 10k would specify 10240 bytes.\n");
Print("\nAs an example, -perf_min 10K, causes transfers to be done in\n");
Print("sizes of 10240 bytes.\n");
Print("\n");
Print("-debug\tSpecifies the level of diagnostic output to display.  The\n");
Print("\thigher this number is, a greater volume of output will be produced\n");
Print("\tdescribing the operations the exerciser is performing.\n");
Print("\tThis parameter is associated with all tests.\n");
Print("\n");
Print("-err_lines  Specifies the maximum number of error messages may be\n");
Print("\tproduced as a result of an individual test.  Limits on error output\n")
Print("\tis done to prevent a flooding of diagnostic messages in the event\n");
Print("\tof persistent errors.\n");
Print("\tThis parameter is associated with all tests.\n");
Print("\n");
Print("-f devname  Used to specify the device special file to perform\n");
Print("\ttesting on.  The devname parameter is the name associated with \n");
Print("\teither a block or character special file which represents the disk\n");
Print("\tto be tested.  The file name must begin with an \"r\" as in ra0 or\n");
Print("\trz1.  The last character of the file name may represent the disk\n");
Print("\tpartition to test, if no partitions are specified it is assumed\n");
Print("\tthat testing is to be done to all partitions.  For example if the\n");
Print("\tspecified devname is /dev/rra0 then testing will be done to all\n");
Print("\tpartitions.  If devname is /dev/rra0a then testing will be done\n");
Print("\ton only the \"a\" partition.  This parameter must be specified\n");
Print("\tin order to allow any testing to be performed.\n");
Print("\tThis parameter is associated with all tests.\n");
Print("\n");
Print("-minutes  Specifies how many minutes to allow testing to continue.\n");
Print("\tThis parameter is associated with the following tests: -r, -w.\n");
Print("\n");
Print("-max_xfer  Specifies the maximum transfer size to be performed.\n");
Print("\tWhen transfers are to be done using random sizes, the sizes will\n");
Print("\tbe within the range specified by the -max_xfer and -min_xfer\n");
Print("\tparameters.  If fixed size transfers are specified \(see the -F\n");
Print("\ttest attribute\) then transfers will be done in a size specified\n");
Print("\tby the -min_xfer parameter.  Transfer sizes to the character\n");
Print1("\tspecial file should be specified in multiples of size %d\n",DEV_BSIZE)
Print("\tbytes.  If the specified transfer size is not an even multiple\n");
Print1("\tthe value will be rounded down to the nearest %d bytes.\n",DEV_BSIZE);
Print("\tThis parameter is associated with the following tests: -r, -w.\n");
Print("\n");
Print("-min_xfer  Specifies the minimum transfer size to be performed.\n");
Print("\tThis parameter is associated with the following tests: -r, -w.\n");
Print("\n");
#ifndef TIN_NO_NBUF
Print("-num_nbuf  Specifies how many buffers to use for n-buffered ");
Print("transfers.\n");
#endif
Print("\tThis parameter is associated with the following tests: -r, -w, -p.\n");
Print("\n");
Print("-num_xfer  Specifies how many transfers to perform before changing\n");
Print("\tthe partition that is currently being tested.  This parameter is\n");
Print("\tonly useful when more than one partition is being tested.  If this\n");
Print("\tparameter is not specified then the number of transfers will be set\n")
Print("\tto be enough to completely cover a partition.\n");
Print("\tThis parameter is associated with the following tests: -r, -w.\n");
Print("\n");
Print("-o filename  Sends output to the specified filename.  The default\n");
Print("\tis to not create an output file and send output to the terminal.\n");
Print("\tThis parameter is associated with all tests.\n");
Print("\n");
Print("-perf_max  Specifies the maximum transfer size to be performed.\n");
Print("\tWhen transfers are to be done using random sizes, the sizes will\n");
Print("\tbe within the range specified by the -perf_min and -perf_max\n");
Print("\tparameters.  If fixed size transfers are specified \(see the -F\n");
Print("\ttest attribute\) then transfers will be done in a size specified\n");
Print("\tby the -perf_min parameter.  \n");
Print("\tThis parameter is associated with the following tests: -p.\n");
Print("\n");
Print("-perf_min  Specifies the minimum transfer size to be performed.\n");
Print("\tThis parameter is associated with the following tests: -p.\n");
Print("\n");
Print("-perf_splits  Specifies how the transfer size will be changed when\n");
Print("\ttesting a range of transfer sizes.  The range of transfer sizes is\n");
Print("\tdivided by perf_splits to obtain a transfer size increment.  For\n");
Print("\texample if perf_splits is set to 10, tests will be run by starting\n");
Print("\twith the minimum transfer size and increasing the size by 1/10th of\n")
Print("\tthe range of values for each test iteration.  The last transfer\n");
Print("\tsize will be set to the specified maximum transfer size.\n");
Print("\tThis parameter is associated with the following tests: -p.\n");
Print("\n");
Print("-perf_xfers  Specifies the number of transfers to be performed in\n");
Print("\tperformance analysis.  If this value is not explicitly specified\n");
Print("\tthe number of transfers will be set equal to the number required\n");
Print("\tto read the entire partition.\n");
Print("\tThis parameter is associated with the following tests: -p.\n");
Print("\n");
Print("Program Output\n");
Print("\n");
Print("The following are descriptions of some of the tables which are\n");
Print("produced by the disk exerciser:\n");
Print("\n");
Print("This is the header used to describe the results of the transfer \n");
Print("tests followed by a description of each column:\n");
Print("Disk Transfer Statistics\n");
Print("\n");
Print("Part Seeks  Seek_Er   Writes Writ_Er  MB_Write   ");
Print("Reads Read_Er  MB_Read  Data_Er\n");
Print("\n");
Print("Part     - A letter used to represent the disk partition.\n");
Print("Seeks    - The number of seek system calls that were issued.\n");
Print("Seek_Er  - The number of seek system calls returning error status.\n")
Print("Writes   - The number of write system calls issued.\n");
Print("Writ_Er  - The number of write system calls returning error status.\n");
Print("MB_Write - The number of megabytes of data written.\n");
Print("Reads    - The number of read system calls issued.\n");
Print("Read_Er  - The number of read system calls returning error status.\n");
Print("MB_Read  - The number of megabytes of data read.\n");
Print("Data_Er  - The number of transfers which had data validation errors.\n");
Print("           This does not specify the number of bytes that were in\n");
Print("           error; rather it specifies that the transfer had at least\n");
Print("           one byte in error.\n");
Print("\n");
Print("This header is used to describe the results of the performance tests.\n")
Print("Performance test results:\n");
Print("\n");
Print("Part-    Transfer  Count of          Read            Write");
Print("             Transfer\n");
Print("ition    Size      Transfers         Rate            Rate");
Print("              Errors\n");
Print("\n");
Print("Partition  - A letter used to represent the disk partition.\n");
Print("Transfer Size - This is the size of the read or write system call.\n");
Print("Count of Transfers - Specifies the number of read or write system calls.\n");
Print("Read Rate  - The observed disk throughput obtained by timing the\n");
Print("             specified number of read system calls.\n");
Print("Write Rate - The observed disk throughput obtained by timing the\n");
Print("             specified number of write system calls.\n");
Print("Transfer Errors - Shows how many read and write system calls resulted\n")
Print("                  in a return value of error status.  If this value \n")
Print("                  is nonzero then the values displayed in the Read\n");
Print("                  and Write Rate columns may not accurately reflect\n");
Print("                  propper device performance.\n");
Print("\n");
Print("Examples\n");
Print("\n");
Print("The following are example command lines with a description of what\n");
Print("the resulting test action will be.\n");
Print("\n");
Print("diskx -f /dev/rra0 -r\n");
Print("\tThe above example will perform read-only testing on the\n");
Print("\tcharacter device special file that rra0 represents.  Since\n");
Print("\tno partition is specified, reading will be done from all\n");
Print("\tpartitions.  The default range of transfer sizes will be used.\n");
Print("\tOutput from the exerciser program will be displayed on the\n");
Print("\tterminal.\n");
Print("\n");
Print("diskx -f /dev/rz0a -o diskx.out -d -debug 10\n");
Print("\tThe disktab test will be run on the \"a\" partition of rz0.\n");
Print("\tProgram output will be logged to the file diskx.out.  The\n");
Print("\tprogram output level is set to 10 which will cause additional\n");
Print("\toutput to be generated.\n");
Print("\n");
#ifndef TIN_NO_NBUF
Print("diskx -f /dev/ra0a -o diskx.out -p -n -S\n");
Print("\tPerformance tests will be run in the \"a\" partition of ra0.\n");
Print("\tProgram output will be logged to the file diskx.out.  The\n");
Print("\t-n parameter will disable the usage on n-buffered I\O.  The\n");
Print("\t-S option will cause sequential transfers for best test results.\n");
Print("\tTesting will be done over the default range of transfer sizes.\n");
Print("\n");
#else
Print("diskx -f /dev/ra0a -o diskx.out -p -S\n");
Print("\tPerformance tests will be run in the \"a\" partition of ra0.\n");
Print("\tProgram output will be logged to the file diskx.out.  The\n");
Print("\t-S option will cause sequential transfers for best test results.\n");
Print("\tTesting will be done over the default range of transfer sizes.\n");
Print("\n");
#endif /* TIN_NO_NBUF */
Print("diskx -f /dev/rra0 -r &; diskx -f /dev/rra1 -r &; diskx -f /dev/rra2 -r &\n");
Print("\tThis command will run the read test on all partitions of the disks.\n");
Print("\tThe disk exerciser is being involked here as 3 separate\n");
Print("\tprocesses.  In this manner the 3 processes will be generating \n");
Print("\ta lot of system I/O activity.  This may be useful for system \n");
Print("\tstress testing purposes.\n");
Print("\n");
Print("See also: dskx(8), dkio(4), Guide to System Exercisers, \n");
Print("getdiskbyname(3x), creatediskbyname(3x), disktab(5).\n");
Print("\n");
flush_output();
}

/*---------------------------------------------------------------------------
 * Interactive mode.
 *
 * Prompt the user for test paramters.  Such as transfer size and the number
 * of transfers of each transfer size use.
 */
get_test_params()
{
    int do_xfer_test = 0;
    int do_perf_test = 0;
    int do_timer = 0;
    TEST *a_test;

    Print("Interactive mode has been specified.  You will now be asked\n")
    Print("enter the test paramters.  A range of acceptable values is\n");
    Print("provided, followed by the default in [].  Enter only a\n");
    Print("carriage return to select the default value.\n\n");

    /*
     * See if transfer tests have been selected so we know to prompt for the
     * associated parameters.  OK I do admit this is pretty messy.
     */
    for (a_test = testmatrix; a_test->option; a_test++) {
	if ((strcmp(a_test->option, "-r") == 0) ||
	    (strcmp(a_test->option, "-w") == 0)) {
		if (a_test->state & RUN_TEST) {
	          do_xfer_test = 1;
		}
	}
    }
    /*
     * See if performance tests have been selected so we know to prompt for the
     * associated parameters.
     */
    for (a_test = testmatrix; a_test->option; a_test++) {
	if (strcmp(a_test->option, "-p") == 0) {
		if (a_test->state & RUN_TEST) {
	          do_perf_test = 1;
		}
	}
    }
    /*
     * The following parameters are associated with the transfer tests.
     */
    if (do_xfer_test) {
	do_timer = 1;			/* These tests can be timed */
	if (attributes & FIXED_SIZE) {
	    Print("Tests will be done using a single transfer size.\n");
	    Print("Enter the transfer size to be used:\n");
	    min_xfer = get_number(MIN_XFER, MAX_XFER, min_xfer);
	    Print("\n");
	}
	else {
	    Print("Tests will be done using a range of transfer sizes.\n");
	    Print("Enter the minimum transfer size to be used:\n");
	    min_xfer = get_number(MIN_XFER, MAX_XFER, min_xfer);
	    Print("\n");
	    Print("Enter the maximum transfer size to be used:\n");
	    max_xfer = get_number(min_xfer, MAX_XFER, max_xfer);
	    Print("\n");
	}
	if (test_partitions == PART_ALL) {
	    Print("Enter the number of transfers to do before alternating\n");
	    Print("testing to a different partition.  If the default value\n");
	    Print("of 0 is selected then the number of transfers will be\n");
	    Print("automatically set on a per-partition basis to be enough\n");
	    Print("transfers to test the entire partition before switching\n");
	    Print("to a different random partition.\n");
	    num_xfer = get_number(num_xfer, MAX_NUM_XFER, num_xfer);
	    Print("\n");
        }
    }
    /*
     * The following parameters are associated with the performance tests.
     */
    if (do_perf_test) {
	if (attributes & FIXED_SIZE) {
	    Print("Tests will be done using a single transfer size.\n");
	    Print("Enter the transfer size to be used:\n");
	    perf_min_xfer = get_number(MIN_XFER, MAX_XFER, perf_min_xfer);
	    Print("\n");
	}
	else {
	    Print("Tests will be done using a range of transfer sizes.\n");
	    Print("Enter the minimum transfer size to be used:\n");
	    perf_min_xfer = get_number(MIN_XFER, MAX_XFER, perf_min_xfer);
	    Print("\n");
	    Print("Enter the maximum transfer size to be used:\n");
	    perf_max_xfer = get_number(perf_min_xfer, MAX_XFER, perf_max_xfer);
	    Print("\n");
	    Print("Enter the number of transfer sizes to do performance\n");
	    Print("measurements on.  For example if you specify 10 then\n");
	    Print("then the range of transfer sizes will be broken up into\n");
	    Print("1/10th's and testing will be done on these transfer sizes.\n");
	    perf_splits=get_number(MIN_PERF_SPLITS,MAX_PERF_SPLITS,perf_splits);
	    Print("\n");
	}
        Print("Enter the number of transfers to do for each\n");
        Print("transfer size.  If the default value\n");
        Print("of 0 is selected then the number of transfers will be\n");
        Print("automatically set on a per-partition basis to be enough\n");
        Print("transfers to test the entire partition before switching\n");
        Print("to a different transfer size.\n");
        perf_xfers = get_number(perf_xfers, MAX_NUM_XFER, perf_xfers);
        Print("\n");
    }
    if (do_timer) {
	Print("Enter the number of minutes to conduct testing.\n");
	Print("If the default value of 0 is selected testing will\n");
	Print("continue until a kill signal is received.\n");
	test_minutes = get_number(0, MAX_TIMER, 0);
	Print("\n");
    }
}

/*---------------------------------------------------------------------------
 * Get_number
 * Prompt the user for a number in a specified range.
 * Use the def value if only a carriage return (no number) is entered.
 * If the input number is out of range, reprompt the user for another
 * number.
 */
get_number(min,max,def)
	int min, max, def;
{
	int num_scan, user_number;
	char user_input[200];
	char newline_input;

tryagain:
	Print3("range \(%d-%d\), [%d] ",min,max,def);
	fflush(stdout);
	/*
	 * read up to a newline.  The newline remains in the input stream.
	 */
	num_scan = scanf("%[^\n]",user_input);
	/*
	 * remove the newline from the input stream.
	 */
	scanf("%c",&newline_input);
	if (num_scan == 1) {
		num_scan = sscanf(user_input,"%d",&user_number);
		Log_print( user_input );/* echo input to log file if logging */
	}
	if (num_scan != 1) {
		user_number = def;
		Log_print( "\0" );/* echo input to log file if logging */
	}
	/*
	 * A valid number has been entered by the user.  Call convert_value
	 * to handle any factors. (for example 10k, or 5b)
	 */
	else {
		user_number = convert_value(user_input);
	}
	if (user_number < min) {
		Print1("Number must be greater than %d\n", min)
		goto tryagain;
	}
	if (user_number > max) {
		Print1("Number must be less than %d\n", max)
		goto tryagain;
	}
	return(user_number);
}

/*---------------------------------------------------------------------------
 * devio_print
 *
 * Print out selected fields of a devio structure.  The devio ioctl only
 * works on the character special file.
 */
int
devio_print(pr_options)
	int pr_options;
{
    struct devget dev_st;

    if (fd_disk <= 0)
	return(1);
    if (is_block_device) {
	return(0);
    }
    if ((ioctl(fd_disk,DEVIOCGET,&dev_st)) < 0) {
    	Print("DEVIOCGET failed\n")
    	return (0);
    }
    else {
        /*
         * A request has been made to print out the device name.
         */
        if (pr_options == DEVIO_NAME) {
            Print(" (")
            if((strcmp(DEV_UNKNOWN,dev_st.interface) == 0) ||
                (dev_st.ctlr_num == -1)) {
                   Print(" unknown device type ")
            } else {
                   Print2("interface %s, controller #%d, ", dev_st.interface,
                            dev_st.ctlr_num)
            }
            if(!(strcmp(DEV_UNKNOWN,dev_st.device) == 0)) {
                   Print1("%s", dev_st.device)
            }
            Print(")")
        }
	/*
	 * Returns 1 if offline, 0 otherwise.
         */
        if (pr_options == DEVIO_OFFLINE) {
	    if (dev_st.stat & DEV_OFFLINE) {
		return(1);
	    }
	    else {
		return(0);
	    }
        }
	/*
	 * Returns 1 if write protected, 0 otherwise.
         */
        if (pr_options == DEVIO_WRTLCK) {
	    if (dev_st.stat & DEV_WRTLCK) {
		return(1);
	    }
	    else {
		return(0);
	    }
        }
    }
    return(1);
}

/*---------------------------------------------------------------------------
 * print_time
 *
 * Print out the time of day.
 */
print_time()
{
	int datm;

	datm = time(0);
	Print1("%s",ctime(&datm));
}

/*---------------------------------------------------------------------------
 * alloc_buffers
 *
 * Allocate the buffer space needed for data transfer operations.
 *
 * It used to be the case that the buffer spcae was allocated and then
 * freed in each transfer routine.  This could slow down things in the case
 * of testing to multiple partitions using n-buf.  This routine allocates
 * all the space at once.  One thing which could be improved is to only
 * allocate all the space for the n-buf data structs if they are needed.
 */
alloc_buffers()
{
    char *malloc_ret;
    int i;

    /*
     * Malloc space for transfer buffers.  Only allocate space the first
     * time this routine is called.  The "xfer_buf" struct is used for
     * non n-buf xfers, while the "nbuf_area" is used for n-buf xfers.
     */
    if (nbuf_area == 0) {
        if ((xfer_buf = (char *) malloc( MAX_XFER )) < (char *)0) {
            Print("\nalloc_buffers: can\'t malloc xfer buffer\n");
    	    do_errlog(LOG_MALLOC);
            exit(1);
        }
        /*
         * Allocate space for the buffers.
         */
        if ((nbuf_area = (NBUF_STRUCT *) malloc(sizeof(NBUF_STRUCT) * num_nbuf))
	    == 0) {
            Print("alloc_buffers: can\'t malloc nbuf_area\n");
    	    do_errlog(LOG_MALLOC);
            exit(1);
        }
        /*
         * Initilize each data structure.
         */
        for (i = 0; i < num_nbuf; i++) {
            if ((malloc_ret = (char *)malloc(MAX_XFER)) == 0) {
                Print1("\nalloc_buffers: can\'t malloc buffer, i = %d\n", i);
    	        do_errlog(LOG_MALLOC);
                exit(1);
            }
	    /*
	     * There was a release note which stated that xfer buffers must be
	     * longword aligned.  I'm not sure if this is still needed.
	     */
            if ((int)malloc_ret & 0x3) {
                Print("malloc buffer not on longword boundary\n");
    	        do_errlog(LOG_MALLOC);
                exit(1);
            }
            nbuf_area[i].buffer = malloc_ret;
        }
    }
}

/*---------------------------------------------------------------------------
 * performance_test
 *
 * Obtain throughput statistics by timing reads and writes.
 */
int
performance_test()
{
    int part;				/* Partition being tested	*/
    int xfer_increment;			/* Difference between xfer size */
    int blkdev_splits;			/* Max splits to a block dev    */
    int bsize_splits;			/* Max splits to a char dev     */
    int max_splits;			/* Max splits for this device   */
    int min_xfsize;			/* Minimum increment size       */
    int splitno;			/* Loop counter - split number  */
    int numxfer;			/* Number of transfers	        */
    int max_seek;			/* Max # of xfers per partition */
    int save_test_minutes = 0;		/* Save the value of test time  */

    Print("\n");
    Print("Disk Throughput Analysis.\n");
    Print("\n");
    Print("This test will measure the time it takes to transfer a known\n");
    Print("amount of bytes and calculate throughput.\n");
    if (attributes & FIXED_SIZE) {
	Print1("All calculations will be based on a transfer size of %d bytes.\n",perf_min_xfer);
    }
    else {
	Print2("Transfer sizes will be in the range of %d to %d bytes.\n", perf_min_xfer, perf_max_xfer);
	Print1("Tests will be done using %d different transfer sizes.\n",perf_splits+1);
    }
    if (test_partitions == PART_ALL) {
	Print("Tests will be done to all partitions of this disk.\n");
    }
    if (attributes & SEQUENTIAL) {
	Print("Tests will be done on a sequential basis.\n");
	if ((perf_xfers == 0) && (test_partitions == PART_ALL)) {
	    Print("Enough transfers will be issued to entirely cover a partition\n");
	    Print("before alternating testing to a different partition.\n");
	}
	if ((perf_xfers != 0) && (test_partitions == PART_ALL)) {
	    Print1("%d transfers will be done before alternating testing to a\n",perf_xfers);
	    Print("different partition.\n");
	}
    }
    else {
	Print("Transfers will be issued to random locations on the disk.  ");
	Print("To accomplish this\n");
	Print("a seek will be issued before each transfers ");
	Print("to force a transfer to a different\n");
	Print("disk region.\n");
    }
#ifndef TIN_NO_NBUF
    if (attributes & NO_NBUF) {
	Print("This test will not utilize n-buffered I/O.\n");
    }
    else {
	Print1("This test will utilize n-buffered I/O with %d buffers.\n", num_nbuf);
    }
#endif
    /*
     * This test can not be run on a timed basis.  If the user tries to
     * specify a timer save off that value and set test_minutes to zero so that
     * the transfers will go to completion for correct calculations.
     */
    if (test_minutes) {
	save_test_minutes = test_minutes;
	test_minutes = 0;
	Print("The performance test can not be run for a user specified time\n")
	Print("interval.  During this test the user suplied time limit of\n");
	Print1("%d minutes will not be enforced.\n",save_test_minutes);
    }
    Print("\n");
    /*
     * Initialize the storage array which stores the results.
     */
    bzero(perf_results, sizeof(PERFSTRUCT) * MAX_PART * MAX_PERF_SPLITS);
    /*
     * Before transfering to the disk call diskx_fsck to make sure that the
     * partitions specified for testing are readable.  If writing is to be
     * done then make sure the operator is willing to overwrite file systems
     * if they exist.
     */
    if (attributes & OPEN_READONLY) {
        if (attributes & NO_READ_PERF) {
	    Print("User error.  Skipping performance testing because both\n");
	    Print("the -R and -Q attribute has been specified.  This implies\n")
	    Print("that neither read nor write performance tests are to be\n");
	    Print("conducted.\n");
	    Print("\n");
	    return;
	}
        if (diskx_fsck(NO_WRITE) == FAILURE) {
            return(FAILURE);
        }
    }
    else {
        if (diskx_fsck(DO_WRITE) == FAILURE) {
            return(FAILURE);
        }
    }
    /*
     * Now that the disk has been opened see if n-buf is requested to the
     * block special file.
     */
    if ((is_block_device) && ((attributes & NO_NBUF) == 0)) {
        Print("\n");
        Print("NOTE: Usage of n-buffered I/O is not supported on the\n");
        Print("block special file.  To run this test on the block device\n")
        Print("the -n option must be specified on the command line to\n");
	Print("disable the usage of n-buffered I/O.  N-buffered I/O is only\n");
        Print("supported on the character special file.\n")
        Print("\n");
        return;
    }
    if ((is_block_device) == 0) {
	Print("Disk type: ");
	devio_print(DEVIO_NAME);
        Print("\n\n");
    }
    flush_output();
    /*
     * Determine the size of transfers to perform.
     */
    if (attributes & FIXED_SIZE) {
	xfer_increment = 0;
  	perf_splits = 0;
	xfer_size = perf_min_xfer;
    }
    else {
	xfer_increment = (perf_max_xfer - perf_min_xfer) / perf_splits;
    }
    /*
     * Now that the setup has been done commence testing.
     * For each testable partition go through a range of xfer sizes.
     */
    for (part = 0; part < MAX_PART; part++) {
      if (test_partitions & (1 << part)) {
	if (debug > 1) {
	    Print1("Perform performance test on the %c partition.\n",part+'a');
	}
	if (perf_splits) {
	    /*
	     * Since this is the performance tests I'd like to make it as fast
	     * as possible.  To do this make the transfer size be a multiple of
	     * BLKDEV_IOSIZE on the block device and a multiple of DEV_BSIZE for
	     * the character device.  If anyone wants to test with other
	     * multiples it can be done using fixed size transfers.
	     */
	    blkdev_splits = (Pt_tab(part).p_size * DEV_BSIZE) / 
				BLKDEV_IOSIZE;
	    bsize_splits = Pt_tab(part).p_size;
	    if (is_block_device) {
		max_splits = blkdev_splits;
		min_xfsize = BLKDEV_IOSIZE;
	    }
	    else {
		max_splits = bsize_splits;
		min_xfsize = DEV_BSIZE;
	    }
	    if (perf_splits > max_splits) {
		Print2("Changing the number of test splits from %d to %d.\n",perf_splits, max_splits);
		Print("fit within partition size constranits.\n");
		perf_splits = max_splits;
	    }
	    if (perf_splits > MAX_PERF_SPLITS) {
		perf_splits = (MAX_PERF_SPLITS - 1);
	    }
	    xfer_size = perf_min_xfer;
	    if (xfer_size < min_xfsize) {
		xfer_size = min_xfsize;
	    }
	}
	for (splitno = 0; splitno < (perf_splits+1); splitno++) {
	    /*
	     * Determine the number of transfers required.
	     */
	    if (perf_xfers == 0) {
		numxfer = (Pt_tab(part).p_size * DEV_BSIZE) / xfer_size;
	    }
	    else {	/* Fixed num of xfers specified */
		numxfer = perf_xfers;
	        if (numxfer > max_splits) {
	            numxfer = max_splits;
	        }
	    }
	    max_seek = (Pt_tab(part).p_size * DEV_BSIZE) / xfer_size;
	    do_perftest(part, max_seek, numxfer);
	    /*
	     * Calculate the new transfer size.  Make this a multiple of
	     * min_xfsize for best results.  For the last split use the
	     * specified maximum transfer size.
	     */
	    xfer_size = (xfer_size + xfer_increment);
	    if (splitno == (perf_splits-1)) {
		xfer_size = perf_max_xfer;
	    }	
	    xfer_size -= (xfer_size % min_xfsize);
	}
      }
    }
    /* 
     * Restore specified timer value.
     */
    if (save_test_minutes) {
	test_minutes = save_test_minutes;
    }
    /*
     * Display the results of the performance testing.
     */
    print_perf();
}

/*---------------------------------------------------------------------------
 * do_perftest
 *
 * Perform the actual performance test using the specified transfer size.
 * Stash the results away for later display.  This routine assumes that the
 * global variable xfer_size has already been setup with the transfer size.
 */
int
do_perftest(part, max_seek, numxfer)
    int  part;		/* The partition to test */
    int  max_seek;	/* Max # of seeks on partition based on xfer size   */
    int  numxfer;	/* Number of transfers to perform on this partition */
{
    int fd_perf;
    char dsk_name[MAX_DISKNAME];
    char partchar[10];

    int writestart = 0;		/* Variables to store times	    */
    int writestop = 0;
    int writetime = 0;
    int readstart = 0;
    int readstop = 0;
    int readtime = 0;

    int datasize;			/* Throughput calculation vars	    */
    float writerate = 0;
    float readrate = 0;
    int how_to_open;			/* Open mode			    */
    int skipwrite = 0;			/* Skip the write tests	            */

    if (debug) {
    	Print("Performance test parameters:\n");
	Print3("Partition: %c, xfer size: %d, num_xfers: %d\n",part+'a',xfer_size,numxfer);
    }
    /*
     * Open the disk device.
     */
    sprintf(partchar, "%c", 'a' + part);
    strcpy(dsk_name, disk_base);
    strcat(dsk_name, partchar);
    if (attributes & OPEN_READONLY) {
	how_to_open = O_RDONLY;
	skipwrite = 1;
    }
    else {
	how_to_open = O_RDWR;
    }
    if ((fd_perf  = open(dsk_name, how_to_open)) < 0) {
	Print1("Cannot open %s.\n", dsk_name)
	if (debug > 10)
		Print1("Open failed, errno = %d\n",errno);
	return(FAILURE);
    }
    /*
     * Clear the results array to insure that all stats are relevant to this
     * test.
     */
    init_results();
    /*
     * If the NO_READ_PERF attribute is specified then skip the read performance
     * analysis and only perform write testing.
     */
    if ((attributes & NO_READ_PERF) == 0) {
        /*
         * Start read timer.
         */
        readstart = time(0);
        if (attributes & NO_NBUF) {
	    non_nbuf_read(fd_perf, NO_VALIDATE, max_seek, numxfer, part);
        }
        else {
	    nbuf_read(fd_perf, NO_VALIDATE, max_seek, numxfer, part);
        }
        /*
         * Stop read timer.
         */
        readstop = time(0);
    }
    if ((devio_print(DEVIO_WRTLCK)) && ((attributes & OPEN_READONLY) == 0)) {
        Print("\n");
        Print("ERROR: Skipping write testing because the disk is write protected.\n");
        Print("\n");
        skipwrite = 1;
    }
    /*
     * Do a virtual reposition back to the beginning of the disk using lseek.
     * In the event of an error warp ahead to display the results from the
     * read phase of the testing.
     */
    if (lseek(fd_perf, (off_t)0, L_SET) < 0) {
        Print("ERROR: Seek back to 0 failed.\n");
        skipwrite = 1;
    }
    else if (skipwrite == 0) {
        /*
         * Start write timer.
         */
        writestart = time(0);
        if (attributes & NO_NBUF) {
	    non_nbuf_write(fd_perf, NO_VALIDATE, max_seek, numxfer, part);
        }
        else {
	    nbuf_write(fd_perf, NO_VALIDATE, max_seek, numxfer, part);
        }
        /*
         * Stop write timer.
         */
        writestop = time(0);
    }
    /*
     * Do the calculations.
     */
    /* Elapsed time is stop time - starting time. */
    writetime = writestop - writestart;
    readtime = readstop - readstart;

    /* The number of bytes transferred equals the transfer size * the
     * number of transfers.
     */
    datasize = numxfer * xfer_size;

    /* Bandwidth is the number of bytes / the elapsed time. */
    /* Normalize results to KB per second */
    /*
     * If the transfers took less than a second the readtime would be 0.
     * To prevent division by 0 set the rate to 1 .  This is not the best
     * approach, but I don't have much sympathy for measurements of that short
     * a duration.
     */
    if (readtime < 1) {
	readtime = 1;
    }
    readrate = (float) (datasize / readtime);
    if (writetime < 1) {
	writetime = 1;
    }
    writerate = (float) (datasize / writetime);

    /*
     * If the test was not run set the results to 0.
     */
    if (attributes & NO_READ_PERF) {
	readrate = 0;
	readtime = 0;
    }
    if (skipwrite) {
	writerate = 0;
	writetime = 0;
    }
    /*
     * Store results for later display.
     */
    perf_results[perf_number].partition = part + 'a';
    perf_results[perf_number].xfer_size = xfer_size;
    perf_results[perf_number].num_xfers = numxfer;
    perf_results[perf_number].read_rate = readrate;
    perf_results[perf_number].write_rate = writerate;
    perf_results[perf_number].errors = xfer_results[part].read_errs +
	xfer_results[part].write_errs;
    /*
     * Increment the number of performance tests run to specify where the
     * results are to be stored.
     */
    perf_number++;
    if (debug > 5) {
        /*
         * Display results.
         */
        Print("Performance test results:\n");
        Print2("Transfer size = %d bytes, %.2f KB\n",xfer_size,(float) (xfer_size/1024));
        Print1("Number of transfers used = %d\n",numxfer);
        Print1("Number of bytes transferred = %d, ",datasize);
        if (((float) (datasize/1024)) > 1024) {
    	    Print1("%.2f MB\n",(float) (datasize/(1024*1024)));
        }
        else {
    	    Print1("%.2f KB\n",(float) (datasize/1024));
        }
        Print1("Elapsed writing time is %d seconds.\n",writetime);
        Print1("Elapsed reading time is %d seconds.\n",readtime);
        if (writerate > (1024*1024)) {
    	    Print1("Write bandwidth is %.2f MB/second\n",(float) (writerate/(1024*1024)));
        }
        else {
    	    Print1("Write bandwidth is %.2f KB/second\n",(float) (writerate/1024));
        }
        if (readrate > (1024*1024)) {
    	    Print1("Read  bandwidth is %.2f MB/second\n",(float) (readrate/(1024*1024)));
        }
        else {
    	    Print1("Read  bandwidth is %.2f KB/second\n",(float) (readrate/1024));
        }
    }
    close(fd_perf);
    flush_output();
}

/*---------------------------------------------------------------------------
 * print_perf
 *
 * Display the results of the performance testing.
 */
int
print_perf()
{
    int i;
    int maxread = 0;
    int maxwrite = 0;
    float temp;
    char factor[20];

    Print("Performance test results:\n");
    Print("\n");
    Print("Part-    Transfer  Count of          Read            Write");
    Print("             Transfer\n");
    Print("ition    Size      Transfers         Rate            Rate");
    Print("              Errors\n");
    print_line();
    for (i = 0; i < perf_number; i++) {
        Print1("%c     ", perf_results[i].partition);
	temp = (float)perf_results[i].xfer_size;
	if (temp > (1024*1024)) {
	    sprintf(factor, "MB");
	    temp /= (1024*1024);
	}
	else {
	    sprintf(factor, "KB");
	    temp /= 1024;
	}
        Print2("%7.1f %s", temp, factor);
        Print1("%10d        ", perf_results[i].num_xfers);
	temp = (float)perf_results[i].read_rate;
	if (temp > (1024*1024)) {
	    sprintf(factor, "MB/sec");
	    temp /= (1024*1024);
	}
	else {
	    sprintf(factor, "KB/sec");
	    temp /= 1024;
	}
	if (perf_results[i].read_rate > maxread) {
	    maxread = perf_results[i].read_rate;
	}
        Print2("%7.2f %s  ", temp, factor);
	temp = (float)perf_results[i].write_rate;
	if (temp > (1024*1024)) {
	    sprintf(factor, "MB/sec");
	    temp /= (1024*1024);
	}
	else {
	    sprintf(factor, "KB/sec");
	    temp /= 1024;
	}
	if (perf_results[i].write_rate > maxwrite) {
	    maxwrite = perf_results[i].write_rate;
	}
        Print2("%7.2f %s  ", temp, factor);
        Print1("     %7d", perf_results[i].errors);
        Print("\n");
    }
    if (perf_number > 1) {
        Print("\n");
	temp = (float)maxread;
	if (temp > (1024*1024)) {
	    sprintf(factor, "MB/sec");
	    temp /= (1024*1024);
	}
	else {
	    sprintf(factor, "KB/sec");
	    temp /= 1024;
	}
        Print2("The maximum observed read rate is  %7.2f %s.\n",temp, factor);
	temp = (float)maxwrite;
	if (temp > (1024*1024)) {
	    sprintf(factor, "MB/sec");
	    temp /= (1024*1024);
	}
	else {
	    sprintf(factor, "KB/sec");
	    temp /= 1024;
	}
        Print2("The maximum observed write rate is %7.2f %s.\n",temp, factor);
    }
    flush_output();
}

/*---------------------------------------------------------------------------
 * print_line
 *
 * This amazing routine prints a line across the screen.
 * I suppose this should ideally do a TIOCGWINSZ to determine window width
 * instead of assuming the width is TTY_WIDTH.  That will be left as an
 * exercise to the reader.
 */
int
print_line()
{
    int column;

    for (column = 0; column < TTY_WIDTH; column++) {
	Print("-");
    }
    Print("\n");
    flush_output();
}

/*---------------------------------------------------------------------------
 * setup_writebuf
 *
 * Initialize the contents of a write buffer to have known contents.  The
 * contents of a write buffer are based on the location on the disk and the
 * global variable seed.
 *
 */
int
setup_writebuf(seek_location, xfer_size, bufptr)
    off_t seek_location;
    register int xfer_size;
    register char *bufptr;
{
    register int i;
    register int baseval;

    baseval = seed + seek_location;
    for (i = 0; i < xfer_size; i++) {
	bufptr[i] = (baseval + i) & 0x7f;
    }
}

/*
 * Log a start or stop message to the error logger.  It is useful to have
 * this landmark within the error log to see if device errors are generated
 * durnig testing.
 */
do_errlog(logstring)
    char *logstring;
{
    char errbuf[255];	/* Max errbuf size is 256? */

    if (geteuid()) {
        if (debug > 200) {
            EPrint("No error log generated, not root.\n");
        }
    }
    else {
	if (strlen(logstring) > 200) {
	    logstring[200] = '\0';
        }
        sprintf(errbuf,logstring);	
	strcat(errbuf, ": ");
	strcat(errbuf, diskname);
        (void)binlogmsg(ELMSGT_DIAG,errbuf);
    }
}
