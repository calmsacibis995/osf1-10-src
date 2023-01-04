/*****	REGIS2 (IDENT = 'V1.30') *****/
/* #module <module name> "X0.0" */
/*
 *  Title:	regis2.c
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
 */

/**
* REGIS2.BLI, text command parse overlay module of ReGIS interpreter
*	-- stolen from: --
* TEXT.REQ, Text command parse of REGIS Interpreter
**/
/**
* FACILITY:	Common ReGIS interpreter for VT200 and CT100
*
* ABSTRACT:	Overlay module 2 of interpreter code:  contains text functions.
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	J. A. Lomicka		CREATION DATE:	7-July-1981
*
* MODIFICATION HISTORY (please update IDENT above):
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*  Aston Chan		17-Dec-1991	V3.1
*	- I18n code merge
*
*  Bob Messenger        17-Jul-1990     X3.0-5
*      Merge in Toshi Tanimoto's changes to support Asian terminals -
*	- character set designation option
*	- mixed 1/2 byte characters processing
*
*  Bob Messenger	 1-Jun-1989	X2.0-13
*	- Update escapement correctly in ucs_update_cell_standard.
*
*  Bob Messenger	28-May-1989	X2.0-13
*	- Treat character height of 0 as 2, and limit height to 25 instead
*	  of 256 (to prevent system hang).
*
*  Bob Messenger	11-May-1989	X2.0-10
*	- Make standard character sizes compatible with the VT340.
*
* --------------- end of DECterm edits ----------------------------
* 030	 8-May-84	/RDM
* Make regis reentrant by including regstuct.h
*
* 029	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
* 028	14-Oct-83	/AFV						   2
* ask GIDIS for standard cell sizes after S(A) command and never	   2
* again.  Save those sizes in global storage				   2
*									   2
* 027	25-Aug-83	/AFV
* Change linkage for SCAN
*
* 026	23-Aug-83	/AFV
* Change tx_x_escapement and tx_y_escapement to account for 
* the 2.5:1 pixel aspect ratio on the Pro.  To aid in 
* portability the actual code is in a MACRO defined in 
* REGDEVDEP.REQ or whatever that calls. This macro's name is
* ADJUST_45_TEXT_Y.
*
* 025	23-Aug-83	/AFV
* Change linkage for inch_q_unq (and puts_regis and getl_regis)
*
* 024	22-Aug-83	/AFV
* Standard size now always asks for cell standard for 
* alphabet 0
*
* 023	19-Aug-83	/AFV
* New parameter: MAX_ERRORS_ALLOWED.  Keep trying to get the 
* right number of words back from a Gidis report, up to 
* MAX_ERRORS_ALLOWED tries
*
* 022	18-Aug-83	/AFV
* Add new parameter -- ASK_FOR_TEXT_ESCAPEMENT.  
*
* 021	18-Aug-83	/AFV
* Change standard size stuff to always ask for standard size 
* in direction 0.
*
* 020	11-Aug-83	/AFV
* Change "send_gidis" to "G_SEND_GIDIS"
*
* 019	 9-Aug-83	/AFV
* use the new global SC_CURRENT_OPCODE to properly flush end 
* list terminated DRAW_CHARACTERS commands which go over
* buffer boundaries.
*
* 018	14-Jun-83	/DCL & AFV
* implement backspace and horizontal tab (Regis V1 edit 017)
*
* 017	 2-May-83	/AFV
* Get ready for GIDIS v2
*
* 016	27-Apr-83	/DCL
* fix a coord_always_relative bug.
*
* 015	24-Mar-83	/DCL
* G_rprt_CELL_STANDARD now returns four words:  unit width, unit
* height, display width, and display height.  Do the correct thing with
* these.
*
* 014	3-Mar-83	/DCL
* fix standard size zero to be half height and full width, not half height
* and half width.
*
* 013	21-Feb-83	/DCL
* Change throwaway calls to routine COORDINATES to calls to the macro
* skip_coord.  Change non-throwaway calls to routine COORDINATES to support
* the argument block method of calling that routine.
*
* 012	21-Feb-83	/DCL
* Rip out references to tx_h; we no longer need to store it.  Also rip out
* occurances of first_process_me = FALSE; they are superfluous.  Also take
* absolute value of arguments to G_CELL_UNIT_SIZE and
* G_CELL_DISPLAY_SIZE to avoid inadvertent cell mirroring in
* non-fourth-quadrant screen addressing environments.
*
* 011	8-Feb-83	/DCL
* Implement T(B)...(E); fix T(M) and T(H).
*
* 010	28-Jan-83	/DCL
* Rip out the GLOBAL LITERAL method of version number tracking.  The
* version number is now in the IDENT modifier of the MODULE heading.
*
* 009	11-Jan-83	/DCL
* Implement the GID_RPRT_ROUTINE conditional compilation switch, i.e. allow
* GID_REPORT, the GIDIS report mechanism, to be either an external routine or
* a macro.
*
* 008 	15-Oct-82	DCL		Steal RGTEXT.REQ, the include file
*			containing ReGIS parser text functionality, to
*			REGIS2.BLI, the overlay module contining ReGIS
*			parser text functionality.
* 007	8-Jan-1982	J.Lomicka	Pass .char to skip_quote
* 006	28-Dec-1981	J.Lomicka	Properly update tx_direction
* 005	22-Dec-1981	J.Lomicka	Correct standard size and H option code
* 004	21-Dec-1981	J.Lomicka	Remove OWN declarations to REGBLK.REQ,
*			where re-enterancy is obtainable.
* 003	30-Nov-81 /DCL	Add ALWAYS_RELATIVE = 1 to all calls to COORDINATES
* 002	26-Aug-1981	Send gidis codes for options
* 001	10-Aug-1981	Properly parse options (none implemented)
* 000	4-Aug-1981	Compilable stub for text command
**/

#include "gidcalls.h"
#include "regstruct.h"

#define TX_STATE_SIZE     11
	/* TX_STATE_SIZE is the number of words required to store the text   */
	/*   state variables which are defined in file REGLOB.C		     */

/*****			E X T E R N A L   F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      MODULE */
/*	--------		-----------			      ------ */
extern
    convert_floating_to_integer(),/*				       REGIS */
    x1_floating_bracketed_pair(),/* Unscaled coordinates parser	       REGIS */
    get_gireport(),		/*				       REPORT*/
    rgid_process(),		/* Send gid commands to be processed   GIDISx*/
    ignore_ch(),		/* Ignore erroneous character	       REGIS */
    inch_q_unq(), 		/* Fetch character, (un)quoted modes   SCAN  */
    number(),			/* Number parser entry point	       REGIS */
    p_inch(),			/* Push character back into scanner.	     */
#if VARIANT > 0
    PutBreak(),			/* Flush all pending terminal output   REGIO */
    PutChar(),			/* Queue character for terminal output REGIO */
    PutDecimal(),		/* Output signed decimal number	       OPSYS */
    Put_String(),		/* Output an ASCIZ string	       OPSYS */
#endif
    rel_coordinates(),		/* Process [ (scaled, normalized)      REGIS3*/
#if SEPARATE_CURRENT_POSITION || ASK_FOR_TEXT_ESCAPEMENT
    rpg_request_gidi_pos(),		/* Request and read current	       REPORT*/
#endif				/*	GIDIS position			     */
    scan(),			/* Character fetch		       REGIS */
    skp_coord(),		/* [] skip state		       REGIS */
    skp_option(),		/* Option letter skip state	       REGIS */
    skp_paren(),		/* Parenthesis skip state	       REGIS */
    skp_quote(),		/* Quotation skip state		       REGIS */
    temp_write_options();	/* Temporary writing options	       REGIS */

/*****		F U N C T I O N S   I N   T H I S   F I L E		 *****/
/*									     */
/*		FUNCTION			DESCRIPTION		     */
/*		--------			-----------		     */
/**
*		a_option()			Process text option A
*		d_option()			Process text option D
*		h_option()			Process text option H
*		i_option()			Process text option I
*		m_option()			Process text option M
*		p2_pixel_offset(v,v,a,a)
*		r1_restore_test_state()
*		save_text_state()
*		send_quoted_string(v)		Process text quoted strings
*		standard_size(v)		Process T(Sn
*		s_option()			Process text S option
*		text_instruction()		Read and Parse T command
*		text_option()			Process text options
*		u_option()			Process text option U
*		update_cell_standard()
**/

a_option()
{
    /**
    *	Called when option character A is encountered
    *	to select alphabet.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: a_option, regis2.c\n", PutChar);
#endif
**/
    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* resync and alphabetic */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, a_option, regis2.c\n", PutChar);
#endif
**/
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quoted string */
		break;
	    case  0 : 		/* Other, possibly numeric arg */
	    {
		int	alphabet;
		int	rel_flag;	/* Unused relativeness flag */

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&alphabet, &rel_flag, 0) ))
		{
		    if (alphabet < TOTAL3_NUMBER_OF_ALPHABETS && alphabet >= 0)
		    {			/*Yes - in valid alphabet range? */
			rs->tx_alphabet = alphabet;	/* make it the current alphabet */
			if (alphabet != rs->gid_alphabet)
			{
			    G66_SET_ALPHABET(alphabet);
				/*...and tell GIDIS about it */
			    rs->gid_alphabet = alphabet;
			}
		    }
		    else
		    {
			ERROR(ALPHABET_OUT_OF_RANGE, 0);
		    }
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 :
		if ( rs->kanji_regis )
		alr_option();   /* character set designation option */
		else
		skp_paren();	/* Paren, not used here */
		break;
	    case  2 :
		skp_coord();	/* '[', not used here */
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, a_option, regis2.c\n", PutChar);
#endif
**/
}

/* character set designation */

alr_option()
{
    register struct regis_cntx *rs;	/* pointer to context block */
    int g_set;

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {
	switch (scan( "LR)" ))
	{
     	    case -3 :
	    case -1 :
	    case  0 :
	    case  3 :		/* resync and alphabetic */
		rs->first_process_me = TRUE;
		return;
	    case  1 :
		g_set = 0;
		break;
	    case  2 :
		g_set = 1;                       
		break;
	    case -2 :
		desig_char_set(g_set, rs->character);	
		break;
	}
    }
}

desig_char_set(g_set, quote_character)
int g_set;
int quote_character;
{
    register struct regis_cntx *rs;	/* pointer to context block */
    char buff[4], *charp;
    int char_set, gc_mask, ch;

    rs = RSTRUCT;			/* point to it */

    for (charp = buff; charp < buff + 4; charp++) *charp = 0;
    charp = buff;

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

/* convert string to character set */

		switch (buff[0])
		{
		    case '(' :
		    case ')' :
		    case '*' :
		    case '+' :
			break;
		    case '$' :
			if (buff[1] != '+') return;
			break;
		    default :
			return;
		}
		if (buff[0] != '$')
		    switch(buff[1])
		    {
		        case 'B' :
			    char_set = REG_ASCII;
			    break;
		        case '0' :
			    char_set = REG_LINE_DRAWING;
			    break;
		        case '<' :
		    	    char_set = REG_SUPPLEMENTAL;
			    break;
		        case 'J' :
			    char_set = REG_JIS_ROMAN;
			    break;
		        case 'I' :
			    char_set = REG_JIS_KATAKANA;
			    break;
			default :
			    return;
		    }
		else
		    switch(buff[2])
		    {
/* while the third character should be checked, VT286 doesn't */

		        case '1' :
		        case '3' :
			default :
			    char_set = REG_DEC_KANJI;
			    break;
		    }         
		gc_mask = 0;
		switch (char_set)
		    {
		        case REG_ASCII:
			    break;
		        case REG_LINE_DRAWING:
			    break;
		        case REG_SUPPLEMENTAL:
			    break;
			case REG_JIS_ROMAN:
			    gc_mask |= ROMAN_TEXT_GC_MASK;
			    break;
			case REG_JIS_KATAKANA:
			    gc_mask |= ROMAN_TEXT_GC_MASK;
			    break;
       			case REG_DEC_KANJI:
			    gc_mask |= KANJI_TEXT_GC_MASK;
			    break;           
		    }
	        if (!(rs->widget->output.font_list[gc_mask >> 1]))
	    	    {
		    open_font(rs->widget, gc_mask);
		    }
		rs->g_char_set[g_set] = char_set;
		rs->g_text_font[g_set] = rs->widget->output.font_list[gc_mask >> 1];
		for ( ch = 128 * g_set; ch < 128 * (g_set + 1); ch++ )
		    {
		    if ( rs->font_glyphs[0][ch] != NULL )
	    		{
	    		XtFree( rs->font_glyphs[0][ch] );
	    		rs->font_glyphs[0][ch] = NULL;
	    		}
		    }
		return;
	    }
	}
	else
	{
	    *charp = rs->character;
	    if (charp < buff + 3) charp++;
	}                                       
    }
}

d_option()
{
    /**
    *	Called for option character D, character row direction
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: d_option, regis2.c\n", PutChar);
#endif
**/
    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* resync and alphabetic */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, d_option, regis2.c\n", PutChar);
#endif
**/
		return;
	    case  -2 :
		skp_quote(rs->character);	/* Quoted string */
		break;
	    case  0 : 		/* Other, possibly numeric arg */
	    {
		int	rel_flag;
		int	rotation_angle;

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&rotation_angle, &rel_flag, 0) ))
		{
		    rs->tx_direction = (rotation_angle % 360); /* % = MOD */
		    if (rs->tx_direction < 0)
		    {
			rs->tx_direction += 360;
		    }
		    G90_SET_CELL_ROTATION(rs->tx_direction);
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 :
		skp_paren();	/* Paren, not used here */
		break;
	    case  2 :
		skp_coord();
	}
    }
}

h_option()
{
    /**
    *	Called for option character H, character height
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* resync and alphabetic */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quoted string */
		break;
	    case  0 : 		/* Other, possibly numeric arg */
	    {
		int	rel_flag;
		int	hght_multiplier;

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&hght_multiplier, &rel_flag, 0) ))
		{
		    hght_multiplier = ((hght_multiplier < 0) ?
				(-hght_multiplier) : hght_multiplier);
		    if (hght_multiplier > 25)
			hght_multiplier = 25;
		    if (hght_multiplier == 0)
			hght_multiplier = 2;
		    /**
		    * Use standard height component only	     2
		    * Note that we divide by 2 - this is
		    * a vestige of VT125 odd-Y simulation,
		    * but the H option is only provided
		    * for VT125 compatibility anyway.
		    **/
		    rs->tx_y_unit_size = (rs->st_y_unit_size * hght_multiplier) >> 1;
		    G76_SET_CELL_UNIT_SIZE(rs->tx_x_unit_size, rs->tx_y_unit_size);
		    rs->tx_y_display_size = rs->tx_y_unit_size;
		    G75_SET_CELL_DISPLAY_SIZE(rs->tx_x_display_size,rs->tx_y_display_size);
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 :
		skp_paren();	/* Paren, not used here */
		break;
	    case  2 :
		skp_coord();
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, h_option, regis2.c\n", PutChar);
#endif
**/
}

i_option()
{
    /**
    *	Called for option character I, set italic angle
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: i_option, regis2.c\n", PutChar);
#endif
**/
    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* resync and alphabetic */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, i_option, regis2.c\n", PutChar);
#endif
**/
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quoted string */
		break;
	    case  0 : 		/* Other, possibly numeric arg */
	    {
		int	rel_flag;	/* Unused relative indicatior */

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&rs->tx_oblique, &rel_flag, 0) ))
		{
		    G91_SET_CELL_OBLIQUE(rs->tx_oblique);
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 :
		skp_paren();	/* Paren, not used here */
		break;
	    case  2 :
		skp_coord();
	}
    }

}

m_option()
{
    /**
    *	Called when option character M is encountered
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    while ( TRUE )
    {
	switch (scan( "[()" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* Re-sync character */
		rs->first_process_me = TRUE;
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quote character */
		break;
	    case  0 :
		ignore_ch();	/* Nothing interesting */
		break;
	    case  1 : 		/* Get multipliers */
	    {
		int	routine_value;
		int	relflags;
		int	x_exponent;
		int	x_mantissa;
		int	y_exponent;
		int	y_mantissa;
/**
* Note that the call to x1_floating_bracketed_pair uses 1 implied decimal
* place each for the height and width multipliers, thus allowing support of
* fractional multipliers.  We divide by 10 below (by using exponent - 1 as
* argument to convert_floating_to_integer) to compensate.
**/
if (IS_TRUE(x1_floating_bracketed_pair(&x_mantissa,&x_exponent,&y_mantissa,&y_exponent, &relflags, 1, 1)))
		{
		    /**
		    *Note that we divide height by 2 -
		    * this is a vestige of VT125 odd-Y
		    * simulation, but the M option is
		    * only provided for VT125
		    * compatibility anyway.
		    **/
		    rs->tx_x_unit_size = convert_floating_to_integer(
			x_mantissa, (--x_exponent));
		    if (rs->tx_x_unit_size  > 16)
		    {
			rs->tx_x_unit_size = 16;
		    }
		    rs->tx_x_unit_size *= rs->st_x_unit_size;
	
		    if (rs->tx_x_unit_size < 0)
		    {
			rs->tx_x_unit_size *= (-1);
		    }
		    rs->tx_y_unit_size = convert_floating_to_integer(
			(y_mantissa * (rs->st_y_unit_size >> 1)), (--y_exponent));
		    if (rs->tx_y_unit_size < 0)
		    {
			rs->tx_y_unit_size *= (-1);
		    }
		    G76_SET_CELL_UNIT_SIZE(rs->tx_x_unit_size, rs->tx_y_unit_size);
		}
		break;
	    }
	    case  2 :
		skp_paren();	/* Unexpected paren */
	}
    }

}

p2_pixel_offset(pv, div_2, p_x_offset, p_y_offset)
int	pv;
int	div_2;
int	*p_x_offset;
int	*p_y_offset;
{

    int		tx_pv;
    int		pvy;
    int		dx_space;
    int		dy_space;
    int		lf_dx;
    int		lf_dy;
    int		dx;
    int		dy;

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* first calculate the offsets for X and Y directions */
    /* for the current text.  Base these offsets on the */
    /* current display size. */


    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: p2_pixel_offset, regis2.c\n", PutChar);
#endif
**/
    tx_pv = (rs->tx_direction + 360 + (45/2)) / 45;

    switch (tx_pv & 3)
    {
	case  0 :			/* 0 or 180 degrees */
	    dx_space = rs->tx_x_display_size;
	    dy_space = 0;
	    lf_dx = 0;
	    lf_dy = rs->tx_y_display_size;
	    break;
	case  1 :			/* 45 or 225 degrees */
	    dx_space = rs->tx_x_display_size;
	    dy_space = (-(ADJUST_45_TEXT_Y(rs->tx_x_display_size)));
	    lf_dx = ADJUST_45_TEXT_Y(rs->tx_y_display_size);
	    lf_dy = rs->tx_y_display_size;
	    break;
	case  2 :			/* 90 or 270 degrees */
	    dx_space = 0;
	    dy_space = (-rs->tx_x_display_size);
	    lf_dx = rs->tx_y_display_size;
	    lf_dy = 0;
	    break;
	case  3 :			/* 135 or 315 degrees */
	    dx_space = (-rs->tx_x_display_size);
	    dy_space = (-(ADJUST_45_TEXT_Y(rs->tx_x_display_size)));
	    lf_dx = ADJUST_45_TEXT_Y(rs->tx_y_display_size);
	    lf_dy = (-rs->tx_y_display_size);
	    break;    
    }
    if ((tx_pv & 4) != 0)
    {
	dx_space = (-dx_space);
	dy_space = (-dy_space);
	lf_dx = (-lf_dx);
	lf_dy = (-lf_dy);
    }
    /* now calculate the distances to go in the desired */
    /* direction */

    switch (pv & 3)
    {
	case  0 :			/* +0 or +180 from text direction */
	    dx = dx_space;
	    dy = dy_space;
	    break;
	case  1 :			/* +45 or +225 from text direction */
	    dx = dx_space - lf_dx;
	    dy = dy_space - lf_dy;
	    break;
	case  2 :			/* +90 or +270 from text direction */
	    dx = (-lf_dx);
	    dy = (-lf_dy);
	    break;
	case  3 :			/* +135 or +315 from text direction */
	    dx = (-(dx_space + lf_dx));
	    dy = (-(dy_space + lf_dy));
    }
    if ((pv & 4) != 0) 	/* second numbers above, flip dx and dy */
    {
	dx = (-dx);
	dy = (-dy);
    }
    *p_x_offset = ( (IS_FALSE(div_2)) ? dx : dx >> 1);
    *p_y_offset = ( (IS_FALSE(div_2)) ? dy : dy >> 1);

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, p2_pixel_offset, regis2.c\n", PutChar);
#endif
**/
}

r1_restore_test_state()
{
    /**
    *	EXPLICIT INPUTS:	    none			
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:	    saved text state
    *
    *	IMPLICIT OUTPUTS:	    current text state
    *
    *	SIDE EFFECTS:		    none
    **/

    int		i;

    int		*tx_cur_state;		/* current state */
    int		*tx_svd_state;		/* saved state */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /**
    * Caution:  tx_cur_state and tx_svd_state are the beginning addresses
    *  of the tx current state blocks and saved state blocks which are
    *  defined in file reglob.c but not as blocks but rather consequitive
    *  defined variables.  This is dangerous code and should eventually be
    *  corrected.
    **/

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: r1_restore_test_state, regis2.c\n", PutChar);
#endif
**/
    tx_cur_state = &rs->tx_alphabet;
    tx_svd_state = &rs->s_txalphabet;

    /* if there are any differences between the old and new Regis */
    /* states, make sure that Gidis knows about them also. */

    if (rs->tx_alphabet != rs->s_txalphabet)
    {
	G66_SET_ALPHABET(rs->s_txalphabet);
	rs->gid_alphabet = rs->s_txalphabet;
    }
    if (rs->tx_direction != rs->s_txdirection)
    {
	G90_SET_CELL_ROTATION(rs->s_txdirection);
    }
    if ((rs->tx_x_display_size != rs->s_6txx_display_size) ||
	(rs->tx_y_display_size != rs->s_5txy_display_size))
    {
	G75_SET_CELL_DISPLAY_SIZE(rs->s_6txx_display_size, rs->s_5txy_display_size);
    }
    if ((rs->tx_x_unit_size != rs->s_txx_unit_size) ||
	(rs->tx_y_unit_size != rs->s_txy_unit_size))
    {
	G76_SET_CELL_UNIT_SIZE(rs->s_txx_unit_size, rs->s_txy_unit_size);
    }
    if ((rs->tx_x_escapement != rs->s_2txx_escapement) ||
	(rs->tx_y_escapement != rs->s_1txy_escapement))
    {
	G83_SET_CELL_EXPLICIT_MOVEMENT(rs->s_2txx_escapement, rs->s_1txy_escapement);
    }
    if (rs->tx_oblique != rs->s_txoblique)
    {
	G91_SET_CELL_OBLIQUE(rs->s_txoblique);
    }
    /* now update the Regis state table */

    for (i = 0; i < TX_STATE_SIZE; i++)
    {
	tx_cur_state[i] = tx_svd_state[i];
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, r1_restore_test_state, regis2.c\n", PutChar);
#endif
**/
}

s_option()
{
    /**
    *	Called for option character S, character set size
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: s_option, regis2.c\n", PutChar);
#endif
**/
    while ( TRUE )
    {
	switch (scan( "([)" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* resync and alphabetic */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, s_option, regis2.c\n", PutChar);
#endif
**/
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quoted string */
		break;
	    case  0 : 		/* Other, possibly numeric arg */
	    {
		int	rel_flag;
		int	alph_size;

		rs->first_process_me = TRUE;

		if (IS_TRUE( number(&alph_size, &rel_flag, 0) ))
		{			
		    standard_size(alph_size);
		}
		else
		{
		    ignore_ch();
		}
		break;
	    }
	    case  1 :
		skp_paren();	/* Paren, not used here */
		break;
	    case  2 : 		/* set display size to coordinates */
		if (IS_TRUE(rel_coordinates() ))
		{
		    rs->tx_x_display_size = ((rs->rel_x_coord < 0) ?
				(-rs->rel_x_coord) : rs->rel_x_coord);
		    rs->tx_y_display_size = ((rs->rel_y_coord < 0) ?
				(-rs->rel_y_coord) : rs->rel_y_coord);

		    if ((rs->tx_x_display_size == 0) || (rs->tx_y_display_size == 0))
		    {

			if (rs->tx_x_display_size == 0)
			{
			    rs->tx_x_display_size = rs->st_x_display_size;	   /* 2 */
			}
			if (rs->tx_y_display_size == 0)
			{
			    rs->tx_y_display_size = rs->st_y_display_size;	   /* 2 */
			}
		    }
		    G75_SET_CELL_DISPLAY_SIZE(rs->tx_x_display_size,rs->tx_y_display_size);
		}
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, s_option, regis2.c\n", PutChar);
#endif
**/
}

save_text_state()
{
    /**
    *	EXPLICIT INPUTS:	    none
    *
    *	EXPLICIT OUTPUTS:	    none
    *
    *	IMPLICIT INPUTS:	    current_text_modes
    *
    *	IMPLICIT OUTPUTS:	    saved_text_modes
    *
    *	SIDE EFFECTS:		    none
    **/

    int		i;

    int		*tx_cur_state;		/* current state */
    int		*tx_svd_state;		/* saved state */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /**
    * Caution:  tx_cur_state and tx_svd_state are the beginning addresses
    *  of the tx current state blocks and saved state blocks which are
    *  defined in file reglob.c but not as blocks but rather consequitive
    *  defined variables.  This is dangerous code and should eventually be
    *  corrected.
    **/

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: save_text_state, regis2.c\n", PutChar);
#endif
**/
    tx_cur_state = &rs->tx_alphabet;
    tx_svd_state = &rs->s_txalphabet;

    for (i = 0; i < TX_STATE_SIZE; i++)
    {
	tx_svd_state[i] = tx_cur_state[i];
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, save_text_state, regis2.c\n", PutChar);
#endif
**/
}

send_quoted_string( quote_character )
int	quote_character;
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *   Read a quoted string from the input stream, sending characters
    *   to GIDIS to be displayed in the current text mode on the device.
    *
    * FORMAL PARAMETERS:
    *	INPUT: quote_character: ascii value of begin quote character
    *
    * IMPLICIT INPUTS:		characters form scanners INCH_Q routine
    *
    * IMPLICIT OUTPUTS:	instructions send to GIDIS process
    **/

    register struct regis_cntx *rs;	/* pointer to context block */
    int char_saved = False;

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: send_quoted_string, regis2.c\n", PutChar);
#endif
**/

#if SEPARATE_CURRENT_POSITION
    if (IS_FALSE( rs->gid_xy_valid ))
    {
	rpg_request_gidi_pos();
    }
#endif
    rs->tx_x_begin = rs->gid_x;
    rs->tx_y_begin = rs->gid_y;

    while ( TRUE )
    {
	rs->character = INCH_Q(0);	/* Get the next character */

	if (!(rs->kanji_regis && char_saved) && rs->character == quote_character)
	{

	    if ((rs->character = INCH_Q(0) ) == quote_character)
	    { 		/* Proceed as if it were a normal character */
	    }
	    else
	    {
		p_inch(rs->character);	/* push character back into scanner */
		if (rs->sc_current_opcode != 0)
		{
		    G2_END_LIST(); 	/* and end the string */
		    rs->sc_current_opcode = 0;
		}

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, send_quoted_string, regis2.c\n", PutChar);
#endif
**/
		return;
	    }
	}

	switch (rs->character)
	{
	    case  8 :			/*	Backspace */
		if (rs->sc_current_opcode != 0)
		{
		    G2_END_LIST();
		    rs->sc_current_opcode = 0;
		}
#if SEPARATE_CURRENT_POSITION
		if (IS_FALSE(rs->gid_xy_valid))
		{
		    rpg_request_gidi_pos();
		}
#endif
		rs->gid_x -= rs->tx_x_escapement;
		rs->gid_y -= rs->tx_y_escapement;
		G56_SET_POSITION(rs->gid_x, rs->gid_y);
		break;
	    case  9 :			/*	Horizontal Tab */
		if (rs->sc_current_opcode != 0)
		{
		    G2_END_LIST();
		    rs->sc_current_opcode = 0;
		}
#if SEPARATE_CURRENT_POSITION
		if (IS_FALSE(rs->gid_xy_valid))
		{
		    rpg_request_gidi_pos();
		}
#endif
		rs->gid_x += rs->tx_x_escapement;
		rs->gid_y += rs->tx_y_escapement;
		G56_SET_POSITION(rs->gid_x, rs->gid_y);
		break;
	    case 13 : 			/*	Carriage return movement */
		if (rs->sc_current_opcode != 0)
		{
		    G2_END_LIST();
		    rs->sc_current_opcode = 0;
		}
		G56_SET_POSITION(rs->tx_x_begin, rs->tx_y_begin);
		rs->gid_x = rs->tx_x_begin;
		rs->gid_y = rs->tx_y_begin;
		break;
	    case 10 :		/*	Line feed movement */
	    case 11 :		/* for LF, VT and FF */
	    case 12 :
	    {
		int	lf_x;
		int	lf_y;

		if (rs->sc_current_opcode != 0)
		{
		    G2_END_LIST();
		    rs->sc_current_opcode = 0;
		}
		/* ask service routine how large a '6' pixel vector offset is. */

		p2_pixel_offset(6, FALSE, &lf_x, &lf_y);
		rs->tx_x_begin += lf_x;
		rs->tx_y_begin += lf_y;
#if SEPARATE_CURRENT_POSITION
		if (IS_FALSE(rs->gid_xy_valid))
		{
		    request_current_position( &rs->gid_x, &rs->gid_y );
		    rs->gid_xy_valid = TRUE;
		}
#endif
		    rs->gid_x += lf_x;
		    rs->gid_y += lf_y;
		    G56_SET_POSITION(rs->gid_x, rs->gid_y);
		break;
	    }
	    default :
		if (rs->kanji_regis)
		    if (char_saved)
			char_saved = False;
		    else if ((rs->character < 128 && 
			    rs->g_char_set[0] == REG_DEC_KANJI) ||
			    (rs->character > 127 &&
			    rs->g_char_set[1] == REG_DEC_KANJI))
			char_saved = True;
		if (rs->sc_current_opcode != G94_OP_DRAW_CHARACTERS)
		{
		    if (rs->sc_current_opcode != 0)
		    {
			G2_END_LIST();
		    }
		    G95_DRAW_CHARACTERS_END_LIST();
		    rs->sc_current_opcode = G94_OP_DRAW_CHARACTERS;
		}
#if VARIANT == 1
		Put_String( (" [send char "), PutChar );
		PutDecimal( rs->character, PutChar );
		Put_String( ("] "), PutChar );
		PutBreak();
#endif
		if ( rs->tx_alphabet != 0 && rs->character > 128 )
		    break;	/* no GR for lodaed alphabets */

		rs->temp_gid_buffer[0] = 
			((rs->tx_alphabet == 0) ? rs->character : CH_FOLD(rs->character) );
		rgid_process( rs->temp_gid_buffer, 1 );
		rs->gid_x += rs->tx_x_escapement;
		rs->gid_y += rs->tx_y_escapement;

#if SEPARATE_CURRENT_POSITION
		rs->gid_xy_valid = FALSE;
#endif
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, send_quoted_string, regis2.c\n", PutChar);
#endif
**/
}

standard_size( alph_size )
int	alph_size;
{
    /**
    * Process T(Sn
    **/

    int		pv;

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /**
    * Do different things based on the alphabet standard size requested
    *
    * Size zero = normal width, half height
    * Size one = normal width and height
    * Size n > 1 = n * width, 3/4 * n * height
    **/

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: standard_size, regis2.c\n", PutChar);
#endif
**/

    switch (alph_size)
    {
	case  0 :
	    rs->tx_x_unit_size = rs->st_x_unit_size;				   /* 2 */
	    rs->tx_y_unit_size = (rs->st_y_unit_size + 1) >> 1;			   /* 2 */
	    rs->tx_x_display_size = rs->st_x_display_size;			   /* 2 */
	    rs->tx_y_display_size = (rs->st_y_display_size + 1) >> 1;		   /* 2 */
	    break;
	case  1 :
	    rs->tx_x_unit_size = rs->st_x_unit_size;				   /* 2 */
	    rs->tx_y_unit_size = rs->st_y_unit_size;				   /* 2 */
	    rs->tx_x_display_size = rs->st_x_display_size;			   /* 2 */
	    rs->tx_y_display_size = rs->st_y_display_size;			   /* 2 */
	    break;
	case  2 :
	case  3 :
	case  4 :
	case  5 :
	case  6 :
	case  7 :
	case  8 :
	case  9 :
	case 10 :
	case 11 :
	case 12 :
	case 13 :
	case 14 :
	case 15 :
	case 16 :
	    rs->tx_x_unit_size = rs->st_x_unit_size * alph_size;		   /* 2 */
	    rs->tx_y_unit_size = (rs->st_y_unit_size * alph_size * 3 + 1) >> 2;/* 2 */
	    rs->tx_x_display_size = rs->st_x_display_size * alph_size;	   /* 2 */
	    rs->tx_y_display_size = ((rs->st_y_display_size * alph_size * 3)+1)>>2;
	default :
	    ERROR(TEXT_STANDARD_SIZE_ERROR, 0);
    }
#if VARIANT == 2
    Put_String( (" [std size "), PutChar );
    PutDecimal( alph_size, PutChar );
    Put_String( (" = U["), PutChar );
    PutDecimal( rs->tx_x_unit_size, PutChar );
    Put_String( (", "), PutChar );
    PutDecimal( rs->tx_y_unit_size, PutChar );
    Put_String( ("] S["), PutChar );
    PutDecimal( rs->tx_x_unit_size, PutChar );
    Put_String( (", "), PutChar );
    PutDecimal( rs->tx_x_unit_size, PutChar );
    Put_String( ("] ] "), PutChar );
    PutBreak();
#endif
    G76_SET_CELL_UNIT_SIZE(rs->tx_x_unit_size, rs->tx_y_unit_size);
    G75_SET_CELL_DISPLAY_SIZE(rs->tx_x_display_size, rs->tx_y_display_size);

    /* now calculate tx_x/y_escapement, which is the  */
    /* distance a space should move. */

    /**
    * There are two algorithms for determining escapement:
    * The first actually draws a space (in transparent mode)
    * and loads tx_x/y_escapement from the gidis current 
    * position.  The second makes an educated guess from the
    * display size.  The second one also adjusts for the 2.5 
    * to 1 pixel aspect ratio of the PRO by using the macro
    * ADJUST_45_TEXT_Y defined in REGDEVDEP or thereabouts.
    **/
#if ASK_FOR_TEXT_ESCAPEMENT
  {
    int		temp_x,;
    int		temp_y;

#if SEPARATE_CURRENT_POSITION
    if (IS_FALSE(rs->gid_xy_valid))
    {
	rpg_request_gidi_pos();  /* where are we now */
    }
#endif
    temp_x = rs->gid_x;
    temp_y = rs->gid_y;

    G77_SET_CELL_MOVEMENT_MODE(G80_IMPLICIT_LOCAL);  /* set special mode */
    G83_SET_CELL_EXPLICIT_MOVEMENT(0, 0);
    G90_SET_CELL_ROTATION(rs->tx_direction);
    G37_SET_WRITING_MODE(G38_MODE_TRANSPARENT);
   {
    int		array_of_parameters[1];
    array_of_parameters[0] = 0;
    G94_DRAW_CHARACTERS(1, array_of_parameters);		/* draw 1 character */
   }
    rpg_request_gidi_pos();		/* find out distance moved */
    rs->tx_x_escapement = rs->gid_x - temp_x;
    rs->tx_y_escapement = rs->gid_y - temp_y;

    G56_SET_POSITION(temp_x, temp_y);	/* go back to original position */
    rs->gid_x = temp_x;
    rs->gid_y = temp_y;
#if SEPARATE_CURRENT_POSITION
    rs->gid_xy_valid = TRUE;
#endif
    G37_SET_WRITING_MODE(rs->md_mode | rs->md_negative_writing); 
    G77_SET_CELL_MOVEMENT_MODE(G78_EXPLICIT_LOCAL) /* and movement mode */
  }    
#else
    /* calculate escapement myself */

    pv = (rs->tx_direction + 360 + (45/2)) / 45;

    switch (pv & 3)
    {
	case  0 :			/* 0 or 180 degrees */
	    rs->tx_x_escapement = rs->tx_x_display_size;
	    rs->tx_y_escapement = 0;
	    break;
	case  1 :			/* 45 or 225 degrees */
	    rs->tx_x_escapement = rs->tx_x_display_size;
	    rs->tx_y_escapement = (-(ADJUST_45_TEXT_Y(rs->tx_x_display_size)));
	    break;
	case  2 :			/* 90 or 270 degrees */
	    rs->tx_x_escapement = 0;
	    rs->tx_y_escapement = (-rs->tx_x_display_size);
	    break;
	case  3 :			/* 135 or 315 degrees */
	    rs->tx_x_escapement = (-rs->tx_x_display_size);
	    rs->tx_y_escapement = (-(ADJUST_45_TEXT_Y(rs->tx_x_display_size)));
    }
    if ((pv & 4) != 0) 	/* second numbers above */
    {
	rs->tx_x_escapement = (-rs->tx_x_escapement);
	rs->tx_y_escapement = (-rs->tx_y_escapement);
    }
#endif

    G83_SET_CELL_EXPLICIT_MOVEMENT(rs->tx_x_escapement, rs->tx_y_escapement);

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, standard_size, regis2.c\n", PutChar);
#endif
**/
}

text_instruction()
{
    /**
    * FUNCTIONAL DESCRIPTION: Read and parse the regis T command, producing
    *   the appropriate corresponding GiDIS instructions.
    *
    * FORMAL PARAMETERS:	None
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:		None
    *
    * SIDE EFFECTS:
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: test_instruction, regis2.c\n", PutChar);
#endif
**/

    if (rs->tx_alphabet != rs->gid_alphabet)
    {
	G66_SET_ALPHABET(rs->tx_alphabet);
	rs->gid_alphabet = rs->tx_alphabet;
    }
    while ( TRUE )
    {
	int	scan_result;

	switch (scan_result = scan( "(01234567[" ))
	{
	    case -3 :
	    case -1 :
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, text_instruction, regis2.c\n", PutChar);
#endif
**/
		return;
	    case -2 :
		send_quoted_string(rs->character);
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :
		text_option();		/* Open paren */
		break;
	    case  2 :
	    case  3 :
	    case  4 :
	    case  5 :
	    case  6 :
	    case  7 :
	    case  8 :
	    case  9 :			/* note: scan_result - 2 used  */
					/* below as the actual pixel vector */
					/* don't change one without the other */
	    {
		int	x_offset;
		int	y_offset;

		/* find out how far to go by asking service routine */

		p2_pixel_offset((scan_result - 2), TRUE, &x_offset, &y_offset);
#if SEPARATE_CURRENT_POSITION
		if (IS_FALSE(rs->gid_xy_valid))
		{
		    request_current_position( &rs->gid_x, &rs->gid_y );
		}
#endif
		rs->gid_x += x_offset;
		rs->gid_y += y_offset;
		G56_SET_POSITION(rs->gid_x, rs->gid_y);
		break;
	    }
	    case 10 : 			/* "[" (Open square brace) */
		if (IS_TRUE(rel_coordinates() )) 
		{
		    G83_SET_CELL_EXPLICIT_MOVEMENT(rs->rel_x_coord, rs->rel_y_coord);
		    rs->tx_x_escapement = rs->rel_x_coord;
		    rs->tx_y_escapement = rs->rel_y_coord;
		}
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, text_instruction, regis2.c\n", PutChar);
#endif
**/
}

text_option()
{
    /**
    *	    Called when text options are expected
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: text_option, regis2.c\n", PutChar);
#endif
**/

    while ( TRUE )
    {
	switch (scan( ")MAIDS(HUWBE" ))
	{
	    case -3 : 			/* Re-sync character */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, text_option, regis2.c\n", PutChar);
#endif
**/
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quote character */
		break;
	    case -1 :
		skp_option();	/* Undefined option */
		break;
	    case  0 :
		ignore_ch();		/* Nothing interesting */
		break;
	    case  1 :

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, text_option, regis2.c\n", PutChar);
#endif
**/
		return;		/* Close paren */
	    case  2 :
		m_option();	/* set pixel multipliers */
		break;
	    case  3 :
		a_option();	/* Select a character set */
		break;
	    case  4 :
		i_option();	/* Set italic angle */
		break;
	    case  5 :
		d_option();	/* Set row direction */
		break;
	    case  6 :
		s_option();	/* Set size of displayed region */
		break;
	    case  7 :
		skp_paren();	/* Skip over other parens */
		break;
	    case  8 :
		h_option();	/* Set height multiplier */
		break;
	    case  9 :
		u_option();	/* Set explicit unit size of character */
		break;
	    case 10 :
		temp_write_options();
		break;
	    case 11 :			/* begin block of text state */
		save_text_state();
		break;
	    case 12 :			/* End block of text state */
		r1_restore_test_state();
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #3, text_option, regis2.c\n", PutChar);
#endif
**/
}

u_option()
{
    /**
    *	Called when option character U is encountered
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: u_option, regis2.c\n", PutChar);
#endif
**/

    while ( TRUE )
    {
	switch (scan( "[()" ))
	{
	    case -3 :
	    case -1 :
	    case  3 :		/* Re-sync character */
		rs->first_process_me = TRUE;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, u_option, regis2.c\n", PutChar);
#endif
**/
		return;
	    case -2 :
		skp_quote(rs->character);	/* Quote character */
		break;
	    case  0 :
		ignore_ch();	/* Nothing interesting */
		break;
	    case  1 : 		/* get sizes			     2 */
		if (IS_TRUE(rel_coordinates()))
		{
		    /**
		    * Take abs value to avoid mirroring.  N. B. To support
		    * cell mirroring, somehow obtain the actual numbers
		    * entered (pre-transformation) and compute sgn( input ) *
		    * abs( transform( input )).
		    **/
		    rs->tx_x_unit_size = ((rs->rel_x_coord < 0) ?
					(-rs->rel_x_coord) : rs->rel_x_coord);
		    rs->tx_y_unit_size = ((rs->rel_y_coord < 0) ?
					(-rs->rel_y_coord) : rs->rel_y_coord);
		    if (rs->tx_x_unit_size == 0)
		    {
			rs->tx_x_unit_size = rs->st_x_unit_size;	     /* 2 */
		    }
		    if (rs->tx_y_unit_size == 0)
		    {
			rs->tx_y_unit_size = rs->st_y_unit_size;	     /* 2 */
		    }
		    G76_SET_CELL_UNIT_SIZE(rs->tx_x_unit_size, rs->tx_y_unit_size);
		}
		break;
	    case  2 :
		skp_paren();	/* Unexpected paren */
	}
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, u_option, regis2.c\n", PutChar);
#endif
**/
}

ucs_update_cell_standard()
{

    int		error_count;

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    report_cell_standard( &rs->st_x_unit_size, &rs->st_y_unit_size,
			  &rs->st_x_display_size, &rs->st_y_display_size );

    rs->tx_x_unit_size = rs->st_x_unit_size;
    rs->tx_y_unit_size = rs->st_y_unit_size;
    rs->tx_x_display_size = rs->st_x_display_size;
    rs->tx_y_display_size = rs->st_y_display_size;
    rs->tx_x_escapement = rs->st_x_display_size;
    rs->tx_y_escapement = 0;
    
    if (rs->tx_direction != 0)
    {
	G90_SET_CELL_ROTATION(rs->tx_direction);	     /* 2 */
    }
    if (rs->gid_alphabet != 0)
    {
	G66_SET_ALPHABET(rs->gid_alphabet);
    }

    G76_SET_CELL_UNIT_SIZE(rs->tx_x_unit_size, rs->tx_y_unit_size);
    G75_SET_CELL_DISPLAY_SIZE(rs->tx_x_display_size, rs->tx_y_display_size);
    G83_SET_CELL_EXPLICIT_MOVEMENT(rs->tx_x_escapement, rs->tx_y_escapement);
}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*2 A_VESPER 21-OCT-1983 16:32:01 "added std size globals -
*		- loaded by S(A) routine"
*	*1 A_VESPER 14-SEP-1983 14:01:37 ""
**/
