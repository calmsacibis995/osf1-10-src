/*****	REGIS3 (IDENT = 'V1.60') *****/			/* 2 */
/* #module <module name> "X0.0" */
/*
 *  Title:	regis3.c
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
* FACILITY:	Common ReGIS interpreter for VT200 and CT100
*
* ABSTRACT:	Main interpreter code
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	David Larrick	CREATION DATE:	6-Jul-81
*
* MODIFICATION HISTORY (please update IDENT above):
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
* Eric Osman		6-Jan-1991	V3.1
*	- Fix convert_floating_to_integer to use actual machine's word
*	  size as limit, rather than assuming 32767 as largest integer.
*
* Bob Messenger		29-May-1989	X2.0-13
*	- W(M0) means don't draw anything for pixel vectors.
*
* Bob Messenger		 8-May-1989	X2.0-10
*	- Limit W(P(Mn))) to 16, to prevent accvios.
*
* Bob Messenger		 7-Apr-1989	X2.0-6
*	- Support variable number of bit planes.  Support w->common.color_map
*	  directly instead of rs->cm_data.
*
*	Edit 060	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 059	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit 058	21-Oct-83	/AFV				   
* fix bug caused by my blindly changing all logical tests		   
* to IS_TRUE(x) tests.  The relative flag produced by			   
* FLOAT_NUMBER needs to have the low order bit tested, not		   
* the test agains 0 that IS_TRUE(x) does.				   
*									   
*	Edit 057	26-Aug-83	/AFV
* Move all ACCUMULATE_... routines plus FLOATING_SCALAR 
* and FLOATING_NUMBER to RENUMBER.BLI module
* Rename FLOATING_NUMBER to FLOAT_NUMBER to eliminate
* interference with x1_floating_bracketed_pair
*
*	Edit 056	25-Aug-83	/AFV
* Change linkage for SCAN
*
*	Edit 055	23-Aug-83	/AFV
* Change linkage for inch_q_unq (and getl_regis and puts_regis)
*
*	Edit 054	 2-Aug-83	/AFV
* Change search for 'closest' writing color to look for only
* those colors we can currently write -- use the current 
* plane mask to determine what we can write.
*
* 	Edit 053	14-Jun-83	/DCL & AFV
* Implement ASYNCHRONOUS_COLOR_MAP -- synchronization
* with a color map that may be modified without the
* Regis parser's knowledge. (Regis v1 edit 053)
*
* (note: Regis v1 edit 052 not applicable.)
*
*
*	Edit 052	 2-May-83	/AFV
* Prepare for GIDIS v2.
*
*	Edit 051	11-Apr-83	/DCL & BKC & AFV & DS
* In subtract_floating, fix a double-dot problem.
*
*	Edit 050	6-Apr-83	/DCL & AFV
* Make routine SKIP_PAREN just count superfluous left parens, rather than
* recursing; therefore, the left paren count can be a local, and can be
* boundless.  Also decrement this count when appropriate.
*
*	Edit 049	22-Mar-83	/DCL
* Bring over routine c_args (color arguments processor) from module REGIS3.
*
*	Edit 048	21-Mar-83	/DCL
* Bring over more routines from module REGIS:  x1_floating_bracketed_pair,
* number, write_instruction, and all of their subordinates.  Move routine
* scalar from REGIS to REGIS3 also, but comment it out since it is
* presently unused.  Also move routine new_regis back to module REGIS.
*
*	Edit 047	16-Mar-83	/DCL
* Support OVERLAY_STYLE = 2, i.e. VT240 overlaying.
*
*	Edit 046	4-Mar-83	/DCL
* Remove standalone global entry points from module REGIS and put them in
* module REGIS3.
*
*	Edit 045	21-Feb-83	/DCL
* Install the argument block parameter passing method for routine
* COORDINATES.
*
*	Edit 044	18-Feb-83	/DCL
* Make all of the variable cell storage size code conditional, to save
* space if it is not necessary.
*
*	Edit 043	15-Feb-83	/DCL
* Change save_state and restore_state masks to exclude text state; it is
* saved separately by the text code in module REGIS2.  As part of parser
* init in new-regis, do a save_state( all_states ) in case we ever restore
* without saving first.
*
*	Edit 042	12-Feb-83	/DCL
* Implement the $P1p entry point:  Make routine new_regis global, and make
* it call routine regis; new_regis is now the powerup entry point.  Don't
* call routine new_regis from routine regis; regis is now the reinit ($P1p)
* entry point (in addition to continuing to be the main parser loop).
*
*	Edit 041	28-Jan-83	/DCL
* Rip out the GLOBAL LITERAL method of version number tracking.  The
* version number is now in the IDENT modifier of the MODULE heading.
*
*	Edit 040	11-Jan-83	/DCL
* fix a conditional compilation switch so we don't get undefineds
* when DIRECT_GID_ENABLED is FALSE.
*
*	Edit 039	14-Dec-82	/DCL
* Implement REVERSE_PATTERN_REGISTER = 0, i.e. put in conditionals for
* whether the pattern register shifts out high bit or low bit first.
* Affects both the pattern register load and character set load routines.
*
*	Edit 038	4-Nov-82	/DCL
* Implement OVERLAY_STYLE = 1, i.e. call REGIS1 and REGIS2 overlays
* through a separate mapping module.
*
*	Edit 037	27-Oct-82	/DCL
* Remove impure data to module REGISRAM.
*
*	Edit 036	15-Oct-82	/DCL
* Strip off color parsing and report routines into overlay module
* REGIS1.  Take text routines from RGTEXT.REQ to make overlay module
* REGIS2.  Make various service routines global so the overlay
* modules can see them.  Also count number of times skip_paren
* recurses so we don't blow the stack away.
*
*	Edit 035	6-Oct-82	/BKC
* Change GID_PROCESS call for the direct path to GIDIS to be a
* GIDI_DIRECT and 9240 to be GID_PASSWORD macros.
*
*	Edit 034	28-Sep-82	/DCL
* Don't resend pattern register on new V instruction when the pattern
* is all ones.
*
*	Edit 033	7-Sep-82	/DCL
* Add an initial dummy argument to G_LINE_TEXTURE.  This argument
* is reserved for future use as a pattern register length.
* Change some error code names:  extra_hardcopy_coordinates -->
* EXTRA_OPTION_COORDINATES (since this error occurs for both S(H) and
* S(A)) and extra_coordinates --> EXTRA_COORDINATE_ELEMENTS.
* Rip out some old commented code related to the possibility of not
* reversing pattern register bits for some products.
*
*	Edit 032	10-Aug-82	/DCL
* Implement correct HLS defaults.
*
*	Edit 031	20-Jul-82	/DCL
* Handle correctly the construct V(W(...))[...][...], i.e. teach
* temporary write options to stay in effect during subsequent
* arguments to the same instruction.
*
*	Edit 030	16-Jun-82	/DCL
* Don't map S(H(P[])) (hardcopy position) specifier.  When S(H) has
* only one position argument, default the other to [+0,+0] correctly.
* Init hardcopy position specifier correctly.  fix a bug in temp
* write options.  Send <CR>s for unimplemented reports.  Make the G
* instruction harder to get at.
*
*	Edit 029	15-Jun-82	/DCL
* Implement init string and remove temporary initialization code from
* new_regis.
*
*	Edit 028	9-Jun-82	/DCL
* Re-send the pattern register for each V instruction; reset various
* states on S(E); make the arguments to S(H(P[,])) sticky; move
* translation of the "RGB" color specifiers from this module (REGIS)
* to the device-dependent color model module (COL???).
*
*	Edit 027	1-Jun-82	/DCL
* Pass the new pattern register multiplier to GIDIS even when the
* pattern register value itself does not change.
*
*	Edit 026	16-Apr-82	/DCL
* fix a bug that causes W(S1,) to generate incorrect GIDIS.  Also
* make a preliminary fix to a one-off error in screen addressing.
*
*	Edit 025	25-Mar-82	/DCL
* fix a 16th-scan-line bug in character set loading.  Implement best
* match algorithm for HLS or RGB specifiers for screen and write
* intensity.
*
*	Edit 024	22-Mar-82	/DCL
* fix a few one-off errors in screen addressing and coordinates
* calculations.  Reverse bits in loadable fonts.  finish job of
* reversing bits in pattern register loading, especially predefined
* patterns.
*
*	Edit 023	17-Feb-82	/DCL
* Install hooks to call HLS to RGB color mapping module, thus
* partially implementing color map handling.  Reverse order of bits
* in pattern register.  In screen addressing calculations, use number
* of pixels vs. highest pixel number correctly.  fix ASCIZ problems
* of reports by modifying binary_to_decimal_convert to return the
* number of characters in the output string.
*
*	Edit 022	9-Feb-1982	J.A.Lomicka
* Change name of main routine from main to regis.  Remove main
* declaration.
*
*	Edit 021	27-Jan-82	/DCL
* Implement negated writing modes.  Start all canned patterns with 1.
* Implement G instruction for GIDIS debugging.  Implement W(F).
* Implement S(D) (data shift toggle).  Delimit error report with
* double quotes, not parens.
*
*	Edit 020	11-Jan-82	/DCL
* Implement external report paths.  Enhance resynch (;) handling.
*
*	Edit 019	8-Jan-82	/DCL
* Handle redundant comma in character set loading.  fix lower case
* problem following quotation (use INCH, not INCH_Q).  fix precedence
* of "E" and "." in numbers (2E1.9 no longer yields 29).
*
*	Edit 018	7-Jan-82	/DCL
* Implement P(S) and V(S).  Make reentrant data structure global.
*
*	Edit 017	6-Jan-82	/DCL
* fix another annoying bug in character set loading.
*
*	Edit 016	4-Jan-82	/DCL
* fix an annoying bug in character set loading.  gid_PROC is no
* longer an external routine; its equivalent is declared in GIDDEF.
*
*	Edit 015	22-Dec-81	/DCL
* Implement internal parser initialization.
*
*	Edit 014	19-Dec-81	/DCL
* Convert all OWN variables to block references, hidden by macro
* calls.  This technique provides hooks for reentrancy.
*
*	EDIT 013	17-Dec-81	/DCL
* Implement temporary write options.
*
*	Edit 012	15-Dec-81	/DCL
* Reporting functions internal to the parser are implemented.
* Integration and testing of report functions await implementation of
* the appropriate hooks in GIDIS (for position report) and SCAN (for
* macrograph reports).  Alphabet name report and error report are
* fully functional.
*
*	Edit 011	25-Nov-81	/DCL
* Significantly change coordinate handling once again, to handle
* always-square logical pixels.  Implement character set loading.
*
*	Edit 010	28-Oct-81	/DCL
* finish implementing shading.  Implement pattern register processing.
*
*	Edit 009	12-Oct-81	/DCL
* Modified SKIP_QUOTE routine to take its terminating character as an
* argument rather than in .CHAR.  Partially implemented shading.
* Revised screen addressing and coordinate handling to use powers of
* 2 in addition to powers of 10, thereby increasing both efficiency
* and resolution.  The upper limit of coordinates passed to GIDIS for
* S(A instructions will be between 2 ** 13 and 2 ** 14 - 1, thus
* allowing one bit for positive offscreen addressing and one bit for
* negative offscreen addressing.
*
*	Edit 008	23-Sep-81	/DCL
* Implemented screen addressing and appropriate transformations of
* coordinates and pixel vectors.
*
*	Edit 007	14-Sep-81	/DCL
* Implemented write instructions.
*
*	Edit 006	29-Aug-81	/DCL
* Implemented screen instructions, except for some of the color
* processing - colors are collected in HLS, not yet converted to
* RGB.  Pixel vectors are now translated into vectors, etc, by
* this program, although pixel vector multipliers and logical
* pixel size have yet to be implemented.
* 
*	Edit 005	25-Aug-81	/DCL
* Partially implemented screen instructions.  Rewrote COORDINATES
* routine to work around a compiler problem.
*
*	Edit 004	20-Aug-81	/DCL
* Installed fraction handling and exponent adjustment for coordinates.
* Generalized the number parsing routine to handle non-coordinate
* numbers and to perform range checking.
*
*	Edit 003	10-Aug-81	/DCL
* Significantly improve coordinate handling, but still use the old
* word-encoded (4095 maximum) coordinate passing scheme for GIDIS.
*
*	Edit 002
* Coordinates and pixel vectors work for C instructions.
* Install an include command so Jeff can proceed independently on
* implementing text instructions.
*
*	Edit 001	14-Jul-81	/DCL
* fill in skeleton.  Coordinates and pixel vectors work for P and
* V instructions.  Only integers (no fractions or exponents) are
* implemented for coordinates.  Default for coordinates is absolute
* zero, not relative zero.
**/



#include "gidcalls.h"
#include "regstruct.h"

/*****			E X T E R N A L   F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      MODULE */
/*	--------		-----------			      ------ */
extern
	get_field(),		/* Get a specified bit field	      FIELDS */
	put_field(),		/* Put a bit field into word	      FIELDS */
	float_number(),		/* Get signed, non-normalized number RENUMBER*/
	hlsrgb(),		/* HLS to RGB color conversion	      COLrgb */
	inch(),			/* Fetch char, normal mode	      SCAN   */
	inch_q_unq(),		/* Fetch char, quoted mode	      SCAN   */
	p_inch(),		/* Push character back to scanner     SCAN   */
#if VARIANT == 1
	PutBreak(),		/* Flush all pending terminal output  REGIO  */
	PutChar(),		/* Queue character on terminal output REGIO  */
	PutDecimal(),		/* Output signed decimal number       OPSYS  */
	Put_String(),		/* Output an ASCIZ string	      OPSYS  */
#endif
	rgbhls(),		/* "RGB" to HLS color conversion      COLrgb */
#if SEPARATE_CURRENT_POSITION
	rpg_request_gidi_pos(),	/*				      REPORT */
#endif
	save_state();		/*				      REGIS4 */
#if ASYNCHRONOUS_COLOR_MAP
extern
	update_color_map();	/* ?? This function's definition could not   */
#endif				/*       be found			     */

/*****		F U N C T I O N   I N   T H I S   F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION			     */
/*	--------			-----------			     */
/**
*	bracketed_pair(a,a,a)		Process [	(unscaled, normalized)
*	c_args(a,a)			Parse color specifiers
*	convert_floating_to_integer(v,v)
*	coordinates()			Process [ (scaled, normalized, abs)
*	divide_by_10_rounded(v,v)
*	fetch_one_character(v)
*	x1_floating_bracketed_pair(a,a,a,a,a,a,v,v)
*					Process [  (unscaled, not normalized)
*	ignore_ch()			Skip over a character
*	intensity_suboption(a)		Process S(I and W(I
*	number(a,a,v)			Collect signed, normalized number
*	p0_pattern_register()		Process W(P
*	p1_pattern_register_suboption(a) Process W(P(
*	rel_coordinates()		Process [ (scaled, normalized)
*	scan(a)				Lookup char in list
*	scrn_write_intensity(v)		Process S(I and W(I
*	set_line_texture()		Calculate pt.size
*	s0_shading()			Process W(S
*	s1_shading_action(v,v)		Generate GIDIS for W(S(
*	s2_shading_suboption()		Process W(S(
*	skip_coord()			[] skip state
*	skip_option()			Option skip state
*	skip_paren()			() skip state
*	skip_quote(v)			Quotation skip state
*	subtract_floating(v,v,v,v,a,a)
*	temp_write_options()		Process <instruction>(W
*	write_instruction()		Process W
*	w1_write_option()		Process W(
**/

bracketed_pair(x, y, relflags)
int	*x;
int	*y;
int	*relflags;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Coordinates state - handle ordered pairs of the form [x,y]
    *   where x and y are generally floating-point and may be
    *   relative (signed) or absolute.  Do not scale the result to
    *   reflect the current screen addressing.  Used when the [x,y]
    *   syntax is used for numbers other than screen addressing
    *   coordinates.
    *
    * FORMAL PARAMETERS:
    *   x	  = address in which to store the x coordinate
    *   y	  = address in which to store the y coordinate
    *   relflags = address of the "relative flags word" in which to
    *		  store the logical OR of:
    *			256 = x is relative
    *			512 = y is relative
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:
    *   0 = routine terminated on ; (Resynch) (Note that
    *		first_process_me is also set in this case)
    *   1 = routine terminated on ]
    *
    * SIDE EFFECTS:		none
    **/

    int		routine_value;
    int		x_exponent;
    int		x_mantissa;
    int		y_exponent;
    int		y_mantissa;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    routine_value = x1_floating_bracketed_pair(&x_mantissa, &x_exponent,
	&y_mantissa, &y_exponent, relflags, 0, 0);
    *x = convert_floating_to_integer(x_mantissa, x_exponent);
    *y = convert_floating_to_integer(y_mantissa, y_exponent);

    return(routine_value);
}

c_args(red, green, blue, mono, which_color_map)
int	*red;
int	*green;
int	*blue;
int	*mono;
int	*which_color_map;
{
    /**
    *			Parse color specifiers
    * rgb_mono_word	Input address in which to store
    * 			  word-encoded red-green-blue-mono values.
    * which_color_map)	Input address in which to store
    * 			  -1 = primary color map [color + mono]
    * 			  0  = first alternate [color only]
    *			  1  ... more alternates [color + mono]
    **/
    /**
    * Implicit inputs:
    *   ALTERNATE_MONITOR is a literal which indicates whether
    *   alternate monitor support is enabled.
    *
    * Implicit outputs:
    *
    * Function:
    *   Parse HLS or RGB color specifiers, and translate HLS to RGB.
    *
    * Side effects:		none
    **/

    int		hue;
    int		lightness;
    int		rel;
    int		saturation;
    int		got_color = 0;	/* 1 = rgb, 2 = hls */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */


#if ASYNCHRONOUS_COLOR_MAP

    update_color_map();		/* See comment above under external functions*/
#endif

    *which_color_map = (-1);		/* unspecified - no 'A' */
    hue = (-1);				/*Indicate no hue specified, */
    lightness = (-1);			/* no lightness specified */
    saturation = (-1);			/* and no saturation specified */

    while ( TRUE )
    {
	switch (scan( "(ABCDGHLMRSWY)" ))
	{
	    case -3 : 			/*Resynch */
		rs->first_process_me = TRUE;
		return(FALSE);
	    case -2 :
		skp_quote(rs->character);
		break;
	    case -1 :
		skp_option();		/*Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/*Nothing interesting */
		break;
	    case  1 :
		skp_paren();		/*Left paren */
		break;
	    case  2 : 			/*A */

#if ALTERNATE_MONITOR 
	    {
		int	valu;
		int	rel;

		number(&valu, &rel, 0);
		*which_color_map = ((valu < 0) ? (-valu) : valu);
	    }
#else
		ignore_ch();
#endif
		break;
	    case  3:	/* B - blue */
		get_rgb( BLUE, red, green, blue, mono );
		got_color = 1;
		break;
	    case 4:	/* C - cyan */
		get_rgb( CYAN, red, green, blue, mono );
		got_color = 1;
		break;
	    case 5:	/* D - black (dark) */
		get_rgb( BLACK, red, green, blue, mono );
		got_color = 1;
		break;
	    case 6:	/* G - green */
		get_rgb( GREEN, red, green, blue, mono );
		got_color = 1;
		break;
	    case 9:	/* M - magenta */
		get_rgb( MAGENTA, red, green, blue, mono );
		got_color = 1;
		break;
	    case 10:	/* R - red */
		get_rgb( RED, red, green, blue, mono );
		got_color = 1;
		break;
	    case 12:	/* W - white */
		get_rgb( WHITE, red, green, blue, mono );
		got_color = 1;
		break;
	    case 13:	/* Y - yellow */
		get_rgb( YELLOW, red, green, blue, mono );
		got_color = 1;
		break;

	    case  7 : 			/*H */
		got_color = 2;
		number(&hue, &rel, 0);
		hue = hue % 360;	/* hue MOD 360 */
		if (hue < 0)
		{
		    hue += 360;
		}
		break;
	    case  8 : 			/*L */
		got_color = 2;
		number(&lightness, &rel, 0);
		if (lightness < 0)
		{
		    lightness = 0;
		}
		else if (lightness > 100)
		{
		    lightness = 100;
		}
		break;
	    case 11 : 			/*S */
		got_color = 2;
		number(&saturation, &rel, 0);
		if (saturation < 0)
		{
		    saturation = 0;
		}
		else if (saturation > 100)
		{
		    saturation = 100;
		}
		break;
	    case 14 : 			/*Right paren */
		if (got_color == 0)
		{		
		    return(FALSE);
		}
		else if (got_color == 1)
		{  /* rgb */
		    return(TRUE);
		}
		if (hue == -1)		/*Was hue specified? */
		{ 			/*No - force saturation to 0 */
		    saturation = 0;
		}
		hlsrgb(hue, lightness, saturation, red, green, blue, mono);
		return(TRUE);
	}		/* end of switch scan ... */

    }			/* end of while TRUE */

}

get_rgb( color_code, red, green, blue, mono )
    int color_code, *red, *green, *blue, *mono;
{
    struct regis_cntx *rs = RSTRUCT;
    DECtermWidget w = rs->widget;

    *red   = w->common.pure_color[color_code].red;
    *green = w->common.pure_color[color_code].green;
    *blue  = w->common.pure_color[color_code].blue;

    *mono = ( *green * 59 + *red * 30 + *blue * 11 ) / 100;
}

/*
 * Compute largest possible non-overflowing positive integer by assuming 8
 * bits per byte, and shifting a 1 to the NEXT to left-most position,
 * subtracting 1 to produce all 1's except in leftmost 2 positions, shifting
 * left once to produce all 1's except in both ends, then add 1 to produce all
 * 1's except on left end.
 */

#define LARGEST_INT ((((1<<(sizeof(int)*8-2))-1)<<1)+1)

convert_floating_to_integer(mantissa, exponent)
int	mantissa;
int	exponent;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   This routine applies the exponent to the mantissa to return
    *   a 16-bit, signed, two's complement integer that is as
    *   accurate as possible.
    *
    * FORMAL PARAMETERS:
    *   mantissa	= value of mantissa
    *   exponent	= value of exponent
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		The answer.
    *
    * SIDE EFFECTS:		none
    **/

    int		answer;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: convert_floating_to_integer, regis3.c\n", PutChar);
#endif
**/

    answer = mantissa;

    if (exponent >= 0)
    {
	int	dummy;

	for (dummy = 1; dummy <= exponent; dummy++)
	{
/*
 * The following line used to say "3276" instead of "largest_int/10", but
 * I changed it on 6-Jan-1992 because the following regis produced a diagonal
 * line only HALFWAY across the screen instead of all the way across:
 *
 *	S(A[3000,3479][3799,3000])p[3000,3479]v[3799,3000]
 *
 * What happened is that scrn_action_addressing called convert_floating_to_int
 * with (3479,1) which caused too much round-off error.
 *
 * The above bug was caught by a customer.  I documented the change this
 * extensively because at this time we do not have a test suite to show
 * confidently whether this change produces other problems, so in case you
 * are a developer that is tracking a problem which you believe is in this
 * area, beware of the above regis and make sure it still works if you
 * make another change here !  Thanks.  --Eric Osman
 */
	    if (((answer < 0) ? (-answer) : answer) > LARGEST_INT/10)
						/*Will *10 overflow? */
	    {	    
		answer = MULTIPLY_BY_SIGN(LARGEST_INT, answer);
	    }			/*Yes - set to correctly signed MAXINT */
	    else
	    {
		answer *= 10;	/*No - it's safe to multiply */
	    }
    	}
    }
    else
    {
	answer = div_10_rounded(answer, (-exponent));
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, convert_floating_to_integer, regis3.c\n", PutChar);
#endif
**/
    return(answer);
}

coordinates()
{
    int		return_value;

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    return_value = rel_coordinates();

    /* if number given by user was explicitly relative,  */
    /* adjust by gid_x and gid_y */

    /* else adjust by the coordinates of the upper left point. */

#if SEPARATE_CURRENT_POSITION
    if (rs->coord_relflags != 0 && IS_FALSE(rs->gid_xy_valid))
    {
	rpg_request_gidi_pos();
    }
#endif
    if ((rs->coord_relflags & XRELATIVE) != 0)
    {
	rs->x_crd = rs->gid_x + rs->rel_x_coord;
    }
    else
    {
	rs->x_crd = rs->rel_x_coord - rs->sa_ulx;
    }
    if ((rs->coord_relflags & YRELATIVE) != 0)
    {
	rs->y_crd = rs->gid_y + rs->rel_y_coord;
    }
    else
    {
	rs->y_crd = rs->rel_y_coord - rs->sa_uly;
    }
    return(return_value);
}

div_10_rounded(dividend, exponent)
int	dividend;
int	exponent;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   This routine performs divisions by powers of 10, handling
    *   rounding correctly.
    *
    * FORMAL PARAMETERS:
    *   dividend	= value of number to be divided by 10
    *   exponent	= value of number of times to divide by 10
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		The quotient
    *
    * SIDE EFFECTS:		none
    **/

    int		answer;
    int		old_answer;
    int		induction;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    answer = dividend;			/*Prime the pump */

    for (induction = 1; induction <= exponent; induction++)
    {
	if (answer == 0)
	{
	    break;	/*Don't keep going for nothing */
	}
	old_answer = answer;
	answer /= 10;

	if (induction == exponent)	/*The last time through... */
	{					/*...try to round */
	    int		temp;
	    temp = old_answer - (answer * 10);
	    temp = ((temp < 0) ? (-temp) : temp);
	    if (temp >= 5)
	    {				/*Does the digit call for rounding? */
	    
		if (dividend < 0)	/*Round in the right direction */
		{		
		    answer--;
		}
		else
		{
		    answer++;
		}
	    }
	}
    }
    return(answer);
}

fetch_one_character(quote_character)
int	*quote_character;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Read a quoted string from the input stream, ignoring all
    *   but the first character.
    *
    * FORMAL PARAMETERS:
    *   quote_character = value of the quotation character to which to skip.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		The single character
    *
    * SIDE EFFECTS:		none
    **/

    int		return_value;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    for (;;)
	{
	rs->character = (return_value = INCH_Q(0));
				/*Fetch one character, literal mode */
	if ( rs->character >= ' ' )
	    break;		/* ignore control characters */
	}
    rs->first_process_me = TRUE;
				/*In case it was a closing quote */
    skp_quote(quote_character);	/*Skip the rest */

    return(return_value);
}

x1_floating_bracketed_pair(x_mantissa, x_exponent, y_mantissa,
    y_exponent, relflags, x_initial_exponent, y_initial_exponent)
int	*x_mantissa;
int	*x_exponent;
int	*y_mantissa;
int	*y_exponent;
int	*relflags;
int	x_initial_exponent;
int	y_initial_exponent;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Coordinates state - handle ordered pairs of the form [x,y]
    *   where x and y are generally floating-point and may be
    *   relative (signed) or absolute.  return both coordinates in
    *   mantissa, exponent form.
    *
    * FORMAL PARAMETERS:
    *   x_mantissa	= address in which to store the mantissa
    *				portion of the x coordinate
    *   x_exponent	= address in which to store the exponent
    *				portion of the x coordinate
    *   y_mantissa	= address in which to store the mantissa
    *				portion of the y coordinate
    *   y_exponent	= address in which to store the exponent
    *				portion of the y coordinate
    *   relflags	= address of the "relative flags word" in
    *				which to store the logical OR of:
    *			256 = x is relative
    *			512 = y is relative
    *   x_initial_exponent	= value of the "initial exponent"
    *				  argument to float_number for x.
    *   y_initial_exponent	= value of the "initial exponent"
    *				  argument to float_number for y.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:
    *   0 = routine terminated on ; (Resynch) (Note that
    *		first_process_me is also set in this case)
    *   1 = routine terminated on [
    *
    * SIDE EFFECTS:		none
    **/

    int		exponent;
    int		initial_exponent;
    int		items_seen;
    int		mantissa;
    int		rel;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    items_seen = 0;			/*0 = nothing seen yet */
    					/*1 = first coordinate only */
    					/*2 = (one) comma seen */
    					/*3 = second coordinate seen */
    *relflags = 0;			/*Init to all absolute */
    initial_exponent = x_initial_exponent;	/*Use the one for x first */

    while ( TRUE )
    {
/**
#if VARIANT == 1
    PutString( "\nBEFORE: mantissa, exponent\n", PutChar);
    PutDecimal(mantissa, PutChar);
    PutString( "\n", PutChar);
    PutDecimal(exponent, PutChar);
    PutString( "\n", PutChar);
#endif
**/
	if (IS_TRUE(float_number(&mantissa, &exponent, &rel,initial_exponent)))
	{				/*Go fetch a number */
/**
#if VARIANT == 1
    PutString( "\nAFTER: mantissa, exponent\n", PutChar);
    PutDecimal(mantissa, PutChar);
    PutString( "\n", PutChar);
    PutDecimal(exponent, PutChar);
    PutString( "\n", PutChar);
#endif
**/
	    items_seen++;
	    switch (items_seen)
	    {
		case  1 :		/*Save x value and relative state */
		    *x_mantissa = mantissa;
		    *x_exponent = exponent;

		    if ((rel & 1) != 0)
		    {
			*relflags = *relflags | XRELATIVE;	 /*2 */
		    }
		    break;
		case  3 :		/*Save y value and relative state */
		    *y_mantissa = mantissa;
		    *y_exponent = exponent;

		    if ((rel & 1) != 0)
		    {
			*relflags = *relflags | YRELATIVE;	 /*2 */
		    }
		    break;
		default :
		    ERROR(EXTRA_COORDINATE_ELEMENTS, 0);
	    }
	}
	switch (scan( "]," )) /*What made NUMBER terminate? */
	{
	    case -3 :			/*; */
		rs->first_process_me = TRUE;
		return(0);
	    case -2 :
	    case -1 :
	    case  0 : 		/*Unexpected characters - skip to ], */

		while ( TRUE )
		{
		    int		exit;

		    exit = FALSE;

		    switch (scan( "]," ))
		    {
			case -3 :		/*; */
			    rs->first_process_me = TRUE;
			    return(0);
			case -2 :
			case -1 :
			case  0 : 	/*Unexpected - continue loop */
			    ignore_ch();
			    break;
			case  1 :
			case  2 :	/*], */
			    rs->first_process_me = TRUE;
			    exit = TRUE;
		    }
		    if ( exit )
		    {
			break;	/* exit while loop. */
		    }
		}
		break;		/* exit switch */
	    case  1 :			/*] */

		switch (items_seen)
		{
		    case  0 :		/*return relative zero for x and y */
			*x_mantissa = 0;
			*x_exponent = 0;
			*y_mantissa = 0;
			*y_exponent = 0;
			*relflags = *relflags | XRELATIVE | YRELATIVE;
			break;
		    case  1 :
		    case  2 :		/*return relative zero for y */
			*y_mantissa = 0;
			*y_exponent = 0;
			*relflags = *relflags | YRELATIVE;
		}
		return(1);
	    case  2 :

		switch (items_seen) 	/*Comma */
		{
		    case  0 :		/*Plug in relative zero for x */
			*x_mantissa = 0;
			*x_exponent = 0;
			*relflags = *relflags | XRELATIVE;
			items_seen = 2;
			initial_exponent = y_initial_exponent;
			break;
		    case  1 :
			items_seen = 2;
			initial_exponent = y_initial_exponent;
			break;
		    case  2 :		/*Plug in relative zero for y */
			*y_mantissa = 0;
			*y_exponent = 0;
			*relflags = *relflags | YRELATIVE;
			items_seen = 3;	/*Pretend we saw a y */
			break;
		    default :
			ignore_ch();
		}
	}	
    }
}

ignore_ch()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Called when the current value of CHAR is of no interest for
    *   one reason or another.  This condition may be considered an
    *   error, so the error code is set and the value of CHAR
    *   preserved as the error character.  The error tracking code
    *   is only compiled if the appropriate switch is set.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		The current value of CHAR
    *
    * IMPLICIT OUTPUTS:		ERR_CODE and ERR_CHAR are set
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    rs->first_process_me = FALSE;
    ERROR(UNEXPECTED_CHARACTER, rs->character);
}

is_intensity_suboption(intensity)
int	*intensity;
{
    /**
    * Handle S(I( and W(I( 
    *   INTENSITY  -- Input address in which to store the number of the
    *   selected color map entry.
    **/
    /**
    * Implicit inputs:		cm_data, the color map array
    *
    * Implicit outputs:		none
    *
    * Function:
    *   Implement the suboptions of the I(ntensity) options of the S(creen) and
    *   W(rite) instructions. Parse the suboptions, translate to RGB if
    *   necessary, and find the best match in the color map.  Note that this
    *   routine never modifies the color map.
    *
    * Algorithm:
    *   Compare each color map entry with the desired RGB values.  The best match
    *   is the first entry found with minimum diff, where diff is defined as the
    *   sum of the absolute values of the red, green, and blue differences.
    *
    * Side effects:		none
    **/

    int		diff;
    int		dummy;
    int		i_entry;
    int		v2_min_diff;
    int		v1_min_diff_pointer;
    int		red;
    int		green;
    int		blue;
    int		mono;
    int		num_colors;
    DECtermWidget w;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    w = rs->widget;

    num_colors = ( 1 << w->common.bitPlanes );

    /* Go parse arguments, but quit if nothing there */

    if (IS_FALSE( c_args(&red, &green, &blue, &mono, &dummy) ))
    {
	return;
    }
    v1_min_diff_pointer = -1;		/* init to something silly */

    for (i_entry = num_colors - 1; i_entry >= 0; i_entry--)
    {
      /* only consider values we can write */

	if ((i_entry & rs->md_plane_mask) == i_entry)
	{
	    int	temp1;
	    int	temp2;
	    int	temp3;
	    int	temp4;

	  if ( rs->color_monitor )
	  {
	    temp1 = w->common.color_map[i_entry].red - red;
	    if (temp1 < 0)
	    {
		temp1 = (-temp1);
	    }
	    temp2 = w->common.color_map[i_entry].green - green;
	    if (temp2 < 0)
	    {
		temp2 = (-temp2);
	    }
	    temp3 = w->common.color_map[i_entry].blue - blue;
	    if (temp3 < 0)
	    {
		temp3 = (-temp3);
	    }
	  }
	    temp4 = w->common.color_map_mono[i_entry] - mono;
	    if (temp4 < 0)
	    {
		temp4 = (-temp4);
	    }
	  if ( rs->color_monitor )
	    diff = temp1 + temp2 + temp3 + temp4;
	  else
	    diff = temp4;

	    if (v1_min_diff_pointer < 0 || diff < v2_min_diff)
	    {
		v2_min_diff = diff;
		v1_min_diff_pointer = i_entry;
	    }
	}
    }
    if ( v1_min_diff_pointer < 0 )
	v1_min_diff_pointer = 0;
    *intensity = v1_min_diff_pointer;
}

number(p_value, p_rel, initial_exponent)
int	*p_value;
int	*p_rel;
int	initial_exponent;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Collect and store a generally floating-point number which
    *   may be relative (signed) or absolute.  Handle integers,
    *   fractions, and exponents.
    *
    * FORMAL PARAMETERS:
    *   p_value = address in which to store the 16-bit, two's
    *		   complement, integer answer
    *   p_rel   = address in which to store:
    *			0 = absolute positive
    *			1 = relative positive
    *			2 = absolute negative
    *			3 = relative negative
    *	initial_exponent
    *		= value of the power of 10 by which the (integer)
    *		  exponent should be adjusted.  Negative exponents
    *		  increase the accuracy of answers; positive exponents
    *		  increase the range of answers.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:
    *   first_process_me is always set, since number scans are non-
    *   self-terminating.  The a character that caused the number scan
    *   to terminate is in CHAR.
    *
    * ROUTINE VALUE:	"No error" flag
    *   0 = No number collected
    *   1 = Number collected
    *
    * SIDE EFFECTS:		none
    **/

    int		exponent;
    int		mantissa;
    int		return_value;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */


    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: number, regis3.c\n", PutChar);
#endif
**/

    return_value = float_number(&mantissa, &exponent, p_rel, initial_exponent);

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("number,regis3.c,debug1\n", PutChar);
#endif
**/
    *p_value = convert_floating_to_integer(mantissa, exponent);

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, number, regis3.c\n", PutChar);
#endif
**/

    return(return_value);
}

p0_pattern_register() 		/*Handle W(P */
{
    /**
    * Routine value:	none
    *
    * Implicit inputs:
    *   multiplier: integer, ReGIS pattern register multiplier.
    *
    * Implicit outputs:
    *   pt_register:  bit string, ReGIS pattern register.  The low-order bit
    *         of pt_register will be the first one used for patterned drawing.
    *   pt_size: size in IDS space of pattern
    *
    * Function:
    *   Implement the P(attern register) option of the W(rite) instruction.
    *   Decimal digit arguments have a special meaning.  0s and 1s are
    *   accumulated to the software pattern register, pt_register. Digits 2
    *   through 9 cause "predefined patterns" to be loaded into the pattern
    *   register. Transitions between individual bit specification and
    *   predefined pattern specification include special handling; basic
    *   ally, the two types of specification overwrite each other.  On exit
    *   from this routine, perform any pattern replication necessitated by
    *   explicit specification of fewer than 16 bits (note that
    *   predefined patterns, being 8 bits long, are in this category) and
    *   pass the result to GIDIS.
    *
    * Algorithm:
    *   The local variable bits_seen is used as both a counter and as a flag.
    *   if positive or zero, it reflects the number of bits that have been
    *   specified explicitly and shifted into pt_register (not allowed to
    *   exceed 16).  The processing of a predefined pattern causes it to
    *   be set to -1.
    *
    *   On receiving a 0 or 1, test whether this is the first 0 or 1 seen (at
    *   all, or since the last predefined pattern specification).  if it
    *   is the first, zero pt_register; if not, shift pt_register one bit to
    *   the right.  In either case, clear or set the high bit of pt_register.
    *
    *   [In the above paragraph, whether the above shift is right or left, and
    *   whether the high bit or low bit of pt_register is set or cleared, are
    *   determined by the setting of the literal REVERSE_PATTERN_REGISTER.]
    *
    *   On receiving a digit 2 through 9, move the corresponding predefined
    *   pattern to pt_register, overwriting any bits already there.
    *
    *   On receiving a character that causes exit from the routine, first
    *   shift the high bits of pt_register (.bits_seen of them) to the low
    *   end of pt_register.  Special code ensures that the lengths of
    *   predefined patterns are handled correctly, that the sign bit does
    *   not propogate, and that full 16-bit patterns are not smashed.  Then
    *   copy (or) these significant bits into the high end of pt_register
    *   in groups of size .bits_seen; any bits that will not fit in 16 bits
    *   are ignored.  This technique replicates the specified pattern in
    *   the correct order (low to high), and selects the correct bits to
    *   replicate if the entire pattern will not fit into the 16-bit word
    *   again.
    *
    *   [Again, the direction of the shift, the end of pt_register that is
    *   copied to the other end, and the sense of the correct order (low to
    *   high or high to low) in the above paragraph are determined by the
    *   literal REVERSE_PATTERN_REGISTER.]
    *
    * Side effects:		issues the GIDIS opcode G49_SET_LINE_TEXTURE.
    **/

    int		bits_seen;		/*1 to 16 = number of bits seen */
					/*0 = no bits seen */
					/*-1 = predefined pattern */

/* The following variables should be bound  ??	*/
    static int	bit_pattern[] = 
    {
#if REVERSE_PATTERN_REGISTER == TRUE
0x0000, 0xFFFF, 0x0F0F, 0x2727, 0x5555, 0x5757, 0x1111, 0x2121, 0x1313, 0x6161
/**
*		binary '0000000000000000',		pattern 0
*		binary '1111111111111111',		pattern 1
*		binary '0000111100001111',		pattern 2
*		binary '0010011100100111',		pattern 3
*		binary '0101010101010101',		pattern 4
*		binary '0101011101010111',		pattern 5
*		binary '0001000100010001',		pattern 6
*		binary '0010000100100001',		pattern 7
*		binary '0001001100010011',		pattern 8
*		binary '0110000101100001'		pattern 9
**/
#else
0x0000, 0xFFFF, 0xF0F0, 0xE4E4, 0xAAAA, 0xEAEA, 0x8888, 0x8484, 0xC8C8, 0x8686
/**
*		binary '0000000000000000',		pattern 0
*		binary '1111111111111111',		pattern 1
*		binary '1111000011110000',		pattern 2
*		binary '1110010011100100',		pattern 3
*		binary '1010101010101010',		pattern 4
*		binary '1110101011101010',		pattern 5
*		binary '1000100010001000',		pattern 6
*		binary '1000010010000100',		pattern 7
*		binary '1100100011001000',		pattern 8
*		binary '1000011010000110',		pattern 9
**/
#endif
    };

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    bits_seen = 0;

    while ( TRUE )
    {

	switch (scan( "(0123456789)" ))
	{
	    case -3 : 			/*Synch */
		rs->first_process_me = TRUE;
		return;
	    case -2 : 			/*Quotation */
		skp_quote(rs->character);
		break;
	    case -1 :
	    case 12 : 			/*Unexpected alpha, right paren */

		if (bits_seen == -1)
		{
		    bits_seen = 16;
		}
		else if (bits_seen == 1)
		{		/* only a 0 or a 1 - use predefined */
		    rs->pt_register = bit_pattern [(rs->pt_register == 0) ? 0 : 1];
		    bits_seen = 16;
		}
		if (bits_seen == 0)	/*Handle predefined patterns */
		{			/*No bits seen ever - fall through */
		}
		else
		{		/*Replicate the bits we've seen */
		    if (bits_seen < 16)	/*Don't smash full_length patterns */
		    {

#if REVERSE_PATTERN_REGISTER == TRUE
			rs->pt_register = rs->pt_register >> 1;	/*Right-shift it once... */
			rs->pt_register &= MASK_OFF_SIGN_BIT;
					/*...clear sign bit so it */
					/* doesn't propogate... */
			rs->pt_register = rs->pt_register >> (16 - bits_seen - 1);
					/*...shift the rest of the way */
#else
			rs->pt_register = rs->pt_register << (16 - bits_seen);
				/*This direction only includes one shift */
#endif
		    }
/**
* Replicate the pattern
**/
		    while (bits_seen < 16)
		    {
			int	temp;	/* We need to replicate the pattern */
					/* on the right: */
#if REVERSE_PATTERN_REGISTER == TRUE
			rs->pt_register = (rs->pt_register << bits_seen) | rs->pt_register;
#else

			temp = rs->pt_register;	/*Make a copy... */
			rs->pt_register = rs->pt_register >> 1;
					/*...right-shift the original once... */
			rs->pt_register &= MASK_OFF_SIGN_BIT;
					/*...clear sign bit so it */
					/* doesn't propogate... */
			rs->pt_register = rs->pt_register >> (bits_seen - 1);
					/*...shift the rest of the way... */
			rs->pt_register = rs->pt_register | temp;
					/*...and OR it in with the saved copy */
#endif

			bits_seen = (bits_seen << 1); /* bits_seen=bits_seen*2 */
		    }
		}
		G49_SET_LINE_TEXTURE(16, rs->pt_register, rs->pt_multiplier);
		rs->first_process_me = TRUE;
		return;
	    case  0 :
		ignore_ch();		/*Unexpected non-alpha */
		break;
	    case  1 : 			/*Left paren */
		p1_pattern_register_suboption(&rs->pt_multiplier);
		break;
	    case  2 :
	    case  3 :			/*0,1 */
		if (bits_seen == -1 || bits_seen == 0)
		{		/*The last thing we saw was nothing, */
	    			/*or a predefined pattern (2 to 9): */
	    			/*smash it and start over */
		    bits_seen = 1;
#if REVERSE_PATTERN_REGISTER == TRUE
		    rs->pt_register = 0;
		    put_field((rs->character - 48), SIGN_BIT, &rs->pt_register);
					/*Move new bit to high end of pt_register */
#else
		    rs->pt_register = rs->character - 48;
					/*Move new bit to low end of pt_register */
#endif
		}
		else
		{
		    bits_seen++;
#if REVERSE_PATTERN_REGISTER == TRUE
		    rs->pt_register = rs->pt_register >> 1;	/*Shift old value right */
		    put_field((rs->character - 48), SIGN_BIT, &rs->pt_register);
					/*Move new bit to high end of pt_register */
#else
		    rs->pt_register = rs->pt_register << 1;	/*Shift old value left */
		    rs->pt_register = rs->pt_register || (rs->character - 48);
					/*Move new bit to low end of pt_register */
#endif
		}

		if (bits_seen > 16)
		{
		    bits_seen = 16;
		}
					/*Handle the extra bit case */
		break;
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :
	    case 10 :
	    case 11 :			/* 2 to 9 */
		rs->pt_register = bit_pattern[rs->character - 48];
		bits_seen = (-1);
	}
    }
}

p1_pattern_register_suboption(multiplier)
int	*multiplier;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the suboptions of the P(attern register) option of
    *   the W(rite) instruction
    *
    * FORMAL PARAMETERS:
    *   multiplier	= address in which to store the argument of
    *			  the W(P(M instruction
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		rel;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {

	switch (scan( "(M)" ))
	{
	    case -3 :			/*Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);	/*Quotation */
		break;
	    case -1 :
		skp_option();		/*Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/*Unexpected non-alpha */
		break;
	    case  1 :
		skp_paren();		/*Left paren */
		break;
	    case  2 :			/*M */
	    {
		int	m;
		number(&m, &rel, 0);
		if (m < 0)
		{
		      m = (-m);
		}
		if ( m > 16 )
		    m = 16;
		if ( m == 0 )
		    m = 1;
		*multiplier = m;
		break;
	    }
	    case  3 :
		return;			/*Right paren */
	}
    }
}

rel_coordinates()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Coordinates state - handle ordered pairs of the form [x,y]
    *   where x and y are generally floating-point and are expected to be
    *   relative (signed).  The coordinates are scaled, but not translated.
    *
    * FORMAL PARAMETERS:  	none
    *
    * IMPLICIT INPUTS:
    *   rel_x_coord	= address in which to store the x coordinate
    *   rel_y_coord	= address in which to store the y coordinate
    *   coord_relflags = address of the "relative flags word" in
    *		  which to store the logical OR of:
    *			256 = x is relative
    *			512 = y is relative
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:
    *   0 = routine terminated on ; (Resynch) (Note that
    *		first_process_me is also set in this case)
    *   1 = routine terminated on ]
    *
    * SIDE EFFECTS:		none
    **/

    int		return_value;
    int		x_exponent;
    int		x_sign;
    int		x_mantissa;
    int		y_exponent;
    int		y_sign;
    int		y_mantissa;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    return_value = x1_floating_bracketed_pair(&x_mantissa, &x_exponent,
	&y_mantissa, &y_exponent, &rs->coord_relflags, rs->sa_10_power, rs->sa_10_power);

    if (rs->sa_2_power != 0)
    {
	x_mantissa = x_mantissa << rs->sa_2_power; /* assuming sa_2_power is pos.*/
	y_mantissa = y_mantissa << rs->sa_2_power; /* assuming sa_2_power is pos.*/
    }
    if (rs->sa_x_extent < 0)
    {
	x_mantissa = (-x_mantissa);
    }
    if (rs->sa_y_extent < 0)
    {
	y_mantissa = (-y_mantissa);
    }
    if (x_exponent == 0)
    {
	rs->rel_x_coord = x_mantissa;
    }
    else
    {
	rs->rel_x_coord = convert_floating_to_integer(x_mantissa, x_exponent);
    }
    if (y_exponent == 0)
    {
	rs->rel_y_coord = y_mantissa;
    }
    else
    {
	rs->rel_y_coord = convert_floating_to_integer(y_mantissa, y_exponent);
    }
    return(return_value);
}

scan(char_plit)
char	*char_plit;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Go fetch a character (if we need one),  analyze it.
    *
    * FORMAL PARAMETERS:
    *   char_plit	- PLIT to a list of characters of interest
    *			  (terminated by NUL)
    *
    * IMPLICIT INPUTS:	first_process_me is interrogated to determine
    *   whether a new character needs to be fetched, or if the
    *   existing .CHAR should be processed.  first_process_me is
    *   always cleared by this routine.
    *
    * IMPLICIT OUTPUTS:	first_process_me is cleared; CHAR is set by
    *   the routine SCAN.
    *
    * ROUTINE VALUE:
    *   -3 = ; seen (resynch)
    *   -2 = ' or " seen (enter quotation state).
    *   -1 = the character seen was not in the PLIT and was alphabetic
    *		(in the range A to Z, ASCII codes 65 to 90).
    *   0  = the character seen was not in the PLIT and was not
    *		alphabetic.
    *   1 ff. = character position in the PLIT that was matched.
    *
    *   This value is intended to be used in a CASE statement
    *   following the routine call.
    *
    * SIDE EFFECTS:		none
    **/

    int		c;
    int		temp;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: scan, regis3.c\n", PutChar);
#endif
**/
    if (IS_TRUE(rs->first_process_me)) 		/*Get char if needed */
    {
	rs->first_process_me = FALSE;
    }
    else
    {
	rs->character = inch();
    }
    switch (rs->character)
    {
	case 59 :

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, scan, regis3.c\n", PutChar);
#endif
**/
	    return(-3);			/*Semicolon */
	case 34 :
	case 39 :

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, scan, regis3.c\n", PutChar);
#endif
**/
	    return(-2);			/*Single or double quote */
    }
    /* Now search for the char in list */

    temp = 1;

    while ((c = *char_plit) != 0)
    {
	char_plit++;
	if (c == rs->character)
	{
	    break;
	}
	temp++;
    }
    char_plit++;

    if (c == 0)			/* not found, use special return value */
    {
	if (rs->character >= 65 && rs->character <= 90)
	{
	    temp = (-1);
	}
	else
	{
	    temp = 0;
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #3, scan, regis3.c\n", PutChar);
#endif
**/
    return(temp);
}

scrn_write_intensity(write_flag)
int	write_flag;
{
    /**
    * Handle S(I and W(I
    * Input value: write_flag -- TRUE = S(I, FALSE = W(I
    **/
    /**
    * Implicit inputs:		none
    *
    * Implicit outputs:		none
    *
    * Function:
    *   Implement the I(ntensity) options of the S(creen) and W(rite)
    *   instructions.  Parse the argument to the option and issue the
    *   appropriate GIDIS opcode.
    *
    * Algorithm:		N/A
    *
    * Side effects:
    *   Issues the GIDIS opcode G36_SET_SECONDARY_COLOR or
    *   G35_SET_PRIMARY_COLOR.
    **/

    int		intensity;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    intensity = (-1);			/*Init to useless value */

    while ( TRUE )
    {

	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/*Synch, unexpected alpha, right paren */

		if (intensity == -1)	/*Did we get a value? */
		{ 			/*No - skip it */
		}
		else
		{
		    if (IS_TRUE(write_flag)) /*What's the current instruction? */
		    {     		/*Write - issue foreground opcode */
			rs->md_writing_color = intensity;
			G35_SET_PRIMARY_COLOR(rs->md_writing_color);
		    }
		    else 		/*Screen - issue background opcode */
		    {
			G36_SET_SECONDARY_COLOR(rs->md_background_color = intensity);
		    }
		}
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);
		break;
	    case  0 : 			/*Unexpected non-alpha */
	    {
		int    valu;
		int    rel;

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&valu, &rel, 0) )) 
		{
		    if (valu < 0)
		    {
			valu = 0;
		    }
		    if (valu >= ( 1 << rs->widget->common.bitPlanes ) )
		    {
		        /* value = value % TOTAL4_SIMULTANEOUS_COLORS; */
			valu %= ( 1 << rs->widget->common.bitPlanes );
		    }
		    intensity = valu;
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 : 			/*Left paren */
		is_intensity_suboption(&intensity);
		break;
	    case  2 :
		skp_coord();		/*[ */
	}
    }
}

s0_shading()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the S(hading) option of the W(rite) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		shading_argument;
    int		texture_character;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    if ( rs->sh_mode < WANT_POLYGON_FILL )
	{
	rs->sh_mode = Y_SHADE_TO;	/*Default is to y */
	shading_argument = (-1);	/*Default is pattern register shading */
	texture_character = 0;		/*Default is no shading character */
#if SEPARATE_CURRENT_POSITION
	if (IS_FALSE(rs->gid_xy_valid))
	    {
	    rpg_request_gidi_pos();
	    }
#endif
	rs->sh_x = rs->gid_x;			/*Default is current point */
	rs->sh_y = rs->gid_y;
	}

    while ( TRUE )
    {

	switch (scan( "([)" ))
	{
	    case -3 : 			/*Synch */
		rs->first_process_me = TRUE;
		return;
	    case -2 : 			/*Quotation */
		texture_character = fetch_one_character(rs->character);
		break;
	    case -1 :
	    case  3 : 			/*Unexpected alpha, right paren */
		s1_shading_action(shading_argument, texture_character);
		rs->first_process_me = TRUE;
		return;
	    case  0 :			/*Unexpected non-alpha */
	    {
		int    valu;
		int    rel;

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&valu, &rel, 0) )) 
		{
		    shading_argument = valu;
		    texture_character = 0;	/*Turn off character shading */
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 : 			/*Left paren */
		s2_shading_suboption();
		break;
	    case  2 : 			/*[ */
		coordinates();
		if ( rs->sh_mode < WANT_POLYGON_FILL )
		    {
		    rs->sh_x = rs->x_crd;
		    rs->sh_y = rs->y_crd;
		    }
	}
    }
}

s1_shading_action(shading_argument, texture_character)
int	shading_argument;
int	texture_character;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Perform GIDIS code generation for the S(hading) option of
    *   the W(rite) instruction
    *
    * FORMAL PARAMETERS:
    *   shading_argument = value:
    *				0 = shading off
    *				nonzero = shading on
    *   texture_character = value:
    *				0 = pattern register shading
    *				nonzero = shading character
    *
    * IMPLICIT INPUTS:
    *   sh_mode = word containing:
    *				SHADE_OFF
    *				Y_SHADE_TO
    *				X_SHADE_TO		or
    *				SHADE_TO_POINT
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (shading_argument == 0 && rs->sh_mode < WANT_POLYGON_FILL)
    {
	rs->sh_mode = SHADE_OFF;
	return;
    }
    if (texture_character == 0)
    {				/* pattern register shading */
	rs->sh_alphabet = -1;
	rs->sh_char = 0;
	rs->sh_width = 0;
	rs->sh_height = 0;
	G52_SET_AREA_TEXTURE(rs->sh_alphabet, rs->sh_char);
    }
    else
    {
	rs->sh_alphabet = rs->tx_alphabet;	/* character shading */
	if (rs->tx_alphabet == 0)
	{
	    rs->sh_char = texture_character;
	}
	else
	{
	    rs->sh_char = CH_FOLD(texture_character);
	}
	rs->sh_width = rs->tx_x_unit_size;
	rs->sh_height = rs->tx_y_unit_size;

	G52_SET_AREA_TEXTURE(rs->sh_alphabet, rs->sh_char);
	G53_SET_AREA_SIZE(rs->sh_width, rs->sh_height);

	if (rs->al_width[rs->sh_alphabet] == 8 && rs->al_height[rs->sh_alphabet] == 10)
	{
	    G54_SET_AREA_CELL_SIZE(8, 8);
	}
    }
}

s2_shading_suboption()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the suboptions of the S(hading) option of the W(rite)
    *   instruction
    *
    * FORMAL PARAMETERS:
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *   sh_mode	= flag to contain:
    *				SHADE_OFF
    *				Y_SHADE_TO
    *				X_SHADE_TO		or
    *				SHADE_TO_POINT
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {

	switch (scan( "(PXY)" ))
	{
	    case -3 :			/*Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);	/*Quotation */
		break;
	    case -1 :
		skp_option();		/*Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/*Unexpected non-alpha */
		break;
	    case  1 :
		skp_paren();		/*Left paren */
		break;
	    case  2 : 			/*P */
		rs->sh_mode = SHADE_TO_POINT;
		break;
	    case  3 : 			/*X */
		rs->sh_mode = X_SHADE_TO;
		break;
	    case  4 : 			/*Y */
		rs->sh_mode = Y_SHADE_TO;
		break;
	    case  5 :
		return;			/*Right paren */
	}
    }
}

skp_coord()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle [] skip state.  When an unexpected open bracket is seen,
    *   come here and wait for the matching close bracket
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		CHAR, the current ReGIS character
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:	skips past matching close bracket
    *			nothing is allowed to terminate this
    *			except ';'
    *			first_process_me is set to TRUE if we
    *			terminated on ';'
    *			note that neither quote mark has any affect
    *			on the scan for ']' or ';'
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE(rs->first_process_me))
    {
	rs->first_process_me = FALSE;
    }
    else
    {
	rs->character = INCH_Q(0);
    }
    while ((rs->character != ']') && (rs->character != ';'))
    {
	rs->character = INCH_Q(0);
    }
    if (rs->character == ';')
    {
	rs->first_process_me = TRUE;
    }
}

skp_option()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle skipping of unimplemented or otherwise unexpected
    *   options.  When an unexpected option is seen, come here and
    *   wait for the next option.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:
    *   This routine will always exit with first_process_me set and
    *   the next option character in CHAR.
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {

	switch (scan( "()" ))
	{
	    case -3 :
	    case -1 :
	    case  2 : 		/*Sync, alphabetic, or right paren */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);
		break;
	    case  0 :
		ignore_ch();		/*Nothing interesting */
		break;
	    case  1 :
		skp_paren();		/*Left paren */
	}
    }
}

skp_paren()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle() skip state.  When an unexpected left paren is seen,
    *   come here and wait for the matching right paren (or ;).
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		CHAR, the current ReGIS character
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		skips past next right paren
    **/

    int		skp_count = 0;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {

	switch (scan( "()" )) /*Only parens are of interest */
	{
	    case -3 : 			/*Sync */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);
		break;
	    case -1 :
	    case  0 : 			/*Nothing interesting */
		ignore_ch();
		break;
	    case  1 : 			/*Left paren  - count so we don't */
					/* blow the stack away */
		skp_count++;
		break;
	    case  2 : 			/*Right paren */

		if (skp_count > 0)
		{
		    skp_count--;
		}
		else
		{
		    return;
		}
	}
    }
}

skp_quote(quote_character)
int	quote_character;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Read a quoted string from the input strean, ignoring it.
    *
    * FORMAL PARAMETERS:
    *   quote_character	= value of the quotation character to which
    *   to skip
    *
    * IMPLICIT INPUTS:
    *   module own char, containing the opening quote character
    *   characters form scanners INCH_Q routine
    *
    * IMPLICIT OUTPUTS:
    *   Module wide own first_process_me is TRUE, char is next char in
    *   input stream.
    *
    * NOTE:
    *   SKIP_QUOTE( character ) with first_process_me set will, quite
    *   obviously, return immediately.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {

	if (IS_TRUE(rs->first_process_me)) 	/*Only get if necessary */
	{
	    rs->first_process_me = FALSE;
	}
	else
	{
	    rs->character = INCH_Q(0);		/* Get the next character */
	}
	if (rs->character == quote_character)
	{

	    if ((rs->character = INCH_Q(0)) == quote_character)
	    {	/* Proceed as if it were a normal character */
	    }
	    else
	    {
		p_inch(rs->character);		/* push this char back into scanner */
		return;
	    }
	}
    }
}

subtract_floating(v3_minuend_mantissa, v4_minuend_exponent,
    v5_v6_subtrahend_mantissa, se_subtrahend_exponent, v7_difference_mantissa,
    v8_difference_exponent)
int	v3_minuend_mantissa;
int	v4_minuend_exponent;
int	v5_v6_subtrahend_mantissa;
int	se_subtrahend_exponent;
int	*v7_difference_mantissa;
int	*v8_difference_exponent;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Subtract one floating number from another and return a
    *   floating difference.  Handle exponent comparability
    *   adjustment, overflow on like signs, and roundoff.
    *
    * FORMAL PARAMETERS:
    *   v3_minuend_mantissa	= value of mantissa of minuend
    *   v4_minuend_exponent	= value of exponent of minuend
    *   v5_v6_subtrahend_mantissa	= value of mantissa of subtrahend
    *   subtrahend_exponent	= value of exponent of subtrahend
    *   v7_difference_mantissa	= address in which to store mantissa
    *				  of difference
    *   v8_difference_exponent	= address in which to store exponent
    *				  of difference. Where difference =
    *				  (minuend - subtrahend).
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		exp_diff;
    int		temp;
    int		v9_temp_min_mant;
    int		v10_temp_min_exp;
    int		v11_temp_sub_mant;
    int		v12_temp_sub_exp;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /**
    *	Skip all of this foolishness if we're subtracting zero
    **/

    if (v5_v6_subtrahend_mantissa == 0)
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug1\n", PutChar);
#endif
**/
	*v7_difference_mantissa = v3_minuend_mantissa;
	*v8_difference_exponent = v4_minuend_exponent;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug1.5\n", PutChar);
#endif
**/
	return;
    }
    /**
    *	Save these so we don't smash them
    **/

    v9_temp_min_mant = v3_minuend_mantissa;
    v10_temp_min_exp = v4_minuend_exponent;
    v11_temp_sub_mant = v5_v6_subtrahend_mantissa;
    v12_temp_sub_exp = se_subtrahend_exponent;

    /**
    *	Adjust the numbers so that their exponents agree.  first try
    *	to increase the mantissa of the number with the larger
    *	exponent, , if still necessary, decrease the mantissa of
    *	the number with the smaller exponent.  These adjustments are
    *	made by multiplying or dividing the mantissa by 10, and
    *	decrementing or incrementing the corresponding exponent.
    *	if this adjustment reduces one of the mantissas to zero, it
    *	was too insignificant to worry about anyway.
    **/

    while ((exp_diff = (v10_temp_min_exp - v12_temp_sub_exp)) != 0)
    {					/*Loop until they're equal */

	if (exp_diff > 0) 		/*Minuend exponent is larger */
	{
	    if (((v9_temp_min_mant < 0) ? (-v9_temp_min_mant) : v9_temp_min_mant)
			 <= 3276)  /*Can we *10 without overflow? */
	    {			/*Yes - do so */
		v9_temp_min_mant *= 10;
		v10_temp_min_exp--;
	    }
	    else
	    {			/*No - must divide */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug2\n", PutChar);
#endif
**/
		v11_temp_sub_mant = div_10_rounded(v11_temp_sub_mant,
		    (v10_temp_min_exp - v12_temp_sub_exp));
		v12_temp_sub_exp = v10_temp_min_exp;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug3\n", PutChar);
#endif
**/
	    }			/*Subtrahend exponent is larger */
	}
	else
	{
	    if (((v11_temp_sub_mant < 0) ? (-v11_temp_sub_mant) : v11_temp_sub_mant)
				 <= 3276)  /*Can we *10 without overflow? */
	    {			/*Yes - do so */
		v11_temp_sub_mant *= 10;
		v12_temp_sub_exp--;
	    }
	    else
	    {			/*No - must divide */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug4\n", PutChar);
#endif
**/
		v9_temp_min_mant = div_10_rounded (v9_temp_min_mant,
		    (v12_temp_sub_exp - v10_temp_min_exp));
		v10_temp_min_exp = v12_temp_sub_exp;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug5\n", PutChar);
#endif
**/
	    }
	}
    }
    *v8_difference_exponent = v10_temp_min_exp;

    /**
    *	Now pretest the mantissa subtraction for overflow.  if
    *	subtracting the present pair of mantissas would cause
    *	overflow, adjust them and their exponents to have less
    *	significance.
    */

    temp = ( (v9_temp_min_mant < 0) ? (-v9_temp_min_mant) : v9_temp_min_mant )
	 + ( (v11_temp_sub_mant < 0) ? (-v11_temp_sub_mant) : v11_temp_sub_mant );
 

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug6\n", PutChar);
#endif
**/
    if ( ((get_field(v9_temp_min_mant, SIGN_BIT))  !=
	  (get_field(v11_temp_sub_mant, SIGN_BIT))) &&
	  ((temp >= 32768) || (temp < 0)));
    {
	v9_temp_min_mant = div_10_rounded(v9_temp_min_mant, 1);
	v11_temp_sub_mant = div_10_rounded(v11_temp_sub_mant, 1);
	(*v8_difference_exponent)++;
    }
    /**
    *	finally, subtract the mantissas.
    **/

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString("Subtract_floating,regis3.c,debug7\n", PutChar);
#endif
**/

    *v7_difference_mantissa = v9_temp_min_mant - v11_temp_sub_mant;

    if (*v7_difference_mantissa > 16383)
    {
	*v7_difference_mantissa /= 10;
	(*v8_difference_exponent)++;
    }
}

temp_write_options()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the W (temporary write) option of the C(urve),
    *   P(osition), S(creen), T(ext), and V(ector) instructions
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Saves the current state of the write options.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    if (IS_TRUE(rs->tmp_write_flag))  /* Are temp write ops already in effect? */
    {				/*Yes - do nothing */
    }
    else
    {			/*No previous temp write ops - */
	rs->tmp_write_flag = TRUE;	/*set flag... */
	save_state();		/*...and save state */
    }
    wi_write_instruction();	/*In either case, the routine that... */
    				/*...handles the W(rite) instruction... */
    				/*...also handles temp write ops */
}

wi_write_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the W(rite) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {

	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 : 			/*Resynch, unexpected alpha */
		rs->first_process_me = TRUE;
		return;			/* only return from W instruction */
	    case -2 :
		skp_quote(rs->character);
		break;
	    case -0 :
		ignore_ch();		/*Nothing interesting */
		break;
	    case  1 :
		w1_write_option();	/*Left paren */
		break;
	    case  2 :
		skp_coord();		/*[ */
		break;
	    case  3 :			/*Right paren */

		if (IS_TRUE(rs->tmp_write_flag))
		{		/*This is how we leave temp write mode */
		    rs->first_process_me = TRUE;
		    return;
		}
		else
		{
		    ignore_ch();
		}
	}
    }
}

w1_write_option()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the options of the W(rite) instruction
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		rel;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {

	switch (scan( "(CEFIMNPRSTV)" ))
	{
	    case -3 :			/*Resynch */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);
		break;
	    case -1 :
		skp_option();		/*Unexpected alpha */
		break;
	    case  0 :
		ignore_ch();		/*Nothing interesting */
		break;
	    case  1 :
		skp_paren();		/*Left paren */
		break;
	    case  2 :			/*C */
		rs->md_mode = G40_MODE_COMPLEMENT;
		G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing);
		break;
	    case  3 :			/*E */
		rs->md_mode = G46_MODE_ERASE;
		G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing);
		break;
	    case  4 :			/*F */
	    {
		int    valu;
		int    rel;

		if (IS_TRUE( number(&valu, &rel, 0) )) 
		{
		    G48_SET_PLANE_MASK(rs->md_plane_mask = valu);
		}
		else
		{
		    rs->first_process_me = TRUE;
		}
		break;
	    }
	    case  5 : 			/*I */
		scrn_write_intensity( TRUE );
		break;
	    case  6 : 			/*M */
		number(&rs->pix_multiplier_vector, &rel, rs->sa_10_power);
		break;
	    case  7 :			/*N */
	    {
		int    temp;
		int    rel;

		if (IS_TRUE( number(&temp, &rel, 0) ))
		{
		    if (temp == 0)
		    {
			rs->md_negative_writing = FALSE;
		    }
		    else
		    {
			rs->md_negative_writing = TRUE;
		    }
		}
		else
		{
		    rs->first_process_me = TRUE;
		}
		G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing);
		break;
	    }
	    case  8 : 			/*P */
		p0_pattern_register();
		break;
	    case  9 :			/*R */
		rs->md_mode = G44_MODE_REPLACE;
		G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing);
		break;
	    case 10 : 			/*S */
		s0_shading();
		break;
	    case 11 :			/*T */
		rs->md_mode = G38_MODE_TRANSPARENT;
		G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing);
		break;
	    case 12 :			/*V */
		rs->md_mode = G42_MODE_OVERLAY;
		G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing);
		break;
	    case 13 :			/*Right paren */
		return;	
	}
    }
}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*2 A_VESPER 21-OCT-1983 16:29:48 "fixed absolute negative bug"
*	*1 A_VESPER 14-SEP-1983 14:01:44 ""
**/
