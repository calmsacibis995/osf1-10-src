/* #module WV_PARSER "X03-306" */
/*
 *  Title:	WV_PARSER
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © 1985, 1993                                                 |
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
 *	Callable re-entrant ANSI parser.  This module contains the entire
 *	parser and action routines which alter the parse state.
 *
 *  Environment:  VAX/VMS    VAX-11 C  V1.2
 *
 *  Author:  Peter Sichel    24-Feb-1984
 *
 *  Modified by:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 *  Aston Chan		17-Dec-1991	V3.1
 *	- I18n code merge
 *
 *  Bob Messenger	17-Jan-1989	X1.1-1
 *	- Moved many ld fields to common area
 *
 *	PAS 30-May-84  added missing "break" to switch case "IGNORE".
 *
 *	JR  22-Aug-84  change to refer to static state data structure for
 *	               performance reasons.
 *
 *	FGK0001 01-Nov-85  modified for use in WV_DRIVER for MicroVMS (UIS)
 *
 *	FGK0002 28-May-86  "SUB" bug.  set incorrect parse state.
 *
 *	FGK0003 22-Jul-86  Update version number to X04-107
 *
 *	FGK0004 12-Jan-87  Move pars_init from wv_termmgr into this module
 *
 *	FGK0005 16-Apr-87  Change data type of ld
 *
 *	FGK0006 21-Apr-87  Mass edit of symbols (here just changes _state macro)
 */

#include "wv_hdr.h"

#define _state _cld wvt$r_com_state.

static char eseq_parse_table[12] = {
    0x32, 0x31, 0x32,
    0x12, 0x21, 0x12,
    0x42, 0x21, 0x42,
    0x02, 0x21, 0x02
};

static char pseq_parse_table[42] = {
    0x34, 0x34, 0x34, 0x34, 0x34, 0x34,
    0x11, 0x14, 0x14, 0x14, 0xF4, 0x14,
    0xE2, 0xE2, 0xE2, 0x14, 0xF4, 0xE2,
    0xE5, 0xE5, 0xE5, 0x14, 0xF4, 0xE5,
    0x13, 0x13, 0x13, 0x13, 0xF3, 0x13,
    0x54, 0x54, 0x54, 0x54, 0xD4, 0x54,
    0x14, 0x14, 0x14, 0x14, 0xF4, 0x14
};

/*
 *  
 *  Interpreting return information from parse_ansi
 *  -----------------------------------------------
 *  
 *  parse_ansi returns with "event" set to one of the following:
 *  
 *      R_PARSE_ERROR     (parse stack overflow, or software bug)
 *      R_CONTINUE        no event, continue parsing
 *      R_GRAPHIC         recognized a graphic character
 *      R_CONTROL         recognized a control character
 *      R_ESC_SEQ         recognized an escape sequence
 *      R_CSI_SEQ         recognized a control sequence
 *      R_DCS_SEQ         recognized a DCS sequence
 *  
 *  
 *  The character or character sequence recognized is on the "state" stack.
 *  parse_ansi returns two integers pointing to the first and final
 *  characters of the sequence on the stack.
 *  
 *  Each stack entry has three fields, a "class" field, a "value" field and
 *  a "data" field.  The possible values for these fields are defined in the
 *  include files "gparse.h", and "lparse.h".
 *  
 *  Examples of typical stack data follows:
 *  
 *               value        data
 *            ----------    --------
 *  final ->  CON_START      41 'A'    event = R_GRAPHIC (the letter "A")
 *  
 *  final ->  CON_START      0D        event = R_CONTROL (carriage return)
 *  
 *  final ->  ES_IN_SEQ      41 'A'    event = R_ESC_SEQ
 *            ES_IN_SEQ      28 '('    (designate UK into G0)
 *  start ->  SEQ_START      1B ESC
 *  
 *  final ->  CS_IGNORE      68 'h'    event = R_CSI_SEQ
 *            CS_PARAM       3         parameter value 3
 *            CS_PRIVATE     3F '?'    DEC private char
 *  start ->  SEQ_START      9B CSI    (set 132 col mode)
 *  
 *  final ->  CS_IGNORE      72 'r'    event = R_CSI_SEQ
 *            CS_PARAM       14        parameter value 14(hex)
 *            CS_PARAM       1         parameter value 1
 *  start ->  SEQ_START      9B CSI    (set scroll region from line 1 to 18)
 *  
 *  final ->  CS_IGNORE      72 'p'    event = R_DCS_SEQ
 *  start ->  SEQ_START      90 DCS    (begin REGIS device control string)
 *  
 *  final ->  CON_START      9C ST     event = R_CONTROL (string terminator)
 *  
 */

/***************************************************************************/
parse_ansi(ld, c, event, start, final) /* re-entrant ansi parser executive */
/***************************************************************************/

wvtp ld;
int c;	          /* input character */
int *event;       /* event recognized (by ref)                        */
int *start;       /* stack index of first character of event (by ref) */
int *final;       /* stack index of final character of event (by ref) */

{
register int sp;
int action;

c &= 0xFF;

/*..............................................*/


	/* if DEL code during ESC sequence, then ignore it.	*/
   if (_state class[_state index] == ESEQ && c == 0x7F ) {
	*event = R_CONTINUE;
	return(*event);
   }

    /* if in sequence, convert 8 bit graphics to 7 bits */
    if (_state class[_state index] <= LAST_SEQ)
	if (c >= 0xA0) c &= 0x7F; 

    /* invoke appropriate parser based on state class */
    switch (_state class[_state index])
	{
	case CON:
	case ESEQ:
	    /* parse escape sequences */
            action = parse(ld, c, eseq_class(ld, c), eseq_parse_table);
	    break;

	case CSI:
	case DCS:
	    /* parse parameter sequence */
            action = parse(ld, c, pseq_class(ld, c), pseq_parse_table);
	    break;
	}


    /* process recognized events */
    switch (action)
	{
	case CONTINUE:
            *event = R_CONTINUE;
            break;

	case GRAPHIC:
            *event = R_GRAPHIC;
            *start = _state index;
            *final = _state index;
	    _state index -= 1;
            break;

	case CONTROL:
	    pcontrol(ld, event, start, final);
	    break;

	case ESC_SEQ:
	    pescseq(ld, event, start, final);
	    break;

	case PAR_SEQ:
            /* find start of sequence */
            for (sp=_state index; _state value[sp] != SEQ_START; sp--)
                ;
            *start = sp;
            *final = _state index;
            switch (_state data[*start])
		{
                case 0x9B: *event = R_CSI_SEQ; break;
                case 0x90: *event = R_DCS_SEQ; break;
                default:   *event = R_PARSE_ERROR; break;
		}
            /* remove seq from parse stack, and return to previous parser */
            _state index = *start - 1;
	    break;

	case PARAM:

		/*  input was either a numeral, or a semicolon,
		 *  reduce parameter chars to parameter value
	         *  if current char is a numeral then
		 *    if prev is a value, mul by 10 and add current.
		 *  else store current as a value.
		 *    else if current is a semicolon
		 *      store value of zero
		 *  if prev is not a value,
		 *    advance stack and store a value of zero.
		 */

            *event = R_CONTINUE;
            sp = _state index;

	    if (_state data[sp] != 0x3B)  /* numeral */
		{
		 if (_state value[sp-1] == CS_OMIT)
                     _state value[sp-1] = CS_PARAM;
		 if (_state value[sp-1] == CS_PARAM)
			{
			 _state data[sp-1] *= 10;
			 _state data[sp-1] += _state data[sp] - 0x30;
			 _state index = sp - 1;
			}
		 else
			{
			 _state data[sp] -= 0x30;
			}
		}
	    else  /* semicolon */
		{
		 _state data[sp] = 0;
		 if ((_state value[sp-1] != CS_PARAM) &&
                     (_state value[sp-1] != CS_OMIT))
			{
			 _state class[sp+1] = _state class[sp];
			 _state value[sp+1] = CS_OMIT;
			 _state data[sp+1] = 0;
			 _state index = sp+1;
			}
		}
            break;

        case CANCEL:
            if (_state class[_state index] <= LAST_SEQ)
		{
		int sp;
		sp = _state index;
		while (_state value[ sp ] != SEQ_START)
		    sp--;
		if ( _state data[sp] == 0x90 )
		    _cld wvt$b_in_dcs = IGNORE_DCS;
		else
		    _state index = sp - 1;
		}
            *event = R_CONTINUE;
            break;

	case P_IGNORE:
            *event = R_CONTINUE;
	    _state index -= 1;
	    break;

	default:
            *event = R_PARSE_ERROR;
            pars_init(ld);    /* re-initialize */
	    break;

    }   /* end switch event */

    return(*event);

}   /* end routine parse_ansi */


/*************************************/
eseq_class(ld, c) /* get input class */
/*************************************/

wvtp ld;
int c;

/*
 *
 * eseq_class - return escape sequence parser input class
 *              of given character.
 *
 */

{
int c_class;

    /* determine character class */
    if (0x20 <= c && c <= 0x2F)
    c_class = ES_INTER;
	else if ((0x00 <= c && c <= 0x1F) ||
		 (0x7F <= c && c <= 0x9F))
	c_class = ES_CONTROL;
		else if (0x30 <= c && c <= 0x7E)
		c_class = ES_FINAL;
			else c_class = ES_GRAPHIC;

    return(c_class);
}

/*************************************/
pseq_class(ld, c) /* Get input class */
/*************************************/

wvtp ld;
int c;

/*
 *
 *  pseq_class - return parameter sequence parser input class of
 *  given character
 *
 */

{

int pi_class;

    /* determine character class */
    if (0x30 <= c && c <= 0x39)
    pi_class = PI_NUMERAL;   
	else if (c == 0x3B)
	pi_class = PI_SEMICOL;   
		else if (0x20 <= c && c <= 0x2F)
		pi_class = PI_INTER;      
			else if (0x3C <= c && c <= 0x3F)
			pi_class = PI_PRIVATE;    
				else if (0x40 <= c && c <= 0x7E)
				pi_class = PI_FINAL;  
					else if ((0x00 <= c && c <= 0x1F) ||
						 (0x7F <= c && c <= 0x9F))
					pi_class = PI_CONTROL; 
						else pi_class = PI_OTHER;

    return(pi_class);
}

/***************************************************/
parse(ld, c, c_class, parse_table) /* do the parse */
/***************************************************/

wvtp ld;
int c;                         /*  input character      */
int c_class;                   /*  input class          */
char parse_table[];            /*  parse table          */

/*
 *
 * parse - table driven parse routine
 *
 * returns the following events:
 *   PARSE_ERROR
 *   CONTINUE
 *   GRAPHIC
 *   CONTROL
 *   ESC_SEQ
 *   CON_SEQ
 *   PARAM
 *   CANCEL
 *   IGNORE
 *
 * The parser is a table driven finite state automata, where the next state
 * and action are a function of the current state and input.
 *
 */

{

int next_action;
int current_state;
int next_state;
register int sp;

/*...................................*/

    /* get next state */
    sp = _state index;
    current_state = _state value[sp];
    next_state = parse_table[c_class + current_state] & 0x0F;

    /* get action */
    next_action = parse_table[c_class + current_state] & 0xF0;

    /* default action */
    if (sp >= STACK_SIZE) return(PARSE_ERROR);
    sp += 1;
    _state class[sp] = _state class[sp-1];
    _state value[sp] = next_state;
    _state data[sp] = c;
    _state index = sp;

    /* return action */
    return(next_action);

}   /* end routine parse */


/******************************************************/
pcontrol(ld, event, start, final) /* process controls */
/******************************************************/

wvtp ld;
int *event;
int *start;
int *final;

/*
 *
 * pcontrol - process controls that alter
 *            the parse state.
 *
 */
{

register int sp;
int code;

    sp = _state index;
    code = _state data[sp];

    /* ESC */
    if (code == 0x1B)

		{

	         /* if already in escape or control sequence, restart it */
	         if (_state class[sp] <= LAST_SEQ)
	            while (_state value[sp] != SEQ_START)
	                sp -= 1;
	         _state class[sp] = ESEQ;	/* switch to escape seq parser */
	         _state value[sp] = SEQ_START;
	         _state data[sp] = 0x1B;
	         _state index = sp;
	         *event = R_CONTINUE;
	         return(0);
		}

    /* CAN */
    if (code == 0x18)
		{
		/* if already in DCS sequence, set the flag.	*/
		if ( _state class[sp] == DCS ) {
			_cld wvt$b_can_sub_detected = 1;
			sp -= 1;
			_state index = sp;
			*event = R_CONTINUE;
			return(0);
			}
		 /* if already in escape or control sequence, cancel it */
		 if (_state class[sp] <= LAST_SEQ)
			{
			 while (_state value[sp] != SEQ_START)
			 sp -= 1;
			 sp -= 1;
			}
		 _state index = sp;
		 *event = R_CONTINUE;
	         return(0);
		}

    /* SUB */
    if (code == 0x1A)
		{
		/* if already in DCS sequence, set the flag.	*/
		if ( _state class[sp] == DCS ) {
			_cld wvt$b_can_sub_detected = 1;
			sp -= 1;
			_state index = sp;
			*event = R_CONTINUE;
			return(0);
			}
		 /* if already in escape or control sequence, cancel it */
		 if (_state class[sp] <= LAST_SEQ)
			{
			 while (_state value[sp] != SEQ_START)
			 sp -= 1;
			 _state index = sp-1;
			 *event = R_CONTINUE;
/****************************************************************/
/*	Displayed SUB(1Ah) at used in escape sequence		*/
/*				and CSI, DCS sequence		*/
/*				~~~~~~~~~~~~~~~~~~~~~		*/
	                if (_state class[sp]==ESEQ || _state class[sp]==CSI ||
			    _state class[sp]==DCS ) {
	                    xinsertchar(ld, 128);
	                    display_segment(ld, _ld wvt$l_actv_line, _ld wvt$l_disp_pos,
				_ld wvt$l_actv_column - _ld wvt$l_disp_pos);
	                    _ld wvt$l_vt200_specific_flags &= ~vts1_m_last_column;
	                    }
			}
		 else
			{
			 *event = R_CONTROL;
			 _state data[sp] = code;
			 *start = sp;
			 *final = sp;
			 _state index = sp - 1;
			}
		 return(0);
		}


    /* is it a C1 control ? */
    if (code > 0x7F)
		{
		 /* if already in escape or control sequence, cancel it */
		 if (_state class[sp] <= LAST_SEQ)
		 while (_state value[sp] != SEQ_START)
			sp -= 1;

		 /* CSI */
		 if (code == 0x9B)
			{
			 /* switch to parameter seq parser */
			 _state class[sp] = CSI;
			 _state value[sp] = SEQ_START;
			 _state data[sp] = 0x9B;
			 _state index = sp;
			 *event = R_CONTINUE;
			 return(0);
			}

		 /* DCS */
		 if (code == 0x90)
			{
			 /* switch to parameter seq parser */
			 _state class[sp] = DCS;
			 _state value[sp] = SEQ_START;
			 _state data[sp] = 0x90;
			 _state index = sp;
			 *event = R_CONTINUE;
			 return(0);
			}

		}   /* end C1 control */


    /* recognize other controls */
    _state data[sp] = code;
    *event = R_CONTROL;
    *start = sp;
    *final = sp;
    _state index = sp - 1;
    return(0);
}

/*************************************************************/
pescseq(ld, event, start, final) /* process escape sequences */
/*************************************************************/

wvtp ld;
int *event;
int *start;
int *final;

/*
 *
 * pescseq - process escape sequences
 *           that alter the parse state.
 *
 */

{

register int sp;

    sp = _state index;

    /* two character escape sequence ? */
    if (_state data[sp-1] == 0x1B)
	{
	 /*
	  *  Convert 7-bit Fe sequences to C1 controls
	  *  The finals will range from "@" (hex 40) to "_" (hex 5f)
	  */
	 if ((0x40 <= _state data[sp]) &&  (_state data[sp] <= 0x5F))
		{
		 _state data[sp-1] = _state data[sp] + 0x40; /* Convert to C1 control */
		 _state index = sp-1; /* Adjust stack */
		 /* steal code from pcontrol */
		 pcontrol(ld, event, start, final);
		 return(0);
		}
	}

    /* recognize other escape sequences */
    *event = R_ESC_SEQ;

    /* find start of sequence */
    for (sp=_state index; _state value[sp] != SEQ_START; sp--)
	;

    *start = sp;
    *final = _state index;
    _state index = sp-1;   /* remove seq from parse stack */
    return(0);

}   /* end routine pescseq */

/******************************************************/
pars_init(ld)   /* pars_init - initialize parse state */
/******************************************************/

wvtp ld;

{
    _state index = 0;
    _state class[0] = CON;
    _state value[0] = CON_START;
    return(0);
}
