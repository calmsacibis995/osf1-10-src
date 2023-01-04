/*
**++
**  COPYRIGHT (c) 1987, 1988, 1989, 1991, 1992 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
**
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
**
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
**
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/


/*
**++
**  MODULE NAME:
**	iprdw.h
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	internal only definitions for PrintScreen DECwindows
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:
**	March 1, 1987 MFA
**
**--
*/


#define ISL
#define PRINTWID

#include "prdw_compat.h"
#include <stdio.h>
#include "prdw.h"
#include "smconstants.h"

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <img/ChfDef.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#ifndef TRUE
#define TRUE 		1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#define BITSINBYTE	8
#define	BITSINTWOBYTES	16
#define	BITSINFOURBYTES	32
#define	BYTESINWORD	4

#ifndef NULL
#define NULL 0
#endif
   
/*
 * Postscript sizes of 8.5x11 piece of paper
 */
#define	PSPAGELEN	792
#define	UPSPAGELEN	756
#define PSPAGEWID	612
#define UPSPAGEWID	576

#define MAX_OUTPUT_BUFFER     256        /* Maximum PS line size  */

/*
 * String definitions
 */
#ifdef VMS
#define DEFAULT_FILENAME    "decw$printscreen.tmp"
#else
#define DEFAULT_FILENAME    "decw_printscreen.tmp"
#endif

/*
 * some typedefs
 */
typedef int bool;

typedef struct
{
	int	first;
	int	last;
	Colormap cmap;
	Visual	*visual;
	struct occinfo *next;
} occinfo;

typedef struct
{
	occinfo	*row;
} poccinfo;

/*
 * some simple functions
 */
#define	ROUNDUP(nbits,bits) ((((nbits) - 1 ) + (bits))/(bits))
#define	RoundUp(nbits,bits) ((((nbits) - 1 ) + (bits))/(bits))
#define EucDis(a, b) ( (a) > (b) ) ? ( (a) - (b) ) : ( (b) - (a) )
#define EUCDIS(a, b) ( (a) > (b) ) ? ( (a) - (b) ) : ( (b) - (a) )

#define DPIof(screen) \
    ((HeightOfScreen(screen) * 25.4 ) / HeightMMOfScreen(screen))

/*
 * error definitions
 */
#define XERROR		dxPrscXError	/* internal X error		    */
#define	FUNERROR 	dxPrscFunError	/* functionality error (limited)    */

typedef struct
{
	long		aspect;
	long		print_color;
	long		reverse_image;
	long		storage_format;
	long		form_feed;
	long		print_widget;
	long		send_to_printer;
	long		ungrab;
	long		fit;
	long		orient;
	long		page_size;
	long		sixel_device;
	long		delay;
	Display		*dpy;
	int		x_coord;
	int		y_coord;
	unsigned int	h_coord;
	unsigned int	w_coord;
	long		time_delay;
	long		partial_capture;
	long		command_mode;
	long		run_mode;
	long		template;
	long		screen;
} dxPrscOptions ;

typedef dxPrscOptions DECW$C_PRSC_OPTIONS;

typedef struct
{
	char	DisplayName[512];
	char	print_dest[MAXFILSTR];
	dxPrscOptions	options;
} Command;

#if defined(_MAIN_MODULE_)
Command command;
#else
extern Command command;
#endif

/*
 * the bad function
 */
#define BAD(status) !((status) & 1)

/*
 * this is just debug info, remove later
 */
#define image_info( image ) \
	fprintf(stderr, "\nImage size is %dx%d\n", image->height, image->width ); \
	fprintf(stderr,"Pixels offset in X directions %d\n", image->xoffset ); \
	fprintf(stderr,"Format is %d\n", image->format ); \
	fprintf(stderr,"Byte order is %d\n", image->byte_order); \
	fprintf(stderr,"Bitmap Unit %d\n", image->bitmap_unit); \
	fprintf(stderr,"Bitmap Bit Order %d\n", image->bitmap_bit_order); \
	fprintf(stderr,"Bitmap Pad %d\n", image->bitmap_pad); \
	fprintf(stderr,"Image depth is %d\n", image->depth); \
	fprintf(stderr,"Bytes per line %d\n", image->bytes_per_line ); \
	fprintf(stderr,"Bits per pixel is %d\n", image->bits_per_pixel )


/*
 * external subroutines
 */
XImage		*XGetImage();
Window		XWindow();
/* char		*malloc(); */

/*
 * DEBUG constants, data structures, and macros
 */

#define DbgOn		0x00000001
#define DbgMsgOnly	0x00000002
#define DbgProcStatic	0x00000004
#define DbgIslMeter	0x00000008
#define DbgOverride     0x80000000

extern unsigned int global_debug_flag;

extern int clock_time;

extern char tempFilename[256];

typedef struct {
	unsigned int value;
        unsigned int length;
	} component;

typedef struct {
	unsigned char value[32];
        unsigned int length;
        } string_component;

typedef struct {
	component		cputim;
	component		dfpfc;
	component		dfwscnt;
	component		freptecnt;
	component		jobtype;
	component		master_pid;
	component		mode;
	component		owner;
	component		pageflts;
	component		pagfilcnt;
	component		pgflquota;
	component		pid;
	component		ppgcnt;
	string_component	prcnam;
	component		pri;
	component		uic;
	string_component	username;
	component		virtpeak;
	component		wsauth;
	component		wsauthext;
	component		wsextent;
	component		wspeak;
	component		wsquota;
	component		wssize;
	string_component	jobtype_string;
	string_component	mode_string;
	} jpi;

#define Dprintf( message, flags)\
 if (global_debug_flag) { \
    printf("\n");\
    printf message;\
    printf("\n");\
    PrintDebugInfo(flags);\
 }
