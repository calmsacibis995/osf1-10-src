/*****	Renumber (IDENT = 'V1.61')  *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	renumber.c
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
*	Edit 061	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit 060	??-???-83	/RFD
* Translated the original bliss code into C.
*
*	Edit 059	 7-Nov-83	/AFV
* Allow input of -32768 to produce an internal value
* of -32768 instead of -32767.
*
*	Edit 058	26-Aug-83	/AFV
* Change parameter passing between all the ACCUMULATE 
* routines to use a parameter block pointed to by a global 
* register.
*
*	Edit 057	26-Aug-83	/AFV
* Split out all ACCUMULATE_... routines plus FLOATING_SCALAR 
* and FLOATING_NUMBER. Change FLOATING_NUMBER (now a global)
* to FLOAT_NUMBER to resolve conflict with x1_floating_bracketed_pair.
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
* fix a conditional compilation switch so we don't get unDefineds
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
* reversing bits in pattern register loading, especially preDefined
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
* problem following quotation (use inch, not inch_Q).  fix precedence
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
* Installed fraction handling and EXPONENT adjustment for coordinates.
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
* V instructions.  Only integers (no fractions or EXPONENTs) are
* implemented for coordinates.  Default for coordinates is absolute
* zero, not relative zero.
**/

#include "gidcalls.h"
#include "regstruct.h"

/*****			E X T E R N A L    F U N C T I O N S	         *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      MODULE */
/*	--------		-----------			      ------ */
extern
#if VARIANT == 1
    PutBreak(),			/* Flush all pending terminal output  REGIO  */
    PutChar(),			/* Queue character on terminal output REGIO  */
    PutDecimal(),		/* Output signed decimal number	      OPSYS  */
    Put_String(),		/* Output an ASCIZ string	      OPSYS  */
#endif
    scan();			/* Lookup char in list		      REGIS3 */

/*****		F U N C T I O N S    I N    T H I S    F I L E	         *****/
/*									     */
/*  FUNCTION			  DESCRIPTION			       VALUE */
/*  --------			  -----------			       ----- */
/**
*  a1_accumulate_digit()		  Add 1 digit to current number 	yes
*  a2_accumulate_exponent()	  Process E in numbers			no
*  a3_accumulate_exponent_digits()  Process E in numbers some more	no
*  a4_accumulate_fraction()	  Process fractional part		no
*  float_number(a,a,a,v)	  Get signed, non-normalized number 	yes
*  floating_scalar()		  Get unsigned, non-normalized number	yes
*  scalar(a,a,v)							yes
**/

/**
* Define linkages and macros for accessing the parameter 
* block.
**/

#define		MANTISSA	(rs->accum_block[0])
#define		EXPONENT	(rs->accum_block[1])
#define		ROUND_FLAG	(rs->accum_block[2])
#define		DIGIT_SEEN	(rs->accum_block[3])

/*#define		ACCUM_BLOCK_SIZE    4
*/
	/* these are needed by accumulate_exponent */
#define		INDEX_OF_DIGIT_SEEN    3

#define		EXP_INDEX      1

/*static	int	actual_accum_block[ACCUM_BLOCK_SIZE] = {0};
*/	/**
	* CAUTION:  accum_block is used liberally throuout this file as a
	*	global variable and as a pointer to several accumulate blocks.
	**/
/*static	int	*accum_block = {0};

Note that actual_accum_block and accum_block have been added to regis_cntx
*/

a1_accumulate_digit()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Come here with a digit in CHAR.  This routine will accumulate
    *   the digit decimally to the MANTISSA, if the digit will fit
    *   without causing overflow.  if the new digit would cause
    *   overflow, the routine rounds the MANTISSA up if appropriate,
    *   and returns a value indicating that the EXPONENT needs to be
    *   incremented if the calling routine has not already seen a
    *   decimal point.
    *
    * PARAMETERS in accum_block :
    *   MANTISSA	= MANTISSA
    *   EXPONENT	= EXPONENT
    *   rel		= a word containing:
    *				0 = absolute positive
    *				1 = relative positive
    *				2 = absolute negative
    *				3 = relative negative
    *   ROUND_FLAG	= a word that initially contains 0.
    *			  a1_accumulate_digit sets this flag to 1 when it
    *			  performs roundoff, and checks it to avoid
    *			  rounding again.
    *
    * IMPLICIT INPUTS:
    *   Digit in CHAR
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:
    *   0 = the digit was accumulated without overflow
    *   1 = the digit would have caused overflow
    *
    * SIDE EFFECTS:		none
    **/

    int		digit;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: accumulate_digit, renumber.c\n", PutChar);
#endif
**/

    digit = rs->character - 48;			/*Binary of ASCII digit */

    /* check for overflow */

    if (MANTISSA < 0) 
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, accumulate_digit, renumber.c\n", PutChar);
#endif
**/
	return(1);			/* must be -32768 */
    }
    if ((MANTISSA < 3276) || (MANTISSA == 3276 && digit <= 8))
    {				/*No - calculate and go away */

	if (MANTISSA != 0)		/*Test for zero to save a multiply */
	{	
	    MANTISSA *= 10;
	}
	MANTISSA += digit;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, accumulate_digit, renumber.c\n", PutChar);
#endif
**/
	return(0);
    }
    else
    {
	if (IS_FALSE(ROUND_FLAG))	/*Overflow - no previous rounding? */
	{
	    ROUND_FLAG = TRUE;		/*Set flag to avoid rounding again */

	    /* Does digit call for rounding? */

	    if (digit >= 5)
	    {
		MANTISSA++;
	    }
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #3, accumulate_digit, renumber.c\n", PutChar);
#endif
**/
    return(1);			/*"Overflow" return code */
}

a2_accumulate_exponent()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Come here when you've seen an "E" in a number.  This routine
    *   tests for a sign character,  calls
    *   a3_accumulate_exponent_digits to build the EXPONENT value.
    *
    * PARAMETERS in accum_block:
    *   EXPONENT	= EXPONENT already accumulated -
    *			also place in which to store answer
    *   DIGIT_SEEN	= a flag which should be set when
    *			  a digit character is processed
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    int		*save_block;
    int		exp_block[ACCUM_BLOCK_SIZE];
    int		negative_flag;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    save_block = rs->actual_accum_block;	/* save main number */
    rs->accum_block = exp_block;	/* set pointer to EXPONENT param block */

    negative_flag = FALSE;		/* Initialize to positive  */
    MANTISSA = 0;
    EXPONENT = 0;
    DIGIT_SEEN = save_block[INDEX_OF_DIGIT_SEEN];

    switch (scan( "+-" ))
    {
	case -3 : 				/*Synch ], */
	    rs->first_process_me = TRUE;
	    return;
	case -2 :
	case -1 :
	case  0 : 			/*Others */
	    rs->first_process_me = TRUE;
	    a3_accumulate_exponent_digits();
	    break;
	case  1 :
	    a3_accumulate_exponent_digits();	/*Plus sign */
	    break;
	case  2 :			/*Minus sign */
	    negative_flag = TRUE;
	    a3_accumulate_exponent_digits();
    }

    /* now put accumulated EXPONENT into the EXPONENT field */
    /* of the main number.  Ignore overflow at this point. */

    if (IS_TRUE(negative_flag))
    {
	MANTISSA = (-MANTISSA);
    }
    save_block[EXP_INDEX] += MANTISSA;
    save_block[INDEX_OF_DIGIT_SEEN] = DIGIT_SEEN;

    /* now restore pointer to main number */

    rs->accum_block = save_block;
}

a3_accumulate_exponent_digits()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   This routine builds the EXPONENT value by processing
    *   characters until it sees a terminating character.  Note
    *   that a decimal point or a (second) "E" is included in the
    *   list of terminating characters.
    *
    * PARAMETERS in accum_block:
    *   MANTISSA	= a place to store the EXPONENT
    *   ROUND_FLAG	= a flag saying when to round
    *   DIGIT_SEEN	= a flag which should be set when
    *			  a digit character is processed
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    ROUND_FLAG = FALSE;

    while ( TRUE )
    {
	int	exit;

	exit = FALSE;

	switch (scan( "0123456789" ))
	{
	    case -3 :
	    case -2 :
	    case -1 :
	    case  0 : 		/*All manner of non-digits */
		exit = TRUE;
		break;
	    case  1 :
	    case  2 :
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :
	    case 10 :
		DIGIT_SEEN = TRUE;

		if (IS_TRUE( a1_accumulate_digit() ))
					/*returns 1 if digit causes overflow */
		exit = TRUE;
	}
	if ( exit )
	{
	    break;		/* exit while ( TRUE ) */
	}
    }
    rs->first_process_me = TRUE;
    return;
}

a4_accumulate_fraction()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Come here when you've seen a decimal point.  This routine
    *   builds the fractional value by processing characters until
    *   it sees a terminating character:  one of ;],E  Other
    *   non-digit characters cause this routine to enter a skip
    *   state.  Note that a (second) decimal point is included in
    *   this list of unexpected characters.
    *
    * PARAMETERS in accum_block:
    *   MANTISSA	= MANTISSA
    *   EXPONENT	= EXPONENT
    *   ROUND_FLAG	= a word that initially contains
    *			  0.  The routine a1_accumulate_digit sets this
    *			  flag to 1 when it performs roundoff, and
    *			  checks it to avoid rounding again.
    *   DIGIT_SEEN	= a flag which should be set when
    *			  a digit character is processed
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:	none
    *
    * ROUTINE VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    while ( TRUE )
    {

	switch (scan( "0123456789" ))
	{
	    case -3 :
	    case -2 :
	    case -1 :
	    case  0 : 		/*Unexpected character */
		rs->first_process_me = TRUE;
		return;
	    case  1 :
	    case  2 :
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :
	    case 10 :
		DIGIT_SEEN = TRUE;

		if (IS_TRUE( a1_accumulate_digit() ))
		{			/*returns 1 if digit causes overflow */
		} 			/*Ignore it */
		else
		{
		    EXPONENT--;		/*Adjust EXPONENT */
		}
	}
    }
}

float_number(p_mantissa, p_exponent, p_rel, initial_exponent)
int	*p_mantissa;
int	*p_exponent;
int	*p_rel;
int	initial_exponent;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Collect and store a generally floating-point number which
    *   may be relative (signed) or absolute.  Handle integers,
    *   fractions, and EXPONENTs.
    *
    * FORMAL PARAMETERS:
    *   p_mantissa= address in which to store the MANTISSA of the answer
    *   p_exponent= address in which to store the EXPONENT of the answer
    *   p_rel	= address in which to store:
    *			0 = absolute positive
    *			1 = relative positive
    *			2 = absolute negative
    *			3 = relative negative
    *   initial_exponent
    *		= value of the power of 10 by which the (integer)
    *		  EXPONENT should be adjusted.  Negative EXPONENTs
    *		  increase the accuracy of answers; positive EXPONENTs
    *		  increase the range of answers.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:
    *   first_process_me is always set, since number scans are non-
    *   self-terminating.  The character that caused the number scan
    *   to terminate is in CHAR.
    *
    * ROUTINE VALUE:	"No error" flag
    *   0 = No number collected
    *   1 = Number collected
    *
    * SIDE EFFECTS:	sets up "accum_block" linkage to all of the
    *			ACCUM... routines.  Included in accum_block
    *			are these variables:
    *
    *				MANTISSA
    *				EXPONENT
    *				ROUND_FLAG
    *				DIGIT_SEEN
    *
    *			This routine reserves space (on the stack)
    *			for this block, initializes it and at the
    *			end puts it in the callers output locations
    *
    **/

    int		rel_flag;
    int		return_value;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: float_number, renumber.c\n", PutChar);
#endif
**/

    rs->accum_block = rs->actual_accum_block;			/* set pointer to block */

    rel_flag = 0;				/*Initialize to absolute zero */
    MANTISSA = 0;
    EXPONENT = initial_exponent;

    switch (scan( "],+-#=_" ))
    {
	case -3 :
	case -2 :
	case  1 :
	case  2 : 		/*Resynch, quotation, ], comma */
	    rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, float_number, renumber.c\n", PutChar);
#endif
**/
	    return_value = 0;			/*No number collected */
	case -1 :
	case  0 :			/*Anything else */
	    rs->first_process_me = TRUE;
	    return_value = floating_scalar();
	    break;
	case  3 :			/*plus */
	    rel_flag = 1;
	    return_value = floating_scalar();
	    break;
	case  4 :			/*minus */
	    rel_flag = 3;
	    return_value = floating_scalar();
	    break;
	case  5 :			/*# (explicit absolute positive) */
	    rel_flag = 0;
	    return_value = floating_scalar();
	    break;
	case  6 :
	case  7 :			/*=_ (absolute negative) */
	    rel_flag = 2;
	    return_value = floating_scalar();
    }
    /* save the accumulated number in the user's space */

    if (MANTISSA == -32768)
    {
	if (rel_flag >= 2) 		/* want positive or negative number? */
	{    				/* if we want negative, we have it */
	    /* do nothing */
	}
	else
	{
	    MANTISSA = 32767;		/* so make the largest positive number */
	}
    }
    else if (rel_flag >= 2)
    {
	MANTISSA = (-MANTISSA);
    }
    *p_mantissa = MANTISSA;
    *p_exponent = EXPONENT;
    *p_rel = rel_flag;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, float_number, renumber.c\n", PutChar);
#endif
**/

    return(return_value);
}

floating_scalar()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Collect and store a generally floating-point number.  This
    *   routine thinks of the number as unsigned because the calling
    *   routine, NUMBER, has already set up REL.  Handle integers,
    *   fractions, and EXPONENTs.
    *
    * PARAMETERS in accum_block:
    *
    *   MANTISSA= MANTISSA of answer
    *   EXPONENT= EXPONENT of answer
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:
    *   first_process_me is always set, since number scans are non-
    *   self-terminating.  The character tha caused the number scan
    *   to terminate is in CHAR.
    *
    * ROUTINE VALUE:	"No error" flag
    *   0 = No number collected
    *   1 = Number collected
    *
    * SIDE EFFECTS:		none
    **/

    int		e_seen;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: floating_scaler, renumber.c\n", PutChar);
#endif
**/

    DIGIT_SEEN = FALSE;
    e_seen = FALSE;
    ROUND_FLAG = FALSE;

    while ( TRUE )
    {
	int	exit;

	exit = FALSE;

	switch (scan( "0123456789.E" ))
	{
	    case -3 :
	    case -2 :
	    case -1 :
	    case  0 : 		/*Resynch or terminating character */
		rs->first_process_me = TRUE;
		exit = TRUE;
		break;
	    case  1 :
	    case  2 :
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :
	    case 10 : 		/*digit */
		DIGIT_SEEN = TRUE;

		if (IS_TRUE( a1_accumulate_digit() ))  /* accumulates 1 digit */
		{			/*returns 1 if digit causes overflow */
		    EXPONENT++;
		}
		break;
	    case 11 :			/* decimal point (.)  */
		if (IS_TRUE(e_seen))	/* have we processed an E? */
		{		/*Yes - go away */
		    rs->first_process_me = TRUE;
		    exit = TRUE;
		}
		else
		{
		    a4_accumulate_fraction();	/* accumulates as many digits */
		}				/* as are present */
		break;
	    case 12 :		/*E */
		e_seen = TRUE;
		a2_accumulate_exponent();
	}
	if ( exit )
	{
	    break; 		/* exit while ( TRUE )	*/
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, floating_scaler, renumber.c\n", PutChar);
#endif
**/
    return(DIGIT_SEEN);
}

/**
*scalar(arg, rel, initial_exponent )
*int	*arg;
*int	*rel;
*int	initial_exponent;
*{
**/ /**
*    * FUNCTIONAL DESCRIPTION:
*    *   Collect and store a generally floating-point number.  This
*    *   routine thinks of the number as unsigned because the calling
*    *   routine, NUMBER, has already set up REL.  Handle integers,
*    *   fractions, and exponents.
*    *
*    * FORMAL PARAMETERS:
*    *   arg	= address in which to store answer
*    *   rel	= address of a byte containing:
*    *			0 = absolute positive
*    *			1 = relative positive
*    *			2 = absolute negative
*    *			3 = relative negative
*    *   initial_exponent
*    *		= value of the power of 10 by which the (integer)
*    *		  exponent should be adjusted.
*    *
*    * IMPLICIT INPUTS:	none
*    *
*    * IMPLICIT OUTPUTS:
*    *   first_process_me is always set, since number scans are non-
*    *   self-terminating.  The character tha caused the number scan
*    *   to terminate is in CHAR.
*    *
*    * ROUTINE VALUE:	"No error" flag
*    *   0 = No number collected
*    *   1 = Number collected
*    *
*    * SIDE EFFECTS:		none
*    **/
/**
*    int	exponent;
*    int	mantissa;
*    int	return_value;
*
*	return_value = floating_scalar(&mantissa, &exponent, rel, 
*		initial_exponent );
*
*	*arg = convert_floating_to_integer( mantissa, exponent );
*
*	return(return_value);
*}
*
*	End of commented-out routine SCALAR
**/

/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:01:58 ""
**/
