/*****	scanner ( IDENT = 'V1.17' )  *****/				  /*3*/
/* #module <module name> "X0.0" */
/*
 *  Title:	scan.c
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
* ABSTRACT:	Scanner module: Performs all tasks related to replacing
*		references of Macrographs in the input stream, and for
*		the loading and clearing of macrographs.  The "inch" and
*		"INCH_Q" routines are to be used by the ReGIS parser module.
*		The Macrograph Memory is maintained by the MgMM module.
*
* ENVIRONMENT:	PDP-11
*
* AUTHOR:	J. A. Lomicka		CREATION DATE:	7-July-1981
*
* MODIFICATION HISTORY (please update IDENT above):
*
*  Eric Osman		 4-Oct-1993	BL-E
*	- Show parsed characters if DECTERM_SHOW_PARSING symbol seen.
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
*  Aston Chan		17-Dec-1991	V3.1
*	- I18n code merge
*
*  Bob Messenger       17-Jul-1990     X3.0-5
*      Merge in Toshi Tanimoto's changes to support Asian terminals -
*	- change ReGIS initialize sequences
*
*--------------End of DECterm edits-----------------------------------------
*
*	Edit	017	 8-May-84	/RDM
* Make regis reentrant by including regstruct.h
*
*	Edit	016	??-???-83	/RFD
* Translated the origional Bliss code into C.
*
*	Edit	015	14-Oct-83	/AFV				
* Change action taken when there is something in the buffer (sc_need_to_flush),
a* an instruction is active (sc_current_opcode) and we must go fetch more   
* data (in inch_q_unq).  Instead of terminating the buffer with an END_LIST 
* and a FLUSH_BUFFERS, just send the buffer the way it is.		   
*									   
*	Edit	014	16-Sep-83	/AFV				  
* Delete partial definition of a macrograph when interrupted		   
* by <ESC> \ <ESC> P 1 p						   
*									   
*	Edit	013	18-Aug-83	/AFV
* Change inch_q_unq to allow VT and FF through in QUOTED mode
*
*	Edit	012	17-Aug-83	/AFV
* add global routine rst_scanner to reset the scanner.  It 
* is called by REGIS.  This removes a dangling pointer 
* problem.
*
*	Edit	011	 9-Aug-83	/AFV
* Add global SC_CURRENT_OPCODE that is zero normally, but 
* when an END_LIST terminated opcode is in effect, it holds 
* that opcode.  Before flushing buffers, check the value.  
* if it is non-zero, send an END_LIST.  Set 
* SC_CURRENT_OPCODE to 0.
*
*	Edit	010	 8-Aug-83	/AFV
* Change to support 8 bit characters - ignore ALL C1 control 
* characters, treat GR characters as GR characters - don't 
* mask off that bit.
*
*	Edit	009	25-May-83	/AFV
* Change to support Gidis v2
*
*	Edit	008	24-Mar-83	/DCL
* Change PSECT allocation for VT240 overlays.
*
*	Edit	007	2-Feb-83	/DCL
* Install the ReGIS init string in this module; it didn't belong in RAM 
* space anyway.
*
*	Edit	006	28-Jan-83	/DCL
* Rip out the GLOBAL LITERAL method of version number tracking.  The 
* version number is now in the IDENT modifier of the MODULE heading.
*
*	Edit	005	28-Jan-83	/DCL
* Reverse order of EXTERNAL ROUTINE declarations and include file
* invokation so the MACRO redefinitions in REGVT240.REQ of the external
* routines can be applied properly. 
* Also, there is no need for reg_init to be an EXTERNAL symbol; it never 
* changes, so can be resolved at compile time.  Therefore, build the ReGIS 
* init string into this module instead of into REGISRAM.
*
*	Edit	004	15-Oct-82	/DCL
* Don't declare regblk and namblk to be external; that stuff is in REGBLK.REQ
* now.
*
*	Edit	003	15-Jun-82	/DCL
* Implement ReGIS init string.
*
*	Edit	002	8-Jan-82	/DCL
* Modify to use reentrant data structure.
*
*	Edit	001	13-July-1981	/JAL
* Checked out and found to be complete and correct, to my satisfaction.
*
*	Edit	000	7-July-1981	/JAL
* Skeletal definition
**/

#include "gidcalls.h"	/* only need FLUSH_BUFFER */
#include "reginit.h"
#include "regstruct.h"

static char *a_reg_init;

/*****			E X T E R N A L    F U N C T I O N S		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION			      MODULE */
/*	--------		-----------			      ------ */
extern
 char *getl_regis(),		/* Get a Line from the system	      OP_SYS */
      mac_append_character(),	/* Continue to define a macro	      MGMM   */
      mac_clear_all(),		/* Clear macrograph definitions	      MGMM   */
      mac_redefine(),		/* Start to define a macrograph	      MGMM   */
      open_regis();		/* Init. operating system interface   OP_SYS */
#if VARIANT == 1
extern
    PutBreak(),			/* Flush all pending terminal output  REGIO  */
    PutChar(),			/* Queue character on terminal output REGIO  */
    PutDecimal(),		/* Output signed decimal number	      OPSYS  */
    PutOctal(),			/* Output octal number		      OPSYS  */
    Put_String();		/* Output an ASCIZ string	      OPSYS  */
#endif

/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION			DESCRIPTION		       VALUE */
/*	--------			-----------		       ----- */
/**
*	clear_all_macrographs()						no
*	define_a_macrograph()						yes
*	expand_a_macrograph(v)						no
*	inch()								yes
*	inch_q_unq(v)							yes
*	new_scanner()							no
*	p_inch(v)							no
*	process_atsign()						yes
*	rst_scanner()							no
**/

clear_all_macrographs()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *
    * FORMAL PARAMETERS:
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:
    *
    * SIDE EFFECTS:
    **/

    /**
    * note:	The lines commented out clear all macrograph execution
    *	that is currently going on.  When commented out, this
    *	interpreter works like the VT125 in that an '@.' executed
    *	within a macrograph kill all pointers to macrograph
    *	starting positions, but the macrograph currently being 
    *	executed (plus those suspended) will continue as if there 
    *	were no '@.'.
    **/
								   
    /**
    *    extern	int	sc_source;
    *    extern	char	*sc_pointer;
    *    extern	char	*sc_stack[];
    *
    *    sc_source = 0;
    *    sc_pointer = sc_stack[sc_source];
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: clear_all_macrograph, scan.c\n", PutChar);
#endif
**/
    rs->mg_count_defined_mgs = 0;
    rs->mg_being_defined = (-1);						 /*2 */

    mac_clear_all();

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, clear_all_macrograph, scan.c\n", PutChar);
#endif
**/
}

define_a_macrograph()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *
    * FORMAL PARAMETERS:
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:
    *
    * SIDE EFFECTS:
    *
    * Note:	definition of macrographs is always from normal source
    * (Source #0) -- never from another macrograph.  This means
    * that there are no macrographs currently being executed or 
    * suspended on the source stack, which simplifies matters a lot.
    **/

    int		next_input_ch;						/*2 */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: define_a_macrograph, scan.c\n", PutChar);
#endif
**/

    next_input_ch = INCH_UNQUOTED(0);
    if ((next_input_ch >= 'A') && (next_input_ch <= 'Z'))
    /**
    *	 clause for defining a macrograph
    **/
    {

	rs->mg_being_defined = next_input_ch - 'A';			   /*2 */

	if (rs->mg_lengths[rs->mg_being_defined] > 0) 			   /*2 */
	{
	    rs->mg_count_defined_mgs--;
	}
	mac_redefine(rs->mg_being_defined);				   /*2 */

	while ( TRUE )
	{
	    int		exit;

	    exit = FALSE;
	    next_input_ch = INCH_Q(0);
	    if (next_input_ch == '@')	/* Check for terminator, "@;" */
	    {				/* and illegal "@:" */
		next_input_ch = INCH_Q(0);
		switch (next_input_ch)
		{
		    case ';' :
			exit = TRUE;
			break;
		    case ':' :
			break;
		    default :
			mac_append_character('@');
			mac_append_character(next_input_ch);
		}
		if ( exit )
		{
		    break;		/* exit while ( TRUE ) */
		}	
	    }				/* End if == '@' */
	    else			/* not == '@' */
	    {
		mac_append_character(next_input_ch);	
	    }
	}				/* End while ( TRUE ) */
	if (rs->mg_lengths[rs->mg_being_defined] > 0) 	/* something there  2 */
	{
	    mac_append_character('\0');	/* append a null to make an ASCIZ */
					/* string */
	    rs->mg_count_defined_mgs++;
	}
	rs->mg_being_defined = (-1);				  /*2 */
    }					/* End if macrograph name in range */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, define_a_macrograph, scan.c\n", PutChar);
#endif
**/
    return(INCH_UNQUOTED(0));					/*2 */
}

expand_a_macrograph( letter )
char	letter;
{
    /**
    * FUNCTIONAL DESCRIPTION:  Set up to read the next sequence of characters
    *	from a macrograph definition.
    *
    * FORMAL PARAMETERS:
    *	letter: the ascii value of the letter that defines the macrograph.
    *		Letter is not range checked.
    *
    * IMPLICIT INPUTS:	Macrograph stack state is updated.
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:
    *
    * SIDE EFFECTS:
    **/

    int		new_mg;		/* Macrograph number for this letter. */
    int		possible_recurse;
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: expand_a_macrograph, scan.c\n", PutChar);
#endif
**/

    new_mg = letter - 'A';

    /* don't bother doing anything if there is nothing in the macrograph. */
    /* note that this test avoids having to store nulls (to make ASCIZ */
    /* strings) for each empty macrograph. */

    if (rs->mg_lengths[new_mg] == 0)
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, expand_a_macrograph, scan.c\n", PutChar);
#endif
**/
	return;
    }
    /**
    *	Disallow implicit recursion
    **/

    for(possible_recurse =1; possible_recurse <= rs->sc_source; possible_recurse++)
    {
	if (rs->sc_macrograph[possible_recurse] == new_mg)
	{

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, expand_a_macrograph, scan.c\n", PutChar);
#endif
**/
	    return;
	}
    }
    /* save the current source */

    rs->sc_stack[rs->sc_source] = rs->sc_pointer;
    /**
    *	Update the active source to this macrograph
    *   no need to check sc_source against maximum -- there are
    * only 26 macrographs and they cannot recurse because of the
    * code above.  Another source is a PUSHED character -- this
    * is added to the stack like macrographs, but still the maximum
    * this can go to is 27.
    **/
    rs->sc_source++;
    rs->sc_pointer = rs->mg_begin[new_mg];
    rs->sc_macrograph[rs->sc_source] = new_mg;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #3, expand_a_macrograph, scan.c\n", PutChar);
#endif
**/
}

inch()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *	"inch" returns the next character of pure regis for processing. "inch"
    *	will ensure that Macrograph definitions and expansions are detected
    *	and performed.
    *
    * FORMAL PARAMETERS:
    *	None
    *
    * IMPLICIT INPUTS:
    *	Characters are gotten from the imported routine GetC_ReGIS or
    *	from Macrograph Memory Manager access functions.
    *
    * IMPLICIT OUTPUTS:
    *	if the characters in the input stream contain a macrograph definiton,
    *	it is loaded and saved before this routine returns.
    *
    * ROUTINE VALUE:
    *	The next available ReGIS stream character is returned.
    *
    * SIDE EFFECTS:
    *	The state of the Macrograph status is updated.
    **/

    int		inp_character;		/* Next character to return */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: inch, scan.c\n", PutChar);
#endif
**/
    inp_character = INCH_UNQUOTED(0);	/* Next character in stream */
    while (inp_character == '@')	/* Detect and deal with macrographs */
    {					/* until the character read is not */
	inp_character = process_atsign();/* a reference to a macrograph. */
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, inch, scan.c\n", PutChar);
#endif
**/
    return(inp_character);
}

inch_q_unq(unquoted)
int	unquoted;
{
    /**
    * FUNCTIONAL DESCRIPTION:  A character is retrieved from the input stream with
    *	minimum processing, that is, elimination of invalid control codes.
    *	This is where the decision of where to get characters from, macrograph,
    *	host, or init string, is made.
    *
    * FORMAL PARAMETERS:	none
    *
    * IMPLICIT INPUTS:	characters from getl_regis(), macrograph memory, or 
    *			the REGIS init string.
    *
    * IMPLICIT OUTPUTS:	updated macrograph state
    *
    * ROUTINE VALUE:	Retreived character
    *
    * SIDE EFFECTS:
    *
    * GLOBAL VARIABLES:	sc_pointer	ptr to ASCIZ string which is
    *					the active source.
    *			sc_stack []	stack of ptr's to macrographs
    *					temporarily preempted
    *			sc_source	stack pointer
    *			sc_macrograph []  # of macrograph on the stack
    *					in this position
    **/

#define		CH_MASK		0377
#define		CH_BLANK	' '
#define		CH_BS		010
#define		CH_CR		015
#define		CH_GLbegin	' '
#define		CH_GLEND	'~'
#define		CH_GRbegin	0240
#define		CH_GREND	0377

    int		input_char; 		/* Character looked for */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: inch_q_unq, scan.c\n", PutChar);
#endif
**/
#if VARIANT == 1
    Put_String( (" [scan: "), PutChar );
    PutBreak();
#endif

    while ( TRUE )
    {

	/* get next character from active source */

	input_char = ( *rs->sc_pointer & CH_MASK);
	rs->sc_pointer++;

#if VARIANT == 1
	Put_String( (" character = "), PutChar );
	PutDecimal( input_char, PutChar );
	PutBreak();
#endif

	/* Now figure out what to do if character should be ignored. */
	/* if the character is null (#o'0') this signifies the end of */
	/* this source, so pop back to the previous source.  if there */
	/* is no previous source, read another line from the system. */

	if (input_char == 0) 	/* must pop source */
	{

	    if (rs->sc_source > 0) 	/* there is a source to pop */
	    {
#if VARIANT == 1
		Put_String( (" [scan: pop 1 level]\n "), PutChar );
		PutBreak();
#endif
		rs->sc_source--;
	    }
	    else			/* no source, must read a line */
	    {
		if (IS_TRUE(rs->sc_need_to_flush) && (rs->sc_current_opcode == 0))
		{ 			   				/*3 */
		      G13_FLUSH_BUFFER();				/*3 */
		}							/*3 */
#if VARIANT == 1
		Put_String( (" scan: getl_regis returns "), PutChar );
		PutBreak();
#endif
		rs->sc_stack[0] = getl_regis();
#if VARIANT == 1
		PutOctal( rs->sc_stack[0], PutChar );
		Put_String( ("]\n"), PutChar );
		PutBreak();
#endif
		rs->sc_source = 0;
		rs->sc_need_to_flush = FALSE;
	    }
	    rs->sc_pointer = rs->sc_stack[rs->sc_source];
	}				/* End of input_char == 0 */
	else				/* input char is not 0 */
	{
	    {   /* show character if user asked to have characters shown */
#if defined(VMS_DECTERM) || defined(VXT_DECTERM)
	    noshare
#endif
	    extern char show_parsing_flag;
	    extern audit_chars ();
	    if (show_parsing_flag) audit_chars ("", 1, &input_char, "");  
	    }
	    if (IS_TRUE(unquoted))
	    {
		if  ((input_char > CH_BLANK) && (input_char <= CH_GLEND))
		{	    
#if VARIANT == 1
		    Put_String( (" ]\n "), PutChar );
		    PutBreak();
#endif

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, inch_q_unq, scan.c\n", PutChar);
#endif
**/
		    return( ((input_char >= 'a') && (input_char <= 'z')) ?
		       (input_char - ('a' - 'A'))  :  input_char);
		}
		else  	/* unquoted, but should be ignored */
		{
		}
	    }
	    else		/* quoted form */
	    {
	       if( (  (input_char >= CH_GLbegin) 	/* GL portion */
		   && (input_char <= CH_GLEND) 
		   )
		|| (  (input_char >= CH_GRbegin) 	/* GR portion */
		   && (input_char <= CH_GREND) 
		   )
		|| (  (input_char >= CH_BS) 		/* BS, HT, LF, */
		   && (input_char <= CH_CR)		/* VT, FF and CR */
		   ) )
		{
#if VARIANT == 1
		    Put_String( (" ]\n "), PutChar );
		    PutBreak();
#endif

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, inch_q_unq, scan.c\n", PutChar);
#endif
**/
		    return(input_char);
		}
		else  	/* quoted, but should be ignored */
		{
		} 		/* do nothing */
	    }
	}
    }				/* End of while TRUE */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #3, inch_q_unq, scan.c\n", PutChar);
#endif
**/
}				/* End of ROUTINE inch_q_unq */

new_scanner()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *	"new_scanner" resets the state of the module, initializing all
    *	the "own" space. 
    *
    * FORMAL PARAMETERS:
    *	None
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:
    *
    * SIDE EFFECTS:
    *	The state of the Macrograph status is updated.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: new_scanner, scan.c\n", PutChar);
#endif
**/

    clear_all_macrographs();
    open_regis();

    rs->sc_source = 0;			/* no macrographs being executed */
    if ( rs->kanji_regis )
    {
    if ( rs->num_planes > 2 )
      a_reg_init =
	";S(A[0,0][767,479],C1,I0)					\
	  T(A0,D0,S1,I0,B)						\
	  W(F7,I7,M1,V,N0,P1(M2),S1,S0)					\
	  P[0,0];";
    else if ( rs->num_planes > 1 )
      a_reg_init =
	";S(A[0,0][767,479],C1,I0)					\
	  T(A0,D0,S1,I0,B)						\
	  W(F3,I3,M1,V,N0,P1(M2),S1,S0) 				\
	  P[0,0];";
    else
      a_reg_init =
	";S(A[0,0][767,479],C1,I0)					\
	  T(A0,D0,S1,I0,B)						\
	  W(F1,I1,M1,V,N0,P1(M2),S1,S0) 				\
	  P[0,0];";
    rs->sc_pointer = a_reg_init;
    }
    else
    rs->sc_pointer = reg_init;		/* the first characters 'read' must */
					/* come from the REGIS init string  */

    /* the scanner will drop automatically into reading from */
    /* the primary source when 'reg_init' runs out. */

    rs->sc_current_opcode = 0;	/* don't need to send an END_LIST */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, new_scanner, scan.c\n", PutChar);
#endif
**/
}

p_inch(character)
char	character;
{
    /**
    *  push the given character onto the input stream
    *  this is done by saving the current sc_pointer in
    *  the stack and create a new pointer to this character,
    *  embedded into a very short ASCIZ string.
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */
    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: p_inch, scan.c\n", PutChar);
#endif
**/
    rs->sc_string[0] = character;
    rs->sc_string[1] = '\0';

    rs->sc_stack[rs->sc_source] = rs->sc_pointer;
    rs->sc_source++;
    rs->sc_pointer = rs->sc_string;

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, p_inch, scan.c\n", PutChar);
#endif
**/
}

process_atsign()
{
    /**
    * FUNCTIONAL DESCRIPTION: All @-sign prefixed commands are processed here.
    *
    * FORMAL PARAMETERS:
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE: After processing the at-sign, the next character to process
    *	by the caller is returned, as if from INCH_UNQUOTED.
    *
    * SIDE EFFECTS:
    **/

    int		inp_character;	/* Character from the input stream */
    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: process_atsign, scan.c\n", PutChar);
#endif
**/

    inp_character = INCH_UNQUOTED(0);	/* see what kind of command it is */
    if (inp_character == '.')
    {
	clear_all_macrographs(); 

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, process_atsign, scan.c\n", PutChar);
#endif
**/

	return( INCH_UNQUOTED(0) );
    }
    else if (inp_character == ':')
    {
	return( define_a_macrograph() );
    }
    else if ((inp_character >= 'A') && (inp_character <= 'Z'))
    {
	expand_a_macrograph(inp_character);

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, process_atsign, scan.c\n", PutChar);
#endif
**/

	return(INCH_UNQUOTED(0));
    }
    else
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #3, process_atsign, scan.c\n", PutChar);
#endif
**/

	return(inp_character);
    }

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #4, process_atsign, scan.c\n", PutChar);
#endif
**/

}

rst_scanner()
{
    /**
    * FUNCTIONAL DESCRIPTION:
    *	"rst_scanner" resets the state of the module, and
    *	resets the scan pointer (SC_POINTER) to point to
    *	a zero byte so that the next call to inch_q_unq will
    *	force a call to getl_regis.
    *	if, however, SC_POINTER is currently pointing to 
    *	REG_INIT, nothing is done, because NEW_REGIS just 
    *	called new_scanner.
    *
    *	9/16/83 added code to check mg_being_defined			   2
    *	and get rid of it.						   2
    *									   2
    * FORMAL PARAMETERS:
    *	None
    *
    * IMPLICIT INPUTS:
    *
    * IMPLICIT OUTPUTS:
    *
    * ROUTINE VALUE:
    *
    * SIDE EFFECTS:
    **/

    register struct regis_cntx *rs;	/* pointer to context block */

    rs = RSTRUCT;			/* point to it */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n BEGIN: rst_scanner, scan.c\n", PutChar);
#endif
**/

    if ( rs->kanji_regis )
    {
    if ((rs->sc_pointer - a_reg_init) == 0)
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, rst_scanner, scan.c\n", PutChar);
#endif
**/
	return;
    }
    }
    else
    if ((rs->sc_pointer - reg_init) == 0)
    {

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #1, rst_scanner, scan.c\n", PutChar);
#endif
**/
	return;
    }
    rs->sc_source = 0;			/* no macrographs being executed */
    rs->sc_pointer = rs->sc_string;
    rs->sc_string[0] = '\0';

    rs->sc_current_opcode = 0;	/* don't need to send an END_LIST */

    /* if we were interrupted in the middle of defining a		   3 */
    /* macrograph, delete the macrograph now.				   2 */
								   /*2 */
    if (rs->mg_being_defined >= 0) 					   /*2 */
    {								   /*2 */
	mac_redefine(rs->mg_being_defined);				   /*2 */
	rs->mg_being_defined = (-1);				   /*2 */
    }								   /*2 */

    /* debug *RFD* */
/**
#if VARIANT == 1
    PutString( "\n EXIT: #2, rst_scanner, scan.c\n", PutChar);
#endif
**/

}
/**
*	CMS REPLACEMENT HISTORY 
**/
/**
*	*3 A_VESPER 21-OCT-1983 16:30:50 "fixed G command bug"
*	*2 A_VESPER 16-SEP-1983 16:11:46 "fixup macrograph definition after P1p interruption"
*	*1 A_VESPER 14-SEP-1983 14:02:15 ""
**/
