/* #module DT_regis.c "X0.0" */
/*
 *  Title:	DT_regis.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1988, 1993                                                 |
 *  | By Digital Equipment Corporation, Maynard, Mass.                       |
 *  | All Rights Reserved.                                                   |
 *  |                                                                        |
 *  | This software is furnished under a license and may be used and  copied |
 *  | only  in  accordance  with  the  terms  of  such  license and with the |
 *  | inclusion of the above copyright notice.  This software or  any  other |
 *  | copies  thereof may not be provided or otherwise made available to any |
 *  | other person.  No title to and ownership of  the  software  is  hereby |
 *  | transfered.                                                            |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	Bob Messenger
 *
 *  Modification history:
 *
 * Alfred von Campe     18-Dec-1993     BL-E
 *	- Change all occurrances of common.foreground to manager.foreground.
 *
 * Alfred von Campe     15-Oct-1993     BL-E
 *      - Change TCALL to only check for errno if thread routine failed.
 *
 * Eric Osman		23-Aug-1993	BL-D
 *	- In regis_destroy, fetch rs from widget, rather than depending on
 *	  global rstruct.
 *
 * Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     03-May-1993     V1.2/BL2
 *      - Casts to satisfy OSF/1 compiler.
 *
 * Alfred von Campe     25-Mar-1993     V1.2/BL2
 *      - OSF/1 code merge.
 *
 * Aston Chan		19-Nov-1992	Post Alpha SSB
 *	- Fix the upside down problem.  In WVT$START_REGIS(), don't always
 *	  call thread.  Change to see if it is called first.
 *
 * Alfred von Campe     08-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Eric Osman		21-Jul-1992	 VXT V12
 *	- Don't hang if "clear comm" during regis pause.  Fix by clearing
 *	  pause in WVT$CLEAR_REGIS.
 *
 * Eric Osman           11-June-1992    V3.2
 *      - Use LWP (light weight process) on Sun systems, for regis
 *	  communication.
 *
 * Eric Osman		11-Mar-1992	V3.2
 *	- Use threads instead of SUBREGIS module for non Alpha/OpenVms
 *	  environment. VAX/VMS doesn't have threads yet (until all customers
 *	  have version 5.5 which is guaranteed to have threads).
 *
 * Alfred von Campe     20-Feb-1992     V3.1
 *      - Add color text support.
 *
 * Aston Chan		19-Dec-1991	V3.1
 *	- I18n code merge
 *
 * Eric Osman		24-Oct-1991	V3.1
 *	- initialize common.free_color_list to NULL.
 * Aston Chan		1-Sep-1991	Alpha
 *	- noshare is duplicated in externaldef macro.  Complained by DECC.
 *	- Solve type casting problem.  Complained by DECC.
 *
 * Eric Osman		22-Mar-1991	V3.0
 *	- Fix crash that occurred when number of bitplanes was changed
 *	  from 2 to 4, and shared-colors also being requested simultaneously.
 *	  Problem was that WVT$RESET_COLORMAP was being called with *new*
 *	  number of planes, but only old number of planes was still allocated.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	17-Jul-1990	X3.0-5
 *	- Merge in Toshi Tanimoto's changes to support Adian terminals -
 *	- use different cell height/width in Kanji ReGIS mode
 *	- regis_set_value_regisScreenMode()
 *
 * Bill Matthews	June 1990
 *	- Make color_name readwrite because it contains address data
 *	  Make color_name globaldef instead of extern
 *
 * Bob Messenger	 5-Aug-1989	X2.0-18
 *	- Switch dynamically between ALLOCATED_PLANES and ALLOCATED_COLORMAP.
 *
 * Bob Messenger	 4-Aug-1989	X2.0-17
 *	- Initialize text_background_index when screenMode is False.
 *
 * Bob Messenger	31-Jul-1989	X2.0-17
 *	- If bitPlanes is set to 0 always set it to the device-specific
 *	  default, even if the display has already been realized (fixes the
 *	  problem with Use System Defaults crashing DECterm).
 *	- Initialize backing_store_active to false (prevents TPU crash).
 *
 * Bob Messenger	27-May-1989	X2.0-13
 *	- No need to adjust clipping rectangle (will be adjusted later when
 *	  addressing changes).
 *
 * Bob Messenger	19-May-1989	X2.0-11
 *	- Fix Ultrix compilation warnings.
 *
 * Bob Messenger	15-May-1989	X2.0-10
 *	- Minor edits (no functional changes).
 *
 * Bob Messenger	28-Apr-1989	X2.0-8
 *	- Call s_clear_display instead of DECwTermClearDisplay, so erased
 *	  lines will be scrolled into transcript.
 *
 * Bob Messenger	11-Apr-1989	X2.0-6
 *	- Allocate space for the colormap arrays in regis_setValueBitPlanes and
 *	  free them in regis_destroy, to support DECCTR (color table report).
 *	- Call allocate_color_map instead of init_color_map.
 *
 * Bob Messenger	 7-Apr-1989	X2.0-6
 *	- Convert to new widget bindings (DECwTerm instead of DwtDECterm).
 *	- Declare color_names as readonly and rstruct as noshare, so the
 *	  DECterm widget can go in shareable library.
 *
 * Bob Messenger	 5-Apr-1989	X2.0-6
 *	- Support variable number of bit planes
 *
 * Bob Messenger	 4-Apr-1989	X2.0-5
 *	- Support backing store enable
 *
 * Bob Messenger	31-Mar-1989	X2.0-5
 *	- Support shared colormap entries.
 *
 * Bob Messenger	15-Mar-1989	X2.0-3
 *	- Added stubs for regis_set_value_transcriptSize,
 *	  regis_set_value_shareColormap, regis_set_value_bitPlanes, and
 *	  regis_set_value_backingStore
 *
 * Bob Messenger	24-Feb-1989	V1.1-1
 *	- Add conditional compilation for MIPS
 *
 * Bob Messenger	30-Jan-1989	X1.1-1
 *	- disable multiple input mode on exit.  Don't enable or disable the
 *	  button handler unless exiting multiple input mode. Added
 *	  WVT$EXIT_REGIS, which should be called whenever exiting or aborting
 *	  ReGIS mode (e.g. during a hard reset).
 *
 * Bob Messenger	17-Jan-1989	X1.1-1
 *	- moved many ld fields to common area
 *
 * Eric Osman		03-Oct-1988	BL10.2
 *	- initialize graphics_visible flag
 *
 * Tom Porcher		21-Apr-1988	X0.4-10
 *	- Split regis_create() into regis_initialize() and regis_realize().
 *
 */


/*

dt_regis.c - code to invoke DECterm ReGIS interpreter

Robert Messenger
October 5th, 1987

*/

#include <stdio.h>
#include "regstruct.h"  /* regstruct.h includes pthread.h which */
#include <errno.h>      /* must be included before errno.h */
#include "wv_hdr.h"

#if defined(VMS_DECTERM)
    noshare
#endif
	char *color_name[] = { "black", "red", "green", "yellow",
			    "blue", "magenta", "cyan", "white" };

#ifdef USE_PTHREADS

#if defined (VMS_DECTERM) || defined(VXT_DECTERM)
noshare
#endif
pthread_mutex_t printf_mutex;
#if defined (VMS_DECTERM) || defined(VXT_DECTERM)
noshare
#endif
printf_mutex_flag = 0;

/*
 * Macros for thread interface.
 */
#define TCALL(a_c_t_i_o_n)					\
    {								\
/*								\
    pthread_mutex_lock (&printf_mutex);				\
    printf ("Thread %d doing a_c_t_i_o_n\n", pthread_self());	\
    fflush (stdout);						\
								\
    pthread_mutex_unlock (&printf_mutex);			\
*/								\
    if ((a_c_t_i_o_n) == -1)					\
	{							\
	pthread_mutex_lock (&printf_mutex);			\
	printf ("a_c_t_i_o_n failed, errno = %d\n", errno);	\
	fflush (stdout);					\
	pthread_mutex_unlock (&printf_mutex);			\
	}							\
    }
#define TLOCK TCALL (pthread_mutex_lock (&rstruct->mutex))
#define TUNLOCK TCALL (pthread_mutex_unlock (&rstruct->mutex))
#define TRING(bell) TCALL (pthread_cond_broadcast (bell))
#define TWAIT(bell) TCALL (pthread_cond_wait (bell, &rstruct->mutex))

#endif USE_PTHREADS

#ifdef USE_LWP
#include <lwp/stackdep.h>
#include <lwp/lwpmachdep.h>
#define LWP_CALL(a_c_t_i_o_n)					\
    {								\
    thread_t self;					        \
    int status;                                                 \
/*								\
    lwp_self(&self);                                            \
    printf ("Thread %d doing a_c_t_i_o_n\n", self);	        \
    fflush (stdout);						\
*/								\
    status = (a_c_t_i_o_n);	       				\
    if (status == -1)						\
	{							\
        lwp_perror ("a_c_t_i_o_n failed");	                \
	}							\
    }
#endif

/*ALPHA  ifdef VMS, then externaldef() macro has been defined noshare in
	 intrinsic.h already.  No need to have noshare here!!
*/
#if !defined(ALPHA)
#ifdef VMS_DECTERM
noshare
#endif
#endif
externaldef(regis) struct regis_cntx *rstruct;
					/* ReGIS context pointer */

extern void new_regis(), regis();

#ifdef USE_LWP

/*
 * The lwp package seems to replace XtCalloc with one that gets too upset
 * about requests for 0-byte blocks.  The following replacement seems to make
 * the world a happier place for you and your children.
 */
char * XtCalloc (a,b)
{
char *mem;
int i,len;
len = a*b;
if (!len) len = 1;
mem = (char *) malloc (len);
if (!mem)
    {
    return mem;
    }
for (i=0; i<len; i++) mem[i] = 0;
return mem;
}

#endif

/* regis_initialize - called from dt_control when widget is created */

void regis_initialize( w )
    DECtermData *w;
{
    Display *display;
    int screen;

    rstruct =
      ( struct regis_cntx * ) XtCalloc( sizeof(struct regis_cntx), 1 );
    w->regis = ( char * ) rstruct;
    rstruct->widget = w;

#ifndef USE_REGIS_ASSEMBLY_CODE
    rstruct->thread_exists_flag = 0;
#endif

#ifdef USE_LWP
    LWP_CALL (lwp_self (&rstruct->main_thread));
    LWP_CALL (lwp_setstkcache (MINSTACKSZ*sizeof(stkalign_t), 1));
#endif

#ifdef USE_PTHREADS
    if (!printf_mutex_flag)
	{
	pthread_mutex_init (&printf_mutex, pthread_mutexattr_default);
	printf_mutex_flag = 1;
	}
#endif

    rstruct->defer_count = 0;
    rstruct->initialized = FALSE;
    rstruct->input_mode = 0;
    rstruct->paused = FALSE;
    rstruct->timer = NULL;
    rstruct->bs_pixmap = NULL;
    rstruct->bs_gc = NULL;
    rstruct->bs_text_gc = NULL;
    rstruct->bs_text_background_gc = NULL;
    rstruct->bs_shade_gc = NULL;

    if ( w->source.wvt$l_ext_flags & vte1_m_asian_common ) {
	rstruct->kanji_regis = True;
	rstruct->max1_alph_cell_height = MAX1_ALPH_WIDE_CELL_HEIGHT;
	rstruct->max2_alph_cell_width = MAX2_ALPH_WIDE_CELL_WIDTH;
    } else {
	rstruct->kanji_regis = False;
	rstruct->max1_alph_cell_height = MAX1_ALPH_CELL_HEIGHT;
	rstruct->max2_alph_cell_width = MAX2_ALPH_CELL_WIDTH;
    }

    w->common.color_map_allocated = FALSE;
    w->common.graphics_visible = FALSE;
    w->common.backing_store_active = FALSE;
    w->common.color_map = NULL;	/* indicates not allocated */
    w->common.free_color_list = NULL;

    display = XtDisplay(w);
    screen = DefaultScreen( display );

    w->common.hardware_planes = DisplayPlanes ( display, screen );
    w->common.default_colormap = DefaultColormap ( display, screen );
    w->common.visual = DefaultVisual( display, screen );
    w->common.black_pixel = BlackPixel( display, screen );
    w->common.white_pixel = WhitePixel( display, screen );

}

void regis_realize( w )
    DECtermData *w;
{
    Visual *visual;
    XColor dummy_color;
    int color, num_colors;

    rstruct = ( struct regis_cntx * ) w->regis;

    switch ( w->common.visual->class )
	{
	case StaticColor:
	case TrueColor:
	case DirectColor:
	case StaticGray:
	case GrayScale:
	    w->common.shareColormapEntries = True;
	    break;
	}

    if ( w->common.hardware_planes == 1 )
	w->common.graphics_mode = SINGLE_PLANE;
    else if ( w->common.shareColormapEntries )
	w->common.graphics_mode = ALLOCATED_COLORS;
    else
	w->common.graphics_mode = ALLOCATED_PLANES;
		/* might be ALLOCATED_COLORMAP later if alloc fails */

    rstruct->window_width = 0;	/* no clipping rectangle yet */
    rstruct->window_height = 0;
    rstruct->x_offset = w->common.origin_x + X_MARGIN;
    rstruct->y_offset = w->common.origin_y + Y_MARGIN;
    rstruct->display = XtDisplay(w);
    rstruct->window = XtWindow(w);
    rstruct->num_planes = w->common.hardware_planes;
    visual = w->common.visual;
#if 1
    rstruct->color_monitor = ( visual->class == StaticColor
      || visual->class == TrueColor || visual->class == PseudoColor
      || visual->class == DirectColor );
#else
    rstruct->color_monitor = ( rstruct->num_planes > 4 );
#endif

/* find the best rgb values for each color */

    for ( color = 0; color < 8; color++ )
	{
	XLookupColor( XtDisplay(w), w->core.colormap, color_name[color],
		&w->common.pure_color[ color ], &dummy_color );
	}

    w->common.color_map_allocated = FALSE;

    find_default_foreground( w );
    find_default_background( w );

/* set screen mode according to whether the foreground is darker than the
   background or vice-versa */

    w->common.screenMode = ( w->common.default_foreground_mono <
		w->common.default_background_mono );

    regis_set_value_bitPlanes( w, w );	/* validate the number of planes */

    num_colors = ( 1 << w->common.bitPlanes );
    w->common.color_map = ALLOC_ARRAY( XColor, num_colors );
    w->common.color_map_mono = ALLOC_ARRAY( unsigned long, num_colors );
    w->common.pixel_valid = ALLOC_ARRAY( Boolean, num_colors );
    w->common.pixel_allocated = ALLOC_ARRAY( Boolean, num_colors );
    w->common.plane_masks = ALLOC_ARRAY( unsigned long, w->common.bitPlanes );

/* set the foreground and background pixel values in the color map array */

    w->common.color_map[ w->common.text_foreground_index ].pixel =
                w->manager.foreground;
    w->common.color_map[ w->common.text_background_index ].pixel =
                w->core.background_pixel;

    WVT$RESET_COLORMAP( w );

    regis_create_display(w, NULL, NULL);

}

/* regis_destroy - called from dt_control when widget is destroyed */

void regis_destroy( w )
    DECtermData *w;
{
    rstruct = ( struct regis_cntx * ) w->regis;   /* subregis uses rstruct */

#ifdef USE_PTHREADS
    if (rstruct->thread_exists_flag)
	{
	pthread_addr_t status;
	subregis (0);
	TCALL (pthread_join (rstruct->thread, &status));
	TCALL (pthread_detach (&rstruct->thread));
	TCALL (pthread_mutex_destroy (&rstruct->mutex));
	TCALL (pthread_cond_destroy (&rstruct->bell));
	rstruct->thread_exists_flag = 0;
	}
#endif

#ifdef USE_LWP
    if (rstruct->thread_exists_flag)
	{
	LWP_CALL (lwp_destroy (rstruct->thread));
	rstruct->thread_exists_flag = 0;
	}
#endif

    if (rstruct->timer)
	{
	XtRemoveTimeOut (rstruct->timer);
	rstruct->timer = NULL;
	}

    if ( w->regis != NULL )
	{
	regis_destroy_display(w);
	XtFree( w->regis );
	w->regis = NULL;
	}

/*
 * If the color map arrays were previously allocated, free them
 */
    if ( w->common.color_map )
	{
	XtFree( (char *)w->common.color_map );
	XtFree( (char *)w->common.color_map_mono );
	XtFree( (char *)w->common.pixel_valid );
	XtFree( (char *)w->common.pixel_allocated );
	XtFree( (char *)w->common.plane_masks );
	}
}

#ifndef USE_REGIS_ASSEMBLY_CODE
open_regis (){};   /* Previous versions needed special OS-specific open-hook */
#endif

#ifdef USE_PTHREADS

/*
 * subregis is the procedure that switches to the regis thread and waits
 * for it to stop, which it will do as soon as it runs out of characters.
 *
 *	PLEASE NOTE:
 *
 *		For non-thread versions, you need subregis and getl_regis
 *		in assembly-language modules.  See files reg_subregis.mar,
 *		and mips.s for examples.
 */
subregis (data) char *data;
{
if (!rstruct->thread_exists_flag)
    {
    TCALL (pthread_mutex_init (&rstruct->mutex, pthread_mutexattr_default));
    TCALL (pthread_cond_init (&rstruct->bell, pthread_condattr_default));
    }
TLOCK;
if (!rstruct->thread_exists_flag)
    {
    TCALL (pthread_create (&rstruct->thread, pthread_attr_default,
	(pthread_startroutine_t)rstruct->regspc, 0));
    rstruct->thread_exists_flag = 1;
    }
rstruct->bufpos = data;
rstruct->new_data_flag = TRUE;
TRING (&rstruct->bell);
while (rstruct->new_data_flag) TWAIT (&rstruct->bell);
TUNLOCK;
}

/*
 * getl_regis is the procedure that switches back from the regis thread
 * until new regis characters are available, and then returns to the
 * regis thread with the pointer 
 */
char *getl_regis()
{
char *result = rstruct->bufpos;
if (! result)
    {
    TLOCK;
    rstruct->new_data_flag = FALSE;
    TRING (&rstruct->bell);
    while (! rstruct->new_data_flag) TWAIT (&rstruct->bell);
    TUNLOCK;
    }

result = rstruct->bufpos;

/*
 * If after WAITing, the non-regis side feeds us a 0, that's our cue to exit.
 */
if (! result)
    {
    rstruct->new_data_flag = FALSE;
    TRING (&rstruct->bell);
    pthread_exit (0);
    }

rstruct->bufpos = 0;
return result;
}

#endif USE_PTHREADS

#ifdef USE_LWP

subregis (data) char *data;
{
if (!rstruct->thread_exists_flag)
    {
    LWP_CALL (lwp_create (&rstruct->thread, rstruct->regspc, pod_getmaxpri(),
	0, STKTOP(lwp_newstk()), 0));
    rstruct->thread_exists_flag = 1;
    }
rstruct->bufpos = data;
LWP_CALL (lwp_yield (rstruct->thread));
}

char *getl_regis()
{
char *result = rstruct->bufpos;
if (! result)
    {
    LWP_CALL (lwp_yield (rstruct->main_thread));
    }

result = rstruct->bufpos;

rstruct->bufpos = 0;
return result;
}

#endif USE_LWP

/* WVT$CLEAR_REGIS - initialize regis */

WVT$CLEAR_REGIS(ld)
    wvtp ld;
{
    rstruct = ( struct regis_cntx * ) ld->regis;

    if ( _cld wvt$b_in_dcs == 'p' )
	{  /* aborting ReGIS mode */
	WVT$EXIT_REGIS( ld );
	}

#ifdef USE_REGIS_ASSEMBLY_CODE
{
    int temp, temp1;	/* temporary variables to appease the Ultrix compiler */
    temp = (int) ( rstruct->regstack + REGSTKSIZ );
    temp1 = temp;  /* the Ultrix compiler thinks temp is really a pointer! */
    temp1 &= ~3;	/* make the stack pointer longword aligned */
    rstruct->regssp = ( char * ) temp1;
#ifdef mips
    rstruct->regssp -= 36;	/*  need to restore "saved" registers */
#endif
    rstruct->regsfp = 0;
    rstruct->regspc = new_regis;
#ifndef mips
    (char *) rstruct->regspc += 2;
#endif
}
#else /* (put stuff here that's common for both pthreads and lwp) */
    rstruct->regspc = new_regis;
#ifdef USE_PTHREADS
    if (rstruct->thread_exists_flag)
	{
	pthread_addr_t status;
	subregis (0);
	TCALL (pthread_join (rstruct->thread, &status));
	TCALL (pthread_detach (&rstruct->thread));
	TCALL (pthread_mutex_destroy (&rstruct->mutex));
	TCALL (pthread_cond_destroy (&rstruct->bell));
	rstruct->thread_exists_flag = 0;
	}
#else /* (if not assembly or pthreads, assume lwp) */
    if (rstruct->thread_exists_flag)
      {
	LWP_CALL (lwp_destroy (rstruct->thread));
	rstruct->thread_exists_flag = 0;
      }
#endif USE_PTHREADS
#endif USE_REGIS_ASSEMBLY_CODE
    rstruct->widget = ld;
    rstruct->input_mode = 0;
    rstruct->input_pending = FALSE;
}

/* WVT$START_REGIS - start or restart ReGIS */

WVT$START_REGIS( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w( ld );
    int temp, temp1;		/* needed to trick the Ultrix compiler */

    regis_execute_deferred( w );
    regis_set_widget( w );
    if ( _cld wvt$b_regis_mode & 1 )
	{  /* bit 1 of DCS parameter means re-enter at command level */

#ifdef USE_REGIS_ASSEMBLY_CODE
#ifndef mips
	/*ALPHA cast it to (char *) for picky  DEC C compiler
	 */
	char *new_regis_addr = (char *) new_regis;

	if ( rstruct->regspc != (void *)(new_regis_addr + 2) )
#else
	if ( rstruct->regspc != new_regis)
#endif
	    {  /* make sure we've initialized the context block */
	    temp = ( int) ( rstruct->regstack + REGSTKSIZ );
	    temp1 = temp;
	    temp1 &= ~3;
	    rstruct->regssp = ( char * ) temp1;
#ifdef mips
	    rstruct->regssp -= 36;
#endif
	    rstruct->regsfp = 0;
	    rstruct->regspc = regis;
#ifndef mips
	    (char *) rstruct->regspc += 2;
#endif
	    }
#else
	if ( (rstruct->regspc != (void *) new_regis) &&
	     (rstruct->thread_exists_flag) )
#ifdef USE_PTHREADS
	    {
	    pthread_addr_t status;
	    subregis (0);
	    TCALL (pthread_join (rstruct->thread, &status));
	    TCALL (pthread_detach (&rstruct->thread));
	    TCALL (pthread_mutex_destroy (&rstruct->mutex));
	    TCALL (pthread_cond_destroy (&rstruct->bell));
	    rstruct->thread_exists_flag = 0;
	    rstruct->regspc = (void *) regis;
	    }
#else /* (assume USE_LWP) */
	    {
	    LWP_CALL (lwp_destroy (rstruct->thread));
	    rstruct->thread_exists_flag = 0;
	    rstruct->regspc = regis;
	    }
#endif USE_PTHREADS
	else
	    rstruct->regspc = (void *) new_regis;
#endif USE_REGIS_ASSEMBLY_CODE
    }
    o_disable_cursor( w, CURSOR_DISABLED_DCS );
}

/* xregis - send data to the ReGIS interpreter */

xregis( ld, pbuffer, length )
    wvtp ld;
    char **pbuffer;
    int length;
{
    int eob, last, drop;
    char ch, *bptr, scratch_buffer[2];

    regis_set_widget( ld );

/* initialize the color map unless it's already initialized */

    if ( ! ld->common.color_map_allocated )
	{
	allocate_color_map( ld );
	}

/* make sure the buffer doesn't start with a null */

    bptr = *pbuffer;
    if ( bptr[0] == '\0' )
	{
	(*pbuffer)++;
	return;
	}

/* scan through the buffer for the first escape or C1 control character */

    for ( eob = 0, drop = FALSE; eob < length; eob++ )
	{
	ch = bptr[eob];
	if ( ch == '\33' || ch == '\30' || ch == '\32'
	  || '\200' <= ch && ch <= '\237' )
	    {
	    drop = TRUE;
	    break;
	    }
	else if ( ch == '\0' )
	    break;	/* parser can't handle nulls */
	}

    if ( eob < length )
	last = eob;
    else
	last = eob - 1;

/* subregis expects a null terminated buffer, while we have a counted buffer.
   Since we don't know the maximum buffer size, send all but one character in
   one packet, then send the last character in a second call to subregis. */

    if ( bptr[last] == '\0' )
	{
	subregis( bptr );
	if ( rstruct->paused )
	    {
	    *pbuffer = rstruct->paused_data;
	    return;
	    }
	}
    else if ( eob > 1 )
	{  /* make sure there is more than one byte */
	scratch_buffer[0] = bptr[last];
	bptr[last] = '\0';
	subregis( bptr );
	if ( rstruct->paused )
	    {
	    bptr[last] = scratch_buffer[0];
	    *pbuffer = rstruct->paused_data;
	    return;
	    }
	bptr[last] = scratch_buffer[0];
	}
    else
	scratch_buffer[0] = bptr[0];
    if ( bptr[last] != '\0' && eob > 0 )
	{
	scratch_buffer[1] = '\0';
	subregis( scratch_buffer );
	}

/* adjust the buffer pointer to skip the part we just parsed */

    *pbuffer += eob;

/* exit ReGIS if we saw a string terminator */

    if ( drop )
	{
	WVT$EXIT_REGIS( ld );
	}
}
	
/* WVT$EXIT_REGIS - return to text mode */

WVT$EXIT_REGIS( ld )
    wvtp ld;
{
    DECtermWidget w = ld_to_w( ld );
    struct regis_cntx *rs = ( struct regis_cntx * ) w->regis;

    _cld wvt$b_in_dcs = False;	/* terminate DCS at ESC or C1 */
    if ( rs->input_mode == 1 )
	{  /* disable multiple input mode */
	rs->input_mode = 0;
	G122_DISABLE_INPUT();	/* disable regis_button_handler */
	}
    o_enable_cursor( ld, CURSOR_DISABLED_DCS );
}

/* puts_reg - write data to the host */

puts_regis( buffer, len)
    char *buffer;
    int len;
{
    struct regis_cntx *rs = RSTRUCT;

    i_report_data( rs->widget, buffer, len );
}

/* regis_set_widget - point to a DECterm widget's ReGIS context */

regis_set_widget( w )
    DECtermWidget w;
{
    rstruct = ( struct regis_cntx * ) w->regis;
    rstruct->widget = w;
}

/* set_value routines - called when values change */

regis_set_value_transcriptSize( oldw, neww )
    DECtermWidget oldw, neww;
{
}

regis_set_value_shareColormap( oldw, w )	/* shareColormapEntries */
    DECtermWidget oldw, w;
{

/* Clear the screen and reset the colormap so that there are no allocated
 * colors visible, but make sure the reset uses current number of planes,
 * not the requested number of planes.
 */

    if ( XtIsRealized(w) )
	{
	int new_bitPlanes = w->common.bitPlanes;
	w->common.bitPlanes = oldw->common.bitPlanes;
	s_clear_display( w );
	WVT$RESET_COLORMAP( w );
	w->common.bitPlanes = new_bitPlanes;
	}

    switch ( w->common.visual->class )
	{
	case StaticColor:
	case TrueColor:
	case DirectColor:
	case StaticGray:
	case GrayScale:
	    w->common.shareColormapEntries = True;
	    break;
	}

    if ( w->common.hardware_planes == 1 )
	w->common.graphics_mode = SINGLE_PLANE;
    else if ( w->common.shareColormapEntries )
	w->common.graphics_mode = ALLOCATED_COLORS;
    else
	w->common.graphics_mode = ALLOCATED_PLANES;
}

regis_set_value_bitPlanes( oldw, w )
    DECtermWidget oldw, w;
{
    int new_bitPlanes, num_colors;

    if ( w->common.bitPlanes <= 0 )
	{
	if ( w->common.visual->class == StaticGray	/* emulate VT330 */
	  || w->common.visual->class == GrayScale
	  || w->common.hardware_planes < 8 )		/* emulate VT241 */
	    w->common.bitPlanes = 2;
	else
	    w->common.bitPlanes = 4;			/* emulate VT340 */
	}
    else if ( w->common.bitPlanes > 4 )
	w->common.bitPlanes = 4;

    num_colors = ( 1 << w->common.bitPlanes );

/* clear the screen and reset the colormap so that there are no allocated
   colors visible */

    if ( w->common.color_map )
	{	/* only true after regis_realize has returned */
	new_bitPlanes = w->common.bitPlanes;
	w->common.bitPlanes = oldw->common.bitPlanes;
	s_clear_display( w );
	WVT$RESET_COLORMAP( w );
	w->common.bitPlanes = new_bitPlanes;
	}

    if ( w->common.bitPlanes == 1 )
	{
	w->common.text_foreground_index = 1;
	}
    else if ( w->common.bitPlanes < 4 )
        {
        w->common.text_foreground_index = 2;
        }
    else
        {
        w->common.text_foreground_index = 7;
        }

    if ( w->common.screenMode )
	{
	w->common.text_background_index = w->common.text_foreground_index;
	w->common.text_foreground_index = 0;
	}
    else
	w->common.text_background_index = 0;

/*
 * If the color map arrays were previously allocated, reallocate them.
 */
    if ( w->common.color_map )
	{
/*
 * Free the arrays dimension to the old number of colors.
 */
	XtFree( (char *)w->common.color_map );
	XtFree( (char *)w->common.color_map_mono );
	XtFree( w->common.pixel_valid );
	XtFree( w->common.pixel_allocated );
	XtFree( (char *)w->common.plane_masks );
/*
 * Allocate arrays dimensioned to the new number of colors.
 */
	w->common.color_map = ALLOC_ARRAY( XColor, num_colors );
	w->common.color_map_mono = ALLOC_ARRAY( unsigned long, num_colors );
	w->common.pixel_valid = ALLOC_ARRAY( Boolean, num_colors );
	w->common.pixel_allocated = ALLOC_ARRAY( Boolean, num_colors );
	w->common.plane_masks = ALLOC_ARRAY( unsigned long,
		w->common.bitPlanes );
/*
 * Now initialize the colormap to default colors
 */
	WVT$RESET_COLORMAP( w );
	}
}

regis_set_value_backingStore( oldw, neww )	/* backingStoreEnable */
    DECtermWidget oldw, neww;
{
    if ( ! neww->common.backingStoreEnable
		&& neww->common.backing_store_active )
	destroy_backing_store( neww );
}

regis_set_value_regisScreenMode( oldw, neww )
    DECtermWidget oldw, neww;
{
    if ( oldw->common.regisScreenMode )
	{
	destroy_backing_store( neww );
	}
    else
	{
	regis_set_widget( neww );
	regis_resize_window( neww );
	}
    o_set_value_fontSetSelection( oldw, neww );
}
