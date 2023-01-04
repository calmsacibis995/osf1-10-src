/*****  REGIS1 (IDENT = 'V1.52')  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	regis1.c
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
* ABSTRACT:	Overlay module 1 of interpreter code:  contains color and
*		report functions.
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
* Alfred von Campe      03-May-1993     V1.2/BL2
*       - Added typecast to satisfy OSF/1 compiler.
*
* Dave Doucette          7-Apr-1993     V1.2/BL2
*	- Added check for input cursor to alter cursor characteristics
*	  in routine rpt1_action_position.
*
*	X3.0-7		 6-Sep-1990	Bob Messenger
*
* Modify rprt2_position so that it can send either a solicited or an
* unsolicited report.  This is needed to avoid a deadlock on VMS when doing
* ReGIS multi-shot input in one window and output in another window at
* the same time.
*
*	X2.0-10		 8-May-1989	Bob Messenger
* Make the default logical color for S(M) be 0, to avoid crashing.
*
*	X2.0-6		 7-Apr-1989	Bob Messenger
* Support variable number of bit Planes.  Don't use rs->cm_data.
*
*	X1.1-1		29-Jan-1989	Bob Messenger
* For R(P(I)), call G123_REQUEST_INPUT in one-shot input mode and
* G124_TRIGGER_INPUT in multiple input mode.
*
*			4-Nov-88	Eric Osman
*
* Put back the carriage return.  They just fixed VT330 to send it !
*
*			21-Oct-88	Eric Osman
* Don't send carriage return when doing one-shot input.  (Send the shot
* instead)
*
*	Edit 054	17-Feb-85	/RDM
* Get LDblock for R(P(I)) from global pointer instead of through __cprocp.
*
*	Edit 053	12-Sep-84	/RDM
* Implement R(P(I)) for Pegasus.
*
*	Edit 052	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 051	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit 050	25-Aug-83	/AFV
* Change linkage of SCAN to LINK_SCAN
*
*	Edit 049	23-Aug-83	/AFV
* Change linkage of puts_regis (and inch_q_unq and getl_regis)
*
*	Edit 048	19-Aug-83	/AFV
* Add parameter MAX_ERRORS_ALLOWED and keep asking for Gidis 
* Reports until we either get the right number of words or 
* till we get MAX_ERRORS_ALLOWED errors.  
*
*	Edit 047	14-Jun-83	/DCL & AFV
* Support variable length alphabet names.  Regis v1 edit 047.
*
*	Edit 046	14-Jun-83	/DCL & AFV
* Implement ASYNCHRONOUS_COLOR_MAP option -- synchronization with a
* color map that may be modified without the Regis parser's
* knowledge.  Regis v1 edit 046
*
* (note: Regis v1 edit 045 not applicable)
*
*	Edit 045	 1-May-83	/AFV
* Prepare for Gidis V2
*
*	Edit 044	15-Apr-83	/DCL
* fix a bug in the conditional compilation logic for
* RPRT_POSITION_INTERACTIVE.
*
*	Edit 043	6-Apr-83	/DCL
* Implement macrograph report lockout.
*
*	Edit 042	25-Mar-83	/DCL
* Whenever we modify the color map, also update the hardcopy mask.
*
*	Edit 041	22-Mar-83	/DCL
* Migrate routine c_args (color argument processor) to module REGIS3.
*
*	Edit 040	2-Mar-83	/DCL
* Implement new alternate monitor support logic - if we see the A
* suboption, modify just the color portion of the color map; if we don't
* see A, modify both the mono and color portions.
*
*	Edit 039	21-Feb-83	/DCL
* Change the call to COORDINATES in c_map to a call to the macro
* skip_coord; it was a throwaway call to COORDINATES anyway.
*
*	Edit 038	28-Jan-83	/DCL
* Rip out the GLOBAL LITERAL method of version number tracking.  The
* version number is now in the IDENT modifier of the MODULE heading.
*
*	Edit 037	20-Jan-83	/DCL
* Implement the RPRT_POSITION_INTERACTIVE conditional compilation
* switch, i.e. parse the R(P(I)) (report position interactive =
* graphics input) ReGIS command.
*
*	Edit 036	11-Jan-83	/DCL
* Implement the GID_RPRT_ROUTINE conditional compilation switch, i.e.
* allow GID_REPORT, the GIDIS report mechanism, to be either an external
* routine or a macro.
*
*	Edit 035	14-Oct-82	/DCL
* Stole color and reports code from module REGIS to make this overlay
* module.  Note that more color code is in the file HLSRGB.BLI,
* module name COLrgb, where r, g, and b are the number of bits of
* red, green, and blue respectively.
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
#include <X11/cursorfont.h>

/*****		     E X T E R N A L   F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      MODULE */
/*	--------		-----------			      ------ */
extern
	c_args(),		/* Parse color specifiers	      REGIS3 */
	get_field(),		/* Retrieve a bit field		      FIELDS */
	put_field(),		/* Store a bit field		      FIELDS */
	get_gireport(),	/*				      REPORT */
	ignore_ch(),		/* Ignore erroneous character	      REGIS3 */
	mgm_report(),		/* Report entry point for MGMM	      MGMM   */
	number(),		/* Number parser entry point	      REGIS3 */
	puts_regis(),		/* String output service	      SCAN   */
#if VARIANT == 1
	PutBreak(),		/* Flush all pending terminal output  REGIO  */
	PutChar(),		/* Queue character on terminal output REGIO  */
	PutDecimal(),		/* Output signed decimal number	      OPSYS  */
	Put_String(),		/* Output an ASCIZ string	      OPSYS  */
#endif
#if SEPARATE_CURRENT_POSITION
	rpg_request_gidi_pos(),	/* Request and read current -	      REPORT */
#endif				/*	GIDIS position		     	     */
	scan(),			/* Character fetch		      REGIS3 */
	skp_option(),		/* Option letter skip state	      REGIS3 */
	skp_paren(),		/* Parenthesis skip state	      REGIS3 */
	skp_quote(),		/* Quotation skip state		      REGIS3 */
	skp_coord();		/* [] skip			      REGIS3 */
#if ASYNCHRONOUS_COLOR_MAP
extern
	update_color_map();
#endif

/*****		F U N C T I O N S   I N   T H I S   F I L E		 *****/
/*									     */
/*	FUNCTIONS				DESCRIPTION		     */
/*	---------				-----------		     */
/**
*	c_map()					Process S(M
*	color_map_suboption(v)			Process S(Md(
*	binary_to_decimal_convert(v,v,a,a)
*	rprt_alphabet()				Process R(L
*	rprt_error()				Process R(E
*	rprt_instruction()			Process R
*	rprt_macrographs(a)			Process R(M
*	r2_rprt_macrographs_suboption(a)	Process R(M(
*	rprt_option()				Process R(
*	rprt_position()				Process R(P
*	rprt1_action_position(v)		Generate GIDIS for R(P and R(P(I
*	rprt_suboption_position()		Process R(P(
*	set_color_map(v,v,v)
**/

c_map()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the M(ap) option of the S(creen) instruction
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
    *   Issues the GIDIS opcode G55_SET_COLOR_MAP_ENTRY
    **/

    int		logical_color;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n\n enter c_map,regis1.c,debug000\n", PutChar);
#endif
**/
#if ASYNCHRONOUS_COLOR_MAP

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug1\n", PutChar);
#endif
**/
    update_color_map();
#endif

    logical_color = 0;	/* in case no number is given */

    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 : 			/* Resynch */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n exit c_map,regis1.c,debug101\n", PutChar);
#endif
**/
		return;
	    case -2 :

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug2\n", PutChar);
#endif
**/
		skp_quote( rs->character );
		break;
	    case -1 :
	    case  3 : 			/* New option, right paren */
	    {
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n exit c_map,regis1.c,debug102\n", PutChar);
#endif
**/
		return;
	    }
	    case  0 : 			/* Unexpected non-alpha - number? */
	    {

		int	temp;
		int	rel;

		rs->first_process_me = TRUE;


    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug5\n", PutChar);
#endif
**/
		if (number(&temp, &rel, 0) != FALSE ) /* Yes - 	*/
		{			/*        - set color map entry */

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug6\n", PutChar);
#endif
**/
		    if (temp < 0)
		    {
			temp = 0;
		    }
		    if (temp >= ( 1 << rs->widget->common.bitPlanes ) )
		    {
			temp %= ( 1 << rs->widget->common.bitPlanes );
					 /* temp=temp MOD TO... */
		    }
		    logical_color = temp;
		}
		else 			/* No */
		{

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug7\n", PutChar);
#endif
**/
		    ignore_ch();
		}
		break;
	    }
	    case  1 : 			/* Left paren */

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug8\n", PutChar);
#endif
**/
		cps_color_map_suboption( logical_color );
		break;
	    case  2 :

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n c_map,regis1.c,debug9\n", PutChar);
#endif
**/
		skp_coord();
	}

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n end switch c_map,regis1.c,debug10\n", PutChar);
#endif
**/
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\n end c_map,regis1.c,debug100\n", PutChar);
#endif
**/
}

cps_color_map_suboption(logical_color)
int	logical_color;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the suboptions of the M(ap) option of the S(creen)
    *   instruction.
    *
    * FORMAL PARAMETERS:
    *   logical_color	= value of the current plane selector register
    *			  (0 to 3)
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		red;
    int		green;
    int		blue;
    int		mono;
    int		cmn_color_map_number;	/* -1 = primary color map [color + mono] */
    					/*  0 = first alternate [color] */
					/*  1 = second alternate [color + mono] */
					/*  ... */
					/*  NUMBER_COLOR_MAPS - 1 */

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /*  process color specifier but quit if nothing there */


    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: color_map_suboption, regis1.c\n", PutChar);
#endif
**/
    if ( IS_FALSE(c_args(&red,&green,&blue,&mono,&cmn_color_map_number)) )
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, color_map_suboption, regis1.c\n", PutChar);
#endif
**/
	return;
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
  PutString("\ncolor_map_suboption,regis1.c,debug1.01\n", PutChar);
#endif
**/
    /**
    * Alternate monitor support algorithm - S(Mn(Ahls))
    *
    * if we DO NOT support an alternate monitor,  we always move the entire
    * rgb_mono_word into the color map entry.
    *
    * if we DO support an alternate monitor,  we interrogate
    * which_color_map, which is set if we have seen an A, clear otherwise.  if
    * we have seen an A, we load only the color bits of rgb_mono_word into the
    * color (alternate) bits of the color map entry, and leave the mono bits
    * unaffected.  if we have not seen an A, we move the entire rgb_mono_word
    * into the color map entry.
    *
    * It is not possible to set just the mono bits without also setting the
    * color bits  This restriction is consistent with the concept of the
    * monochrome monitor as the "primary" monitor and the color monitor as the
    * "secondary" monitor:  all monitors' color maps are set to the primary
    * setting, while alternate monitors' maps may be set independently if
    * explicitly specified.
    **/

/* in DECterm, don't set the color map if we're on a monochrome system and
   the alternate color map was changed */

	if ( cmn_color_map_number >= 0 && ! rs->color_monitor )
	    return;

    /**
    * Set up mask for hardcopy based on monochrome portion of color map
    **/


    if (mono == 0)
    {    

	rs->hd_mask &= ~( 1 << logical_color);
    }
    else
    {

	rs->hd_mask |= ( 1 << logical_color);
    }

    G55_SET_COLOR_MAP_ENTRY( logical_color, red, green, blue, mono );
}

binary_to_decimal_convert(input, exponent, output, out_len)
int	input;
int	exponent;
int	*out_len;
char	*output;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   This routine decomposes a 16-bit, signed, two's complement
    *   integer (or integer plus 16-bit, signed, two's complement
    *   exponent) into a block of ASCII digit characters.  The result
    *   is signed and leading-zero suppressed.
    *
    * FORMAL PARAMETERS:
    *   input	= value of number to be converted
    *   exponent= value of exponent to be converted.  No exponent
    *		  conversion is done if this value is 0.
    *   output	= location of {ning of block in which to put the
    *		  output string.  It is advisable to allow 7 bytes
    *		  if exponent = 0, or 14 bytes if exponent <> 0.
    *   out_len = location in which to store number of
    *		  characters written to the output block.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

#define	MAXDIGITS	5	/* max number of digits before using E notation */
#define	MAXZEROS	0	/* max num of leading zeros before E notation */


    char	*pointer;
    int		e_part;
    int		size;
    int		digit;
    int		in_fraction;
    int		index;
    long	int		temp;

/* This declaration and definition should be bound (it can be stored in ROM) */
    static int	power_table[] = {1, 10, 100, 1000, 10000};

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

#if VARIANT == 1
    Put_String( ("\n [convert input="), PutChar );
    PutDecimal( input, PutChar );
    PutChar( 'E' );
    PutDecimal( exponent, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    if (input == 0)		/* Zero is a special case */
    {
	output[0] = '0';
	*out_len = 1;		/* Only one character in output string */
	return;
    }
    if (input == -32768)
    {
	input = -32767;
    }
    pointer = output;

    if (input < 0)		/* Negative? */
    {				/* Yes - negate it and output a '-' */
	input = (-input);
	*pointer = '-';
	pointer++;
    }
    in_fraction = FALSE;

    /* calculate the number of digits in the mantissa */

    for (index = 4; index >= 0; index--)
    {
	size = index;
	if (input >= power_table[index])
	{
	    break;
	}
    }
    size++;		

#if VARIANT == 1
    Put_String( (" ["), PutChar );
    PutDecimal( size, PutChar );
    Put_String( (" digits] "), PutChar );
    PutBreak();
#endif

    /* decide if we need to use scientific notation 0.xxxxxEyy */

    if ((size + exponent) > MAXDIGITS) 	/* xxxxx000000 -> 0.xxxxxEnnn */
    {
	e_part = size + exponent;
	exponent = size;
    }
    else if ((size + exponent) < (-MAXZEROS)) /* 0.00000xxxx -> 0.xxxxxE-nnn */
    {
	e_part = size + exponent;
	exponent = size;
    }
    else			/* no Exxx part, therefore xxx.xxxx */
    {
	exponent = (-exponent);	/* exponent now has the position  */
	e_part = 0;		/* of the decimal point */
    }
#if VARIANT == 1
    Put_String( (" [e_part = "), PutChar );
    PutDecimal( e_part, PutChar );
    Put_String( (", exponent = "), PutChar );
    PutDecimal( exponent, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    /* now output '0.' if necessary */

    if (exponent >= size) 	/* need 0. and maybe some leading zeros */
    {
	*pointer = '0';
	pointer++;
	*pointer = '.';
	pointer++;
	in_fraction = TRUE;
#if MAXZEROS > 0
	for (index = exponent - size - 1; index >= 0; index--)
	{
	    *pointer = '0';
	    pointer++;
	}
#endif
    }
    /* now output the mantissa, including the decimal point where needed */

    temp = input;

#if VARIANT == 1
    Put_String( (" ["), PutChar );
    PutBreak();
#endif

    for (index = size - 1; index >= exponent; index--)	/* integral part */
    {
	digit = temp/power_table[index];
	temp %= power_table[index];
	*pointer = '0' + digit;
	pointer++;
#if VARIANT == 1
    	Put_String( (" '"), PutChar );
    	PutDecimal( digit , PutChar );
	PutBreak();
#endif
    }
    if (temp != 0) 			/* have a fractional part */
    {

	if (IS_FALSE( in_fraction ))
	{
	    *pointer = '.';
	    pointer++;
	    in_fraction = TRUE;
#if VARIANT == 1
	    Put_String( (" ."), PutChar );
	    PutBreak();
#endif
	}
	for (index = exponent - 1; index >= 0; index--)  /* integral part */
	{
	    digit = temp/power_table[index];
	    temp %= power_table[index];
	    *pointer = '0' + digit;
	    pointer++;
#if VARIANT == 1
	    Put_String( (" "), PutChar );
	    PutDecimal( digit , PutChar );
	    PutBreak();
#endif
	    if (temp == 0)
	    {
		break;
	    }
	}
    }
    /* now output Ennn if needed */

    if (e_part != 0)
    {
	/* force a decimal point if none there yet */

	if (IS_FALSE( in_fraction ))
	{
	    *pointer = '.';
	    pointer++;
	}
	*pointer = 'E';
	pointer++;

	if (e_part < 0)
	{
	    *pointer = '-';
	    pointer++;	
	    e_part = (-e_part);
	}
	for (index = 4; index >= 0; index--)
	{
	    size = index;
	    if (e_part > power_table[index])
	    {
		break;
	    }
	}
	temp = e_part;

	for (index = size; index >= 0; index--)
	{
	    digit = temp/power_table[index];
	    temp %= power_table[index];
	    *pointer = '0' + digit;
	    pointer++;
	}
    }
#if VARIANT == 1
    Put_String( ("]\n\0"), PutChar );
    PutBreak();
#endif

    /* store length of string in user provided storage */

    *out_len = ( pointer - output );

#if VARIANT == 1
    Put_String( (" [length = "), PutChar );
    PutDecimal( *out_len, PutChar );
    Put_String( (", output[0]= "), PutChar );
    PutDecimal( output[0], PutChar );
    Put_String( (", output[len-1] = "), PutChar );
    PutDecimal( output[*out_len - 1], PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif
}

rprt_alphabet()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the L(oadable alphabet) option of the R(eport)
    *   instruction
    *   The data reported are (A"<alphabet-name>")
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
    PUT_REPORT( ("(A\""), 3 );
PUT_REPORT(&rs->al_name[rs->ld_alphabet*TOTAL1_CHARS_PER_ALPH_NAME],rs->al_length[rs->ld_alphabet] );
    PUT_REPORT( ("\")"), 2 );
    PUT_REPORT( "\r", 1 );	/* PUT_REPORT( CARRIAGE_RETURN ) */
}

#if ERROR_RPRT_ENABLED
rprt_error()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the E(rror) option of the R(eport) instruction
    *   The data reported are "<err_code>,<err_char>"
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

    char	temp_block[7];
    int		temp_length;
    int		temp;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    PUT_REPORT( ( "\"" ), 1 );

    for (temp = 0; temp <= 6; temp++)
    {
	temp_block[temp] = 0;
    }
    binary_to_decimal_convert( rs->err_code, 0, temp_block, &temp_length );
    PUT_REPORT( temp_block, temp_length );
    PUT_REPORT( ( "," ), 1 );

    for (temp = 0; temp <= 6; temp++)
    {
	temp_block[temp] = 0;
    }
    binary_to_decimal_convert(rs->err_char, 0, temp_block, &temp_length);
    PUT_REPORT( temp_block, temp_length );
    PUT_REPORT( ( "\"" ), 1 );
    PUT_REPORT( "\r", 1 );	/* <CR> *//* PUT_REPORT( CARRIAGE_RETURN ) */
}
#endif

ri_rprt_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the R(eport) instruction
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
	switch (scan( "([" ))
	{
	    case -3 :
	    case -1 : 			/* Sync, unexpected alpha */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :
		ignore_ch();		/* Unexpected non-alpha */
		break;
	    case  1 :
		rprt_option();	/* Left paren */
		break;
	    case  2 :
		skp_coord();		/* [ */
	}
    }
}

rprt_macrographs(rprt_needed)
int	*rprt_needed;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the M(acrographs) option of the R(eport) instruction
    *
    * FORMAL PARAMETERS:
    *   rprt_needed = address of a word to be set to FALSE if we
    *			 send a report
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
	    case -1 :
	    case  3 : 		/* Sync, unexpected alpha, right paren */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case  0 :
		ignore_ch();		/* Unexpected non-alpha */
		break;
	    case  1 :
		r2_rprt_macrographs_suboption( rprt_needed );	/* Left paren */
		break;
	    case  2 :
		skp_coord();		/* [ */
	}
    }
}

r2_rprt_macrographs_suboption( rprt_needed )
int	*rprt_needed;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the suboptions of the M(acrographs) option of the
    *   R(eport) instruction
    *   The data reported are @=<keyletter><macrograph contents>@;
    *   and (<free>,<total>)
    *
    * FORMAL PARAMETERS:
    *   rprt_needed = address of a word to be set to FALSE if we
    *			send a report
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		free_space;
    int		v1_macrograph_char;
    int		v2_macrograph_index;
    int		pointer;
    int		total_space;
    int		temp_length;
    char	temp_block[7];
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {
	switch (scan( "([=)" ))
	{
	    case -3 :			/* Sync */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 :			/* Unexpected alpha - report on the */
					/* corresponding macrograph */
		if (IS_TRUE( MACROGRAPH_RPRT_OK(0) ))
		{
		    v2_macrograph_index = rs->character - 65;
		    PUT_REPORT( ( "@=" ), 2 );
		    PUT_REPORT( &rs->character, 1 );
		    temp_length = rs->mg_lengths[v2_macrograph_index];
		    if ( temp_length > 1 )
			PUT_REPORT( rs->mg_begin[v2_macrograph_index],
			  temp_length - 1);
					/* don't include trailing null */
		    PUT_REPORT( ( "@;" ), 2 );
		    PUT_REPORT( "\r", 1 );/* PUT_REPORT( CARRIAGE_RETURN ) */
		    *rprt_needed = FALSE;
		}
		break;
	    case  0 :
		ignore_ch();		/* Unexpected non-alpha */
		break;
	    case  1 :
		skp_paren();		/* Left paren */
		break;
	    case  2 :
		skp_coord();		/* [ */
		break;
	    case  3 :			/* = - report on macrograph space */
	    {
		int	temp;

		mgm_report( &free_space, &total_space );
		PUT_REPORT( ( "\"" ), 1 );
		for (temp = 0; temp <= 6; temp++)
		{
		    temp_block[temp] = 0;
		}
		binary_to_decimal_convert( free_space, 0, temp_block,
			 &temp_length );
		PUT_REPORT( temp_block, temp_length );
		PUT_REPORT( ( "," ), 1 );
		for (temp = 0; temp <= 6; temp++)
		{
		    temp_block[temp] = 0;
		}
		binary_to_decimal_convert( total_space, 0, temp_block,
		    &temp_length );
		PUT_REPORT( temp_block, temp_length );
		PUT_REPORT( ( "\"" ), 1 );
		PUT_REPORT( "\r", 1 );/* PUT_REPORT( CARRIAGE_RETURN ) */
		*rprt_needed = FALSE;
		break;
	    }
	    case  4 :
		return;			/* Right paren */
	}
    }
}

rprt_option()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the options of the R(eport) instruction
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

    int		rprt_needed;		/* TRUE = option letter seen but no */
    					/*  report sent yet */
    					/* FALSE = no option letter seen yet */
    					/*  or report already sent */
    int		rel;			/* relative flag (dummy) */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    rprt_needed = FALSE;

    while ( TRUE )
    {
	switch (scan( "([ELMP)I" ))/* Sync */
	{
	    case -3:
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 : 			/* Unexpected alpha */
		if (IS_TRUE( rprt_needed ))
		{
		    PUT_REPORT( "\r", 1 );	/* PUT_REPORT( CARRIAGE_RETURN ) */
		}
		rprt_needed = TRUE;
		skp_option();
		break;
	    case  0 : 			/* Unexpected non-alpha */
		ignore_ch();
		break;
	    case  1 : 			/* Left paren */
		skp_paren();
		break;
	    case  2 : 			/* [ */
		skp_coord();
		break;
	    case  3 : 			/* E */
		if (IS_TRUE( rprt_needed ))
		{
		    PUT_REPORT( "\r", 1 ); /* PUT_REPORT( CARRIAGE_RETURN ) */
		}
		if (IS_TRUE( ERROR_RPRT_ENABLED ))
		{
		    rprt_error();
		    rprt_needed = FALSE;
		}
		else
		{
		    rprt_needed = TRUE;
		}
		break;
	    case  4 : 			/* L */
		if (IS_TRUE( rprt_needed ))
		{
		    PUT_REPORT( "\r", 1 );	/* PUT_REPORT( CARRIAGE_RETURN ) */
		}
		rprt_alphabet();
		rprt_needed = FALSE;
		break;
	    case  5 : 			/* M */
		if (IS_TRUE( rprt_needed ))
		{
		    PUT_REPORT( "\r", 1 ); /* PUT_REPORT( CARRIAGE_RETURN ) */
		}
		rprt_needed = TRUE;
		rprt_macrographs( &rprt_needed );
		break;
	    case  6 : 			/* P */
		if (IS_TRUE( rprt_needed ))
		{
		    PUT_REPORT( "\r", 1 ); /* PUT_REPORT( CARRIAGE_RETURN ) */
		}
#if RPRT_POSITION_INTERACTIVE
		rprt_position();
#else
		rprt1_action_position( FALSE );
#endif
		rprt_needed = FALSE;
		break;
	    case  7 : 			/* Right paren */
		if (IS_TRUE( rprt_needed ))
		{
		    PUT_REPORT( "\r", 1 ); /* PUT_REPORT( CARRIAGE_RETURN ) */
		}
		return;
	    case 8 :			/* I */
		switch ( scan( "([)" ))
		{
		    case -3:		/* Resynch */
		    case -1:		/* unexpected alpha */
		    case 3:		/* right paren */
			rs->first_process_me = TRUE;
			return;
		    case -2:		/* quote */
			skp_quote( rs->character );
			break;
		    case 0:		/* unexpected non-alpha */
			rs->first_process_me = TRUE;
			if ( rs->input_mode == 1 )
			    G122_DISABLE_INPUT();
			if (number(&rs->input_mode, &rel, 0) != FALSE )
			    {
			    if ( rs->input_mode == 1 )
				G121_ENABLE_INPUT();
			    else
				rs->input_mode = 0;
			    PUT_REPORT( "\r", 1 );
			    }
			else
			    ignore_ch();
			break;
		    case 1:		/* left paren */
			skp_option();
			break;
		    case 2:		/* left bracket */
			skp_coord();
			break;
		}
	}
    }
}

#if RPRT_POSITION_INTERACTIVE
rprt_position()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the P(osition) option of the R(eport) instruction
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

    int		suboption_flag = FALSE;	/* Suboption seen flag */
    					/*  0 = no suboption seen; execute */
    					/*      current position report on */
    					/*      right paren */
    					/*  1 = suboption seen; don't execute */
    					/*      current position report on */
    					/*      right paren */

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 : 			/* Sync */
		rs->first_process_me = TRUE;
		return;
		break;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 : 			/* Unexpected alpha */
		rprt1_action_position( FALSE );
		rs->first_process_me = TRUE;
		return;
		break;
	    case  0 : 			/* Unexpected non-alpha */
		ignore_ch();
		break;
	    case  1 : 			/* Left paren */
		suboption_flag = TRUE;
		rprt_suboption_position();
		break;
	    case  2 : 			/* [ */
		skp_coord();
		break;
	    case  3 : 			/* Right paren */
		if (IS_FALSE( suboption_flag )) /*  Have we seen a suboption? */
		{
		    rprt1_action_position( FALSE );
		}
					/* No - report current position */
		rs->first_process_me = TRUE;
		return;
	}
    }					/* End of while loop */
}					/* End of routine */
#endif


rprt1_action_position( interactive )	/* Interactive flag (VI) */
int	interactive;			/*  0 = report current position */
{    					/*  1 = perform interactive graphics */
    					/*      input function and report */
    					/*      resulting position */
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Generate GIDIS for the P(osition) option of the R(eport)
    *   instruction and for the I(nteractive) suboption of the P(osition)
    *   option of the R(eport) instruction
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *   Issues the GIDIS opcode gidi_rprt_position or
    *   gidi_rprt_pos_interactive.
    *
    *	Note that the TRUE setting of the argument is meaningful only if
    *   the RPRT_POSITION_INTERACTIVE conditional compilation switch is
    *   also TRUE.
    **/

    int   	  x, y;			/* coordinates in Gidis space */
    register struct regis_cntx 
		 *rs;			/* pointer to context block */
    void        (*hold_handler)();	/* hold pointer to motion callback  */
    Cursor	cursor;			/* Used for ReGIS input		    */
    unsigned int  cursor_definition;	/* Used to allow user/developer :-) 
					   to define cursor		    */
    char 	 *str;			/* Scratch pointer to getenv value  */

    rs = RSTRUCT;			/* point to ReGIS structure */

    if (  ( str = (char *) getenv("DECW$DECTERM_REGIS_CURSOR" ) ) != 0 )
	{
	cursor_definition = atoi( str );
	}
    else
	{
	cursor_definition = XC_diamond_cross;
	}

#if RPRT_POSITION_INTERACTIVE
    if (IS_TRUE( interactive ))
    {
	if ( rs->input_mode == 0 )
	    {	/* one shot input mode */
		/* Check to see if a motion handler has been defined.  If
		/* so, update the handler and put it back when you're done.
		/* */
	    if ( rs->cursor_motion_handler != NULL )
		{
		rs->cs_rband_active = TRUE;
		hold_handler	    = rs->widget->input.motion_handler;
		i_enable_motion( rs->widget, rs->cursor_motion_handler );
		(*rs->cursor_motion_handler)();	/* Force the 1st call */
		}
	    else
		{
		/* change cursor */
		cursor = XCreateFontCursor( XtDisplay( rs->widget ),
					    cursor_definition );
		if ( (cursor != BadValue) || (cursor != BadAlloc) )
		    XDefineCursor( XtDisplay( rs->widget ), 
				   XtWindow( rs->widget ), cursor );
		}
	    G123_REQUEST_INPUT();	/* block until we get some input */

	    if ( rs->cursor_motion_handler != NULL )
		{
		rs->cs_rband_active = FALSE;
		(*rs->cursor_cleanup_handler)(); /* Clean up afterwards */
		i_enable_motion(  rs->widget, hold_handler );
		}
	    else
		{
		/* Put the cursor back */
		XUndefineCursor( XtDisplay( rs->widget ), 
				 XtWindow(  rs->widget ) );
		}
	    }
	else
	    {	/* multiple input mode. */
	    G124_TRIGGER_INPUT(0);
	    }
	return;
    }
    else 

#endif
    {
#if SEPARATE_CURRENT_POSITION
	if (IS_FALSE( rs->gid_xy_valid ))
	{
	    rpg_request_gidi_pos();
	}
#endif
	x = rs->gid_x;
	y = rs->gid_y;
    }
    rprt2_position( x, y, TRUE );
}

rprt2_position( x, y, solicited )
    int x, y;		/* position to report in Gidis space */
    int solicited;	/* TRUE means the report is sent in response to a
				request from the application.
			   FALSE the report is sent as a result of user
				action. */
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Actually generate a position report, whether interactive or not
    *   The data reported are [<x-coord>,<y-coord>]<CR>
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:
    *	Call PUT_REPORT to actually send the report.
    **/

    char	temp_block[200];
    char	*rptr;
    int		temp_length;
    int		blk_coord_block[3];
    int		temp;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    rptr = temp_block;
    *rptr++ = '[';			/* Convert GIDIS coordinates back */
    					/* to ReGIS coordinates per S(A */
    if (rs->sa_x_extent >= 0)
    {
	x += rs->sa_ulx;
    }
    else
    {
	x = (-rs->sa_ulx) - x;
    }
    if (rs->sa_2_power != 0)
    {
	x = x >> rs->sa_2_power;
    }
    for (temp = 0; temp <= 13; temp++)
    {
	rptr[temp] = 0;
    }
    binary_to_decimal_convert(x, (-rs->sa_10_power), rptr, &temp_length);
    rptr += temp_length;
    *rptr++ = ',';

#if VARIANT == 1
    Put_String( (" [r(p) sa_y_extent = "), PutChar );
    PutDecimal( rs->sa_y_extent, PutChar );
    Put_String( (" sa_uly = "), PutChar );
    PutDecimal( rs->sa_uly, PutChar );
    Put_String( (" input y = "), PutChar );
    PutDecimal( y, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    y = (rs->sa_y_extent >= 0) ? (rs->sa_uly + y) : ((-rs->sa_uly) - y);

#if VARIANT == 1
    Put_String( (" [r(p) output y = "), PutChar );
    PutDecimal( y, PutChar );
    Put_String( ("] "), PutChar );
    PutBreak();
#endif

    if (rs->sa_2_power != 0)
    {
	y = y >> rs->sa_2_power;
    }
    for (temp = 0; temp <= 13; temp++)
    {
	rptr[temp] = 0;
    }
    binary_to_decimal_convert(y, (-rs->sa_10_power), rptr, &temp_length);
    rptr += temp_length;
    *rptr++ = ']';
    *rptr++ = '\r';
    if ( solicited )
	i_report_data( rs->widget, temp_block, rptr - temp_block );
    else
	i_send_unsolicited_report( rs->widget, temp_block, rptr - temp_block );
}

#if RPRT_POSITION_INTERACTIVE
rprt_suboption_position()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Handle the suboptions of the P(osition) option of the
    *   R(eport) instruction
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
	switch (scan( "([I)" ))
	{
	    case -3 : 			/* Sync */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote( rs->character );
		break;
	    case -1 : 			/* Unexpected alpha */
		skp_option();
		break;
	    case  0 : 			/* Unexpected non-alpha */
		ignore_ch();
		break;
	    case  1 : 			/* Left paren */
		skp_paren();
		break;
	    case  2 : 			/* [ */
		skp_coord();
		break;
	    case  3 : 			/* I */
		rprt1_action_position( TRUE );
		break;
	    case  4 : 			/* Right paren */
		return;
	}
    }					/* End of while loop */
}					/* End of routine */
#endif

/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:01:31 ""
**/
