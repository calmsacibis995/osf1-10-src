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
#ifdef OSF1
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calcutils.c,v 1.1.5.2 1993/04/13 22:54:30 Ronald_Hegli Exp $";
#endif
#endif
/* Header from VMS source pool
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calcutils.c,v 1.1.5.2 1993/04/13 22:54:30 Ronald_Hegli Exp $";
*/
/*
**++

  Copyright (c) Digital Equipment Corporation,
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.

**--
**/

/*
**++
**  MODULE NAME:
**	calcutils.c
**
**  FACILITY:
**      OOTB Calculator
**
**  ABSTRACT:
**	Main module for the OOTB calculator
**
**   	This application demonstrates a simple calculator using the X11 and
**   	its Toolkit; Up to 15 (MAX_BUFF) digits can be displayed in the display
**   	window. This does not include the negative sign or the decimal point.
**
**   	The calculator can be displayed in 3 different sizes. Three fonts
**   	are opened at the beginning of the program and the different sizes
**   	are computed based on the font sizes.
**
**   	Thanks to John H. Bradley of UPenn whose X10 program: xcalc.c
**   	I used to get started on this.
**
**
**  AUTHORS:
**      Dennis McEvoy, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/

#include "calcdefs.h"
#include "calc.h"

#include <signal.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>

external CalcWidgetClass calcwidgetclass;

/* forward declarations	*/
void getnumstring();
void ReverseKey();
CalcWidget calc_FindMainWidget();
keyinfo *FindWidgetId();
void error();
void mem_error();
void display_in_window();
void mem_value_dis();
void CalcLoadFontFamilies();
void ParseCS();
void calc_initialize();

extern void calc_get_uil_string();
extern void save_state();
extern void swap_state();
extern void HighlightKey();
extern void UnHighlightKey();

/*
**++
**  ROUTINE NAME:
**	calc_initialize (mw)
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void calc_initialize(mw)
    CalcWidget mw;
{
    Mode(mw) = k_decimal;
    strcpy(OutputFormatStr(mw), "%.15lf");
    strcpy(InputFormatStr(mw), "%lf");

    BufferCount(mw) = 0;
    Buffer(mw)[0] = '\0';

    Strmax(mw) = MAX_BUFF;
    CurrentOp(mw) = EMPTY;
    ErrorFlag(mw) = FALSE;
    strcpy(BufferCopy(mw), ZeroPoint(mw));

    NumStackCount(mw) = 1;
    NumStack(mw)[0] = 0;
    NumStack(mw)[1] = 0;

    MemoryFlag(mw) = FALSE;
    MemErrorFlag(mw) = FALSE;
    MemValue(mw) = 0;

    DisplaySelected(mw) = TRUE;
    UndoEnabled(mw) = FALSE;

    LogoGC(mw) = 0;
    CalcGC(mw) = 0;

    ButtonPressed(mw) = NULL;
    KeyPressed(mw) = NULL;

    HighNo(mw) = (pow(10.0, (double)MAX_BUFF)) - 1;
    LowNo(mw) = -HighNo(mw);

    TrigType(mw) = DEGREES;
    InvsEnabled(mw) = FALSE;

    /* Seed the random number generator   */
    srand(time(NULL));
}

/*
**++
**  ROUTINE NAME:
**	xfact (mw, number)
**
**  FUNCTIONAL DESCRIPTION:
**	return the factorial of the number
**
**  FORMAL PARAMETERS:
**    	double number - the number to be computed
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	The factorial of the number
**
**  SIDE EFFECTS:
**--
**/
double xfact(mw, number)
    CalcWidget mw;
    double number;
{
    double result;
    int i;

    if (number < 0)
	return (HighNo(mw) + 1);
    if (number > floor(number))
	return (HighNo(mw) + 1);

    result = 1;
    for (i = 1; i <= number; i++)
	if ((result < HighNo(mw)) && (result > LowNo(mw)))
	    result = result * i;
	else
	    return (HighNo(mw) + 1);

    return (result);
}

/*
**++
**  ROUTINE NAME:
**	genrandom (mw, number)
**
**  FUNCTIONAL DESCRIPTION:
**	return a random number
**
**  FORMAL PARAMETERS:
**    	double number - the bound of the number (1 to number or -1 to number)
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	The factorial of the number
**
**  SIDE EFFECTS:
**
** NOTE: There is a bug in tha VAX c random number generator function.
**       The rand () function is cyclic on powers of 2 less than 2**16.
**       The simple fix is to always use an odd number and then adjust
**       the result (ignore 0's).
**--
**/
double genrandom(mw, number)
    CalcWidget mw;
    double number;
{
    double result;
    int i, res, num;

    if (number < 2)
	return (HighNo(mw) + 1);

    num = (int) floor(number);
    res = 0;

    if (num % 2 == 0) {
	num++;
	while (res == 0)
	    res = rand() % num;
    } else {
	res = rand() % num;
	res++;
    }

    result = (double) res;
    return (result);
}

/*
**++
**  ROUTINE NAME:
**	key_hit (mw, keynum)
**
**  FUNCTIONAL DESCRIPTION:
**   	Invert the pressed key window and call the appropriate routine
**	for the key struck.
**
**  FORMAL PARAMETERS:
**    	int keynum - the number of the key hit
**
**  IMPLICIT INPUTS:
** 	key window id's, error_flag, info_window_up
**
**  IMPLICIT OUTPUTS:
**	UndoEnabled  (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
key_hit(mw, keynum, fromclip)
    CalcWidget mw;
    int keynum;
    int fromclip;
{

    if ((!fromclip) && (keynum != k_key_degrees) && (keynum != k_key_inverse))
	save_state(mw);

    if (ErrorFlag(mw))
	display_clear(mw);

    if (MemErrorFlag(mw))
	mem_clear(mw);

    switch (keynum) {

	/* The number keypad keys */
	case k_key_0: 
	case k_key_1: 
	case k_key_2: 
	case k_key_3: 
	case k_key_4: 
	case k_key_5: 
	case k_key_6: 
	case k_key_7: 
	case k_key_8: 
	case k_key_9: 
	case k_key_period: 

	/* The number hexadecimal keys */
	case k_key_a: 
	case k_key_b: 
	case k_key_c: 
	case k_key_d: 
	case k_key_e: 
	case k_key_f: 
	    process_digit(mw, keynum);
	    break;

	/* The two operators keys */
	case k_key_ytothex: 
	case k_key_divide: 
	case k_key_times: 
	case k_key_minus: 
	case k_key_plus: 

	/* The logical two operators */
	case k_key_and: 
	case k_key_or: 
	case k_key_xor: 
	case k_key_nor: 
	    process_operator2(mw, keynum);
	    break;

	/* The single operators keys */
	case k_key_sine: 
	case k_key_cosine: 
	case k_key_tangent: 
	case k_key_xfact: 
	case k_key_1overx: 
	case k_key_logrithm: 
	case k_key_natlog: 
	case k_key_pi: 
	case k_key_square_root: 
	case k_key_percent: 
	case k_key_random: 
	case k_key_plus_minus: 

	/* The logical single operators */
	case k_key_not: 
	case k_key_neg: 
	    process_operator1(mw, keynum);
	    break;

	/* The special function keys */
	case k_key_clear: 
	    display_clear(mw);
	    break;

	case k_key_equals: 
	    process_equals(mw);
	    break;

	case k_key_degrees: 
	    change_degrees(mw);
	    break;

	case k_key_inverse: 
	    switch_invserse(mw);
	    break;

	/* The memory keys */
	case k_mem_clear: 
	    mem_clear(mw);
	    break;
	case k_mem_recall: 
	    mem_recall(mw);
	    break;
	case k_mem_plus: 
	    mem_plus(mw);
	    break;
	case k_mem_minus: 
	    mem_minus(mw);
	    break;
	case k_key_clear_entry: 
	    clear_entry(mw);
	    break;

	default: 
	    printf("Unknown Key was kit, value %d \n", keynum);
	    break;
    }
}

/*
**++
**  ROUTINE NAME:
**	typechar (mw, c, fromclip)
**
**  FUNCTIONAL DESCRIPTION:
**  	user has typed a char on the keyboard, use it
**  	if it is valid, beep if it is invalid.
**
**  FORMAL PARAMETERS:
**	char *str -- the string representing the typed
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
typechar(mw, str, fromclip)
    CalcWidget mw;
    char str[10];
    int fromclip;
{
    int i, key = 0;

    key = -1;

    if (strcmp(str, "c") == 0)
	key = k_key_clear;
    else if (strcmp(str, "ce") == 0)
	key = k_key_clear_entry;

    else if (strcmp(str, "0") == 0)
	key = k_key_0;
    else if (strcmp(str, "1") == 0)
	key = k_key_1;
    else if (strcmp(str, "2") == 0)
	key = k_key_2;
    else if (strcmp(str, "3") == 0)
	key = k_key_3;
    else if (strcmp(str, "4") == 0)
	key = k_key_4;
    else if (strcmp(str, "5") == 0)
	key = k_key_5;
    else if (strcmp(str, "6") == 0)
	key = k_key_6;
    else if (strcmp(str, "7") == 0)
	key = k_key_7;
    else if (strcmp(str, "8") == 0)
	key = k_key_8;
    else if (strcmp(str, "9") == 0)
	key = k_key_9;
    else if (strcmp(str, ".") == 0)
	key = k_key_period;
    else if (strcmp(str, "pi") == 0)
	key = k_key_pi;

    else if (strcmp(str, "/") == 0)
	key = k_key_divide;
    else if (strcmp(str, "*") == 0)
	key = k_key_times;
    else if (strcmp(str, "+") == 0)
	key = k_key_plus;
    else if (strcmp(str, "-") == 0)
	key = k_key_minus;
    else if (strcmp(str, "+-") == 0)
	key = k_key_plus_minus;
    else if (strcmp(str, "%") == 0)
	key = k_key_percent;
    else if (strcmp(str, "=") == 0)
	key = k_key_equals;

    else if (strcmp(str, "inv") == 0)
	key = k_key_inverse;
    else if (strcmp(str, "sin") == 0)
	key = k_key_sine;
    else if (strcmp(str, "cos") == 0)
	key = k_key_cosine;
    else if (strcmp(str, "tan") == 0)
	key = k_key_tangent;
    else if (strcmp(str, "log") == 0)
	key = k_key_logrithm;
    else if (strcmp(str, "nat_log") == 0)
	key = k_key_natlog;
    else if (strcmp(str, "sqrt") == 0)
	key = k_key_square_root;
    else if (strcmp(str, "y^x") == 0)
	key = k_key_ytothex;

    else if (strcmp(str, "deg") == 0)
	key = k_key_degrees;
    else if (strcmp(str, "x!") == 0)
	key = k_key_xfact;
    else if (strcmp(str, "1/x") == 0)
	key = k_key_1overx;
    else if (strcmp(str, "rnd") == 0)
	key = k_key_random;

    else if (strcmp(str, "mc") == 0)
	key = k_mem_clear;
    else if (strcmp(str, "mr") == 0)
	key = k_mem_recall;
    else if (strcmp(str, "m+") == 0)
	key = k_mem_plus;
    else if (strcmp(str, "m-") == 0)
	key = k_mem_minus;

    else if (strcmp(str, "ha") == 0)
	key = k_key_a;
    else if (strcmp(str, "hb") == 0)
	key = k_key_b;
    else if (strcmp(str, "hc") == 0)
	key = k_key_c;
    else if (strcmp(str, "hd") == 0)
	key = k_key_d;
    else if (strcmp(str, "he") == 0)
	key = k_key_e;
    else if (strcmp(str, "hf") == 0)
	key = k_key_f;

    else if (strcmp(str, "not") == 0)
	key = k_key_not;
    else if (strcmp(str, "neg") == 0)
	key = k_key_neg;

    else if (strcmp(str, "and") == 0)
	key = k_key_and;
    else if (strcmp(str, "or") == 0)
	key = k_key_or;
    else if (strcmp(str, "xor") == 0)
	key = k_key_xor;
    else if (strcmp(str, "nor") == 0)
	key = k_key_nor;

    else if (strcmp(str, "delete") == 0)
	backspace_key(mw);

    if (key != -1) {
	if ((key < DIGIT_BASE) && (key >= MEM_BASE))
	    KeyPressed(mw) = &Mems(mw)[key - MEM_BASE];
	else if (key < OPS_BASE)
	    KeyPressed(mw) = &Keys(mw)[key - DIGIT_BASE];
	else if (key < CLEAR_BASE)
	    KeyPressed(mw) = &Ops(mw)[key - OPS_BASE];
	else if (key < FUNC_BASE)
	    KeyPressed(mw) = &Clears(mw)[key - CLEAR_BASE];
	else if (key < INV_FUNC_BASE)
	    KeyPressed(mw) = &Func(mw)[key - FUNC_BASE];
	else if (key < HEX_BASE)
	    KeyPressed(mw) = &InvFunc(mw)[key - INV_FUNC_BASE];
	else if (key < LOGICAL_BASE)
	    KeyPressed(mw) = &InvFunc(mw)[key - HEX_BASE];
	else
	    KeyPressed(mw) = &Func(mw)[key - LOGICAL_BASE];

	if (key == k_key_inverse) {
	    if (InvsEnabled(mw))
		UnHighlightKey(mw, KeyPressed(mw));
	    else
		HighlightKey(mw, KeyPressed(mw));
	    KeyPressed(mw) = NULL;
	} else
	    HighlightKey(mw, KeyPressed(mw));

	XSync(Dpy(mw), 0);

	key_hit(mw, key, fromclip);

	if (fromclip) {
	    XSync(Dpy(mw), 0);
	    if (key != k_key_inverse)
		UnHighlightKey(mw, KeyPressed(mw));
	}
    }
}

/*
**++
**  ROUTINE NAME:
**	process_digit (mw, dig_num)
**
**  FUNCTIONAL DESCRIPTION:
**  	add the key struck to the buffer
**  	iff the buffer is not full, update display.
**
**  FORMAL PARAMETERS:
**	int dig_num -- the number of the digit to process
**
**  IMPLICIT INPUTS:
** 	buffer, BufferCount (mw)
**
**  IMPLICIT OUTPUTS:
**	strmax, buffer, BufferCount (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
process_digit(mw, dig_num)
    CalcWidget mw;
    int dig_num;
{
    char dig_char[5];

    if (dig_num == k_key_period)
	Strmax(mw) = MAX_BUFF + 1;

    if ((BufferCount(mw) == 0) && (CurrentOp(mw) == EMPTY))
	NumStackCount(mw) = 0;

    else if (BufferCount(mw) == Strmax(mw))
	return;

    switch (dig_num) {
	case k_key_period: 
	    if (strchr(Buffer(mw), ChrPoint(mw)) != NULL)
		return;
	    else {
		if (BufferCount(mw) == 0) {
		    strcat(Buffer(mw), Keys(mw)[0].label);
		    BufferCount(mw) = 1;
		}
		strcpy(dig_char, Keys(mw)[1].label);
	    }				/* end else */
	    break;

	case k_key_0: 
	    if (strcmp(Buffer(mw), Keys(mw)[0].label) == 0)
		return;
	    else
		strcpy(dig_char, Keys(mw)[0].label);
	    break;

	case k_key_1: 
	    strcpy(dig_char, Keys(mw)[3].label);
	    break;

	case k_key_2: 
	    strcpy(dig_char, Keys(mw)[4].label);
	    break;

	case k_key_3: 
	    strcpy(dig_char, Keys(mw)[5].label);
	    break;

	case k_key_4: 
	    strcpy(dig_char, Keys(mw)[6].label);
	    break;

	case k_key_5: 
	    strcpy(dig_char, Keys(mw)[7].label);
	    break;

	case k_key_6: 
	    strcpy(dig_char, Keys(mw)[8].label);
	    break;

	case k_key_7: 
	    strcpy(dig_char, Keys(mw)[9].label);
	    break;

	case k_key_8: 
	    strcpy(dig_char, Keys(mw)[10].label);
	    break;

	case k_key_9: 
	    strcpy(dig_char, Keys(mw)[11].label);
	    break;

	case k_key_a: 
	    strcpy(dig_char, "A");
	    break;

	case k_key_b: 
	    strcpy(dig_char, "B");
	    break;

	case k_key_c: 
	    strcpy(dig_char, "C");
	    break;

	case k_key_d: 
	    strcpy(dig_char, "D");
	    break;

	case k_key_e: 
	    strcpy(dig_char, "E");
	    break;

	case k_key_f: 
	    strcpy(dig_char, "F");
	    break;
    }

    if ((strcmp(Buffer(mw), Keys(mw)[0].label) == 0) &&
      (dig_num != k_key_period)) {
	BufferCount(mw) = 0;
	Buffer(mw)[0] = '\0';
    }

    strcat(Buffer(mw), dig_char);
    BufferCount(mw)++;
    display_in_window(mw, Buffer(mw), &Disp(mw));
}

/*
**++
**  ROUTINE NAME:
**	process_operator1 (mw, op_num)
**
**  FUNCTIONAL DESCRIPTION:
**  	process an operator expecting one operand.
**  	these are change sign (+-), percent (%), and
**
**  FORMAL PARAMETERS:
**  	int op_num;
**
**  IMPLICIT INPUTS:
** 	CurrentOp (mw), NumStackCount (mw)
**
**  IMPLICIT OUTPUTS:
**	NumStackCount (mw), NumStack (mw), CurrentOp (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
process_operator1(mw, op_num)
    CalcWidget mw;
    int op_num;
{
    double trig_value, minus_one = -1;
    int op;

    flush_buffer(mw);

    /* check for the case where the user is performing an operation on a
     * number and a dirivitive of the number. (e.g. 7*sqrt(7)) */

    if ((CurrentOp(mw) != EMPTY) && (NumStackCount(mw) != 2) &&
      (op_num != k_key_percent)) {
	NumStackCount(mw)++;
	NumStack(mw)[1] = NumStack(mw)[0];
    }

    op = NumStack(mw)[NumStackCount(mw) - 1];
    trig_value = NumStack(mw)[NumStackCount(mw) - 1];

    if (InvsEnabled(mw)) {
	switch (op_num) {

	    case k_key_sine: 
		op_num = k_key_asine;
		break;

	    case k_key_cosine: 
		op_num = k_key_acosine;
		break;

	    case k_key_tangent: 
		op_num = k_key_atangent;
		break;

	    case k_key_logrithm: 
		op_num = k_key_10tox;
		break;

	    case k_key_natlog: 
		op_num = k_key_exp;
		break;

	    case k_key_square_root: 
		op_num = k_key_square;
		break;
	    default: 
		break;
	}
    } else {
	if (TrigType(mw) == DEGREES)
	    trig_value = NumStack(mw)[NumStackCount(mw) - 1] / 180 * PI;

	if (TrigType(mw) == GRADIANTS)
	    trig_value = NumStack(mw)[NumStackCount(mw) - 1] / 200 * PI;
    }

    switch (op_num) {

	case k_key_sine: 
	    NumStack(mw)[NumStackCount(mw) - 1] = sin(trig_value);
	    break;

	case k_key_asine: 
	    NumStack(mw)[NumStackCount(mw) - 1] = asin(trig_value);
	    break;

	case k_key_cosine: 
	    NumStack(mw)[NumStackCount(mw) - 1] = cos(trig_value);
	    break;

	case k_key_acosine: 
	    NumStack(mw)[NumStackCount(mw) - 1] = acos(trig_value);
	    break;

	case k_key_tangent: 
	    NumStack(mw)[NumStackCount(mw) - 1] = tan(trig_value);
	    break;

	case k_key_atangent: 
	    NumStack(mw)[NumStackCount(mw) - 1] = atan(trig_value);
	    break;

	case k_key_logrithm: 
	    if (NumStack(mw)[NumStackCount(mw) - 1] <= 0) {
		error(mw);
		return;
	    } else
		NumStack(mw)[NumStackCount(mw) - 1] =
		  log10(NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_10tox: 
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      pow((double) 10, NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_natlog: 
	    if (NumStack(mw)[NumStackCount(mw) - 1] <= 0) {
		error(mw);
		return;
	    } else
		NumStack(mw)[NumStackCount(mw) - 1] =
		  log(NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_exp: 
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      exp(NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_square_root: 
	    if (NumStack(mw)[NumStackCount(mw) - 1] < 0) {
		error(mw);
		return;
	    } else
		NumStack(mw)[NumStackCount(mw) - 1] =
		  sqrt(NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_square: 
	    NumStack(mw)[NumStackCount(mw) - 1] = NumStack(mw)[
	      NumStackCount(mw) - 1] * NumStack(mw)[NumStackCount(mw) - 1];
	    break;

	case k_key_xfact: 
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      xfact(mw, NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_random: 
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      genrandom(mw, NumStack(mw)[NumStackCount(mw) - 1]);
	    break;

	case k_key_1overx: 
	    if (NumStack(mw)[NumStackCount(mw) - 1] == 0) {
		error(mw);
		return;
	    } else
		NumStack(mw)[NumStackCount(mw) - 1] =
		  1 / NumStack(mw)[NumStackCount(mw) - 1];
	    break;

	case k_key_pi: 
	    NumStack(mw)[NumStackCount(mw) - 1] = PI;
	    break;

	case k_key_plus_minus: 
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      NumStack(mw)[NumStackCount(mw) - 1] * minus_one;
	    break;

	case k_key_percent: 
	    if (NumStackCount(mw) != 2)
		NumStack(mw)[NumStackCount(mw) - 1] = 0;
	    else
		NumStack(mw)[1] = NumStack(mw)[0] * (NumStack(mw)[1] / 100);
	    break;

	case k_key_not: 
	    NumStack(mw)[NumStackCount(mw) - 1] = ~op;
	    break;

	case k_key_neg: 
	    NumStack(mw)[NumStackCount(mw) - 1] = ~op + 1;
	    break;
    }

    if ((op_num == k_key_asine) || (op_num == k_key_acosine) ||
      (op_num == k_key_atangent)) {

	if (TrigType(mw) == DEGREES)
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      NumStack(mw)[NumStackCount(mw) - 1] * 180 / PI;

	if (TrigType(mw) == GRADIANTS)
	    NumStack(mw)[NumStackCount(mw) - 1] =
	      NumStack(mw)[NumStackCount(mw) - 1] * 200 / PI;
    }

    if ((errno == EDOM) || (errno == ERANGE)) {
	NumStack(mw)[0] = HighNo(mw) + 1;
    }

    errno = 0;

    display_top_stack(mw);

    if (InvsEnabled(mw)) {
	InvsEnabled(mw) = FALSE;
	UnHighlightKey(mw, &InvFunc(mw)[k_key_inverse - INV_FUNC_BASE]);
    }
}
/*
**++
**  ROUTINE NAME:
**	process_operator2 (mw, op_num)
**
**  FUNCTIONAL DESCRIPTION:
**  	process an operator expecting two operands.
**  	these are +, -, *, and /.
**
**  FORMAL PARAMETERS:
**    	int op_num -- operator number to process
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	CurrentOp (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
process_operator2(mw, op_num)
    CalcWidget mw;
    int op_num;
{
    pop(mw);
    CurrentOp(mw) = op_num;
}

/*
**++
**  ROUTINE NAME:
**	process_equals (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	unstack the 2 operands, get the result,
**  	and update the display.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	CurrentOp (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
process_equals(mw)
    CalcWidget mw;
{
    pop(mw);
    CurrentOp(mw) = EMPTY;
}

/*
**++
**  ROUTINE NAME:
**	display_clear (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	clear the display window, and
**  	reset all the global variables.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	NumStack (mw), NumStackCount (mw), buffer, BufferCount (mw),
**	strmax, CurrentOp (mw), error_flag
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
display_clear(mw)
    CalcWidget mw;
{
    NumStackCount(mw) = 1;
    NumStack(mw)[0] = 0;
    BufferCount(mw) = 0;
    Buffer(mw)[0] = '\0';

    if (Mode(mw) == k_decimal)
	Strmax(mw) = MAX_BUFF;
    else if (Mode(mw) == k_hexadecimal)
	Strmax(mw) = MAX_HEX_BUFF;
    else
	Strmax(mw) = MAX_OCT_BUFF;

    CurrentOp(mw) = EMPTY;
    display_in_window(mw, ZeroPoint(mw), &Disp(mw));
    ErrorFlag(mw) = FALSE;
}

/*
**++
**  ROUTINE NAME:
**	mem_clear (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	clear the memory, and remove the
**  	memory indicator if it is up.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	MemValue (mw) , MemErrorFlag (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mem_clear(mw)
    CalcWidget mw;
{
    MemValue(mw) = 0;
    MemErrorFlag(mw) = FALSE;
    display_in_window(mw, ZeroPoint(mw), &Memory(mw));
}

/*
**++
**  ROUTINE NAME:
**	mem_recall (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	fetch the current memory, display it,
**  	and put it on top of the stack. (The
**  	buffer is also cleared.)
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
** 	MemErrorFlag (mw), CurrentOp (mw), MemValue (mw)
**
**  IMPLICIT OUTPUTS:
**	NumStackCount (mw), NumStack (mw), BufferCount (mw), buffer
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mem_recall(mw)
    CalcWidget mw;
{
    if (MemErrorFlag(mw) == TRUE)
	error(mw);
    else {
	if (CurrentOp(mw) == EMPTY)
	    NumStackCount(mw) = 0;

	BufferCount(mw) = 0;
	Buffer(mw)[0] = '\0';
	NumStack(mw)[NumStackCount(mw)] = MemValue(mw);
	NumStackCount(mw) = NumStackCount(mw) + 1;

	display_top_stack(mw);
    }
}

/*
**++
**  ROUTINE NAME:
**	mem_plus (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	add the top of the stack to the memory
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
** 	MemErrorFlag (mw), MemValue (mw) , NumStack (mw)
**
**  IMPLICIT OUTPUTS:
**	MemValue (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mem_plus(mw)
    CalcWidget mw;
{
    if (MemErrorFlag(mw) == FALSE) {

	flush_buffer(mw);

	MemValue(mw) = MemValue(mw) + NumStack(mw)[NumStackCount(mw) - 1];
	if (MemValue(mw) > HighNo(mw) || MemValue(mw) < LowNo(mw)) {
	    mem_error(mw);
	    return;
	}
	mem_value_dis(mw);
    }
}

/*
**++
**  ROUTINE NAME:
**	mem_minus (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	subtract the top of the stack from memory
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
** 	MemErrorFlag (mw), MemValue (mw) , NumStack (mw)
**
**  IMPLICIT OUTPUTS:
**	MemValue (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mem_minus(mw)
    CalcWidget mw;
{
    if (MemErrorFlag(mw) == FALSE) {

	flush_buffer(mw);

	MemValue(mw) = MemValue(mw) - NumStack(mw)[NumStackCount(mw) - 1];
	if (MemValue(mw) > HighNo(mw) || MemValue(mw) < LowNo(mw)) {
	    mem_error(mw);
	    return;
	}
	mem_value_dis(mw);
    }
}

/*
**++
**  ROUTINE NAME:
**	clear_entry (mw)
**
**  FUNCTIONAL DESCRIPTION:
**      clear the current buffer
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	BufferCount (mw), buffer
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
clear_entry(mw)
    CalcWidget mw;
{
    if (BufferCount(mw) > 0)
	BufferCount(mw) = 1;
    strcpy(Buffer(mw), Keys(mw)[0].label);
    if (!ErrorFlag(mw))
	display_in_window(mw, Buffer(mw), &Disp(mw));
}

/*
**++
**  ROUTINE NAME:
**	backspace_key (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	user typed a backspace.  If the number has
**  	not already been entered, erase the last char.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
** 	BufferCount (mw), buffer
**
**  IMPLICIT OUTPUTS:
**	BufferCount (mw), buffer, strmax
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
backspace_key(mw)
    CalcWidget mw;
{
    if (BufferCount(mw) != 0)		/* number is still active */
      {

	/* if erasing a decimal point, reset the string maximum length */
	if (Buffer(mw)[BufferCount(mw) - 1] == ChrPoint(mw))
	    Strmax(mw) = MAX_BUFF;

	BufferCount(mw)--;
	Buffer(mw)[BufferCount(mw)] = '\0';

	/* if the buffer is now empty, display a zero */
	if (BufferCount(mw) == 0) {
	    BufferCount(mw) = 1;
	    strcpy(Buffer(mw), Keys(mw)[0].label);
	}
	display_in_window(mw, Buffer(mw), &Disp(mw));
    } else

	/* no active number, beeeeeeeeep */
	XBell(Dpy(mw), 0);
}

/*
**++
**  ROUTINE NAME:
**	change_degrees (mw)
**
**  FUNCTIONAL DESCRIPTION:
**      Change the degrees
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Changes the trig computations and the Degree Label
**--
**/
change_degrees(mw)
    CalcWidget mw;
{
    int rc = 0;
    XRectangle rects[10];

    if (TrigType(mw) == GRADIANTS) {
	calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_DEGREE",
	  Func(mw)[0].label);
	TrigType(mw) = DEGREES;
    } else if (TrigType(mw) == DEGREES) {
	calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_RADIANS",
	  Func(mw)[0].label);
	TrigType(mw) = RADIANS;
    } else {
	calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_GRADIANTS",
	  Func(mw)[0].label);
	TrigType(mw) = GRADIANTS;
    }

    DrawKey(mw, &Func(mw)[0], &rects[rc], &rc, FALSE);

    XFillRectangles(Dpy(mw), XtWindow(mw), CalcClearGC(mw), rects, rc);

    DrawKey(mw, &Func(mw)[0], &rects[rc], &rc, TRUE);
}

/*
**++
**  ROUTINE NAME:
**	switch_invserse (mw)
**
**  FUNCTIONAL DESCRIPTION:
**      Change the degrees
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Changes the trig computations Labels
**--
**/
switch_invserse(mw)
    CalcWidget mw;
{
    InvsEnabled(mw) = 1 - InvsEnabled(mw);
}

/*
**++
**  ROUTINE NAME:
**      display_in_window (mw, new_display, wid)
**
**  FUNCTIONAL DESCRIPTION:
**      Display the passed in string in
**      the passed-in-window.
**
**  FORMAL PARAMETERS:
**	new_display : new string to display
**      win	    : id of window to put string in
** 	win_width   : width of window to display string in
**
**  IMPLICIT INPUTS:
** 	curfontinfo
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void display_in_window(mw, new_value, dw)
    CalcWidget mw;
    char *new_value;
    keyinfo *dw;
{
    XmOffsetPtr	    o = CalcOffsetPtr(mw);
    int reverse, len;
    int direction = NULL;	/* Return direction value */
    int ascent = NULL;		/* Return ascent value */
    int descent = NULL;		/* Return descent value */
    Dimension x, y, wid, hyt;
    GC draw, clear;
    XCharStruct overall;	/* Return extent values */

    reverse = FALSE;
    if (dw == &Disp(mw)) {
	strcpy(BufferCopy(mw), new_value);
	if (DisplaySelected(mw))
	    reverse = TRUE;
    } else if (!DisplaySelected(mw))
	reverse = TRUE;

    if (reverse) {
	draw = CalcClearGC(mw);
	clear = CalcGC(mw);
    } else {
	draw = CalcGC(mw);
	clear = CalcClearGC(mw);
    }

    if (reverse) {
	x = dw->x;
	y = dw->y;
	wid = dw->width;
	hyt = dw->height;
    } else {
	x = dw->x + 1;
	y = dw->y + 1;
	wid = dw->width - 1;
	hyt = dw->height - 1;
    }

    len = strlen(new_value);

    XTextExtents
	(CFont(mw), new_value, len, &direction, &ascent, &descent, &overall);

    XFillRectangle(Dpy(mw), XtWindow(mw), clear, x, y, wid, hyt);

    _XmDrawShadow
    (
	XtDisplay(mw),
	XtWindow(mw),
	dw->inv ? CalcBottomShadowGC(mw,o) : CalcTopShadowGC(mw,o),
	dw->inv ? CalcTopShadowGC(mw,o) : CalcBottomShadowGC(mw,o),
	CalcShadowThickness(mw,o),
	x, y, wid, hyt
    );

    x = dw->x + dw->width - 4 - overall.width;
    y = dw->y + (dw->height - ((dw->height - DisplayFontHeight(mw)) / 2)) -
      CFont(mw)->max_bounds.descent;
    XDrawString(Dpy(mw), XtWindow(mw), draw, x, y, new_value, len);
}

/*
**++
**  ROUTINE NAME:
**	flush_buffer (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	if the buffer has not been entered, put it on
**  	top of the stack, and clear the buffer.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
** 	BufferCount (mw), buffer, NumStack (mw), NumStackCount (mw)
**
**  IMPLICIT OUTPUTS:
** 	BufferCount (mw), buffer, NumStack (mw), NumStackCount (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
flush_buffer(mw)
    CalcWidget mw;
{
    char temp_buffer[MAX_BUFF + 3], *dot_loc;
    double value;
    int comp_value;

    if (BufferCount(mw) != 0) {

	/* if there is not already a dot in the number, append one to the end
	 */
	dot_loc = strchr(Buffer(mw), ChrPoint(mw));
	if (dot_loc == NULL) {
	    strcat(Buffer(mw), Keys(mw)[1].label);
	    display_in_window(mw, Buffer(mw), &Disp(mw));
	}

	/* Replace the user specifed radix poiunt with a '.' so sscanf will
	 * convert the string automatically. dot_loc is the address of the
	 * radix point */
	strcpy(temp_buffer, Buffer(mw));
	dot_loc = strchr(temp_buffer, ChrPoint(mw));
	if (dot_loc != NULL)
	    *dot_loc = '.';
	if (Mode(mw) == k_decimal)
	    sscanf(temp_buffer, InputFormatStr(mw), &value);
	else
	    sscanf(temp_buffer, InputFormatStr(mw), &comp_value);

	/* if there are already 2 items on the stack, replace the top element.
	 */

	if (NumStackCount(mw) == 2)
	    NumStackCount(mw) = 1;

	if (Mode(mw) == k_decimal)
	    NumStack(mw)[NumStackCount(mw)] = value;
	else
	    NumStack(mw)[NumStackCount(mw)] = comp_value;

	NumStackCount(mw)++;
	BufferCount(mw) = 0;
	Buffer(mw)[0] = '\0';
    }
}

/*
**++
**  ROUTINE NAME:
**      display_top_stack (mw)
**
**  FUNCTIONAL DESCRIPTION:
**      display the top element on the stack.  If the
**      number has too many digits, the displayed value
**      rounds off the final displayed digit-- but the
**      internal value does not change.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**      NumStack (mw)
**
**  IMPLICIT OUTPUTS:
**	strmax
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
display_top_stack(mw)
    CalcWidget mw;
{
    char new_disp[INT_MAX_STR];
    double str_max_len;

    /* if the number is too big or small, signal error */
    if ((NumStack(mw)[NumStackCount(mw) - 1] < LowNo(mw)) ||
      (NumStack(mw)[NumStackCount(mw) - 1] > HighNo(mw))) {
	error(mw);
	return;
    } else {
	getnumstring(mw, NumStack(mw)[NumStackCount(mw) - 1]);
	strcpy(new_disp, Numstr(mw));
	display_in_window(mw, new_disp, &Disp(mw));
	if (Mode(mw) == k_decimal)
	    Strmax(mw) = MAX_BUFF;
	else if (Mode(mw) == k_hexadecimal)
	    Strmax(mw) = MAX_HEX_BUFF;
	else
	    Strmax(mw) = MAX_OCT_BUFF;
    }
}

/*
**++
**  ROUTINE NAME:
**	pop (mw)
**
**  FUNCTIONAL DESCRIPTION:
**  	flush the buffer and execute the current
**  	operator on the stack iff the stack has
**  	at least one element and there is a current
**  	operator.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**	NumStackCount (mw), NumStack (mw), CurrentOp (mw)
**
**  IMPLICIT OUTPUTS:
**	NumStack (mw), NumStackCount (mw)
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
pop(mw)
    CalcWidget mw;
{
    int op1, op2;

    flush_buffer(mw);

    if ((CurrentOp(mw) == EMPTY) || (NumStackCount(mw) == 0))
	return;
    else {

	/* if there is only one element on the stack, use it as both operands
	 */
	if (NumStackCount(mw) == 1) {
	    NumStack(mw)[1] = NumStack(mw)[0];
	    NumStackCount(mw)++;
	}

	op1 = (int) NumStack(mw)[0];
	op2 = (int) NumStack(mw)[1];

	if (InvsEnabled(mw) && (CurrentOp(mw) == k_key_ytothex))
	    CurrentOp(mw) = k_key_ytominusx;

	switch (CurrentOp(mw)) {
	    case k_key_ytothex: 
		NumStack(mw)[0] = pow(NumStack(mw)[0], NumStack(mw)[1]);
		break;

	    case k_key_ytominusx: 
		if (NumStack(mw)[1] == 0) {
		    NumStack(mw)[0] = HighNo(mw) + 1;
		} else
		    NumStack(mw)[0] =
		      pow(NumStack(mw)[0], 1 / NumStack(mw)[1]);
		break;

	    case k_key_plus: 
		NumStack(mw)[0] = NumStack(mw)[0] + NumStack(mw)[1];
		break;

	    case k_key_minus: 
		NumStack(mw)[0] = NumStack(mw)[0] - NumStack(mw)[1];
		break;

	    case k_key_times: 
		NumStack(mw)[0] = NumStack(mw)[0] * NumStack(mw)[1];
		break;

	    case k_key_divide: 
		if (NumStack(mw)[1] == 0) {
		    error(mw);
		    return;
		} else
		    NumStack(mw)[0] = NumStack(mw)[0] / NumStack(mw)[1];
		break;

	    case k_key_and: 
		NumStack(mw)[0] = op1 & op2;
		break;

	    case k_key_or: 
		NumStack(mw)[0] = op1 | op2;
		break;

	    case k_key_xor: 
		NumStack(mw)[0] = op1 ^ op2;
		break;

	    case k_key_nor: 
		NumStack(mw)[0] = !(op1 | op2);
		break;
	}

	if ((errno == EDOM) || (errno == ERANGE)) {
	    NumStack(mw)[0] = HighNo(mw) + 1;
	}
	errno = 0;
	NumStackCount(mw) = 1;
	display_top_stack(mw);
    }

    if (InvsEnabled(mw)) {
	InvsEnabled(mw) = FALSE;
	UnHighlightKey(mw, &InvFunc(mw)[k_key_inverse - INV_FUNC_BASE]);
    }
}

/*
**++
**  ROUTINE NAME:
**      getnumstring (mw, num)
**
**  FUNCTIONAL DESCRIPTION:
**      return the string representation of the passed in number : num
**
**  FORMAL PARAMETERS:
**	num 	: the double float whose string representation is returned
**
**  IMPLICIT INPUTS:
**      Strmax (mw)  : longest allowed string
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	char
**
**  SIDE EFFECTS:
**--
**/
void getnumstring(mw, num)
    CalcWidget mw;
    double num;
{
    char *dot_locc;
    double adder, round_off, dot_locl, disp_num;
    int i, dot_loc, str_max_len, oldstrmax, comp_num, num_length;
    unsigned long mask;
    XmString *CSnum_str;

    oldstrmax = Strmax(mw);

    if (Mode(mw) == k_decimal)
	Strmax(mw) = MAX_BUFF + 1;

    if (Mode(mw) == k_decimal) {
#ifdef TEST_XNLS
	mask = 0;
	mask |= XNL_SIGNIFNEG;

	num_length = xnl_strfnum(LanguageId(mw), &CSnum_str,
	  CSNumberFormat(mw), mask, num);

	if (num_length != 0)
	    DumpCS(&CSnum_str);
	else
	    printf("Unable to process input %.15lf \n", num);

	DumpCS(&CSnum_str);

	if (num_length != 0) {
	    ParseCS(Numstr(mw), CSnum_str);
	}
#else
	sprintf(Numstr(mw), OutputFormatStr(mw), num);
#endif
    } else {
	comp_num = (int) num;
	sprintf(Numstr(mw), OutputFormatStr(mw), comp_num);
    }

    /* strip trailing 0's */
    if (Mode(mw) == k_decimal) {
	while (Numstr(mw)[strlen(Numstr(mw)) - 1] == ChrZero(mw))
	    Numstr(mw)[strlen(Numstr(mw)) - 1] = '\0';

	strip_leading_characters(Numstr(mw), ' ');

	/* if the number is negative, allow an extra space for the "-" */
	if (num < 0)
	    str_max_len = Strmax(mw) + 1;
	else
	    str_max_len = Strmax(mw);

	/* if the display string is too long, only display MAX_BUFF characters 
	 * with the last digit rounded off. */

	if (strlen(Numstr(mw)) > str_max_len) {

	    /* find the decimal point in the string */
	    i = -1;
	    dot_loc = -1;
	    while ((dot_loc == -1) && (i <= str_max_len)) {
		i = i + 1;
		if (Numstr(mw)[i] == '.')
		    dot_loc = i;
	    }				/* while */

	    dot_locl = (double) dot_loc;
	    if (dot_locl < str_max_len) {
		round_off = str_max_len - dot_locl;
		adder = 5 / pow(10.0, round_off);

		if (NumStack(mw)[NumStackCount(mw) - 1] < 0)
		    disp_num = num - adder;
		else
		    disp_num = num + adder;

		if (Mode(mw) == k_decimal)
		    sprintf(Numstr(mw), OutputFormatStr(mw), disp_num);
		else {
		    comp_num = (int) num;
		    sprintf(Numstr(mw), OutputFormatStr(mw), comp_num);
		}			/* else */
	    }				/* if */

	      /* else the decimal point is on the end, no rounding is needed */
	      else
		disp_num = num;

	    if (disp_num < 0)
		Numstr(mw)[Strmax(mw) + 1] = '\0';
	    else
		Numstr(mw)[Strmax(mw)] = '\0';

	    while (Numstr(mw)[strlen(Numstr(mw)) - 1] == ChrZero(mw))
		Numstr(mw)[strlen(Numstr(mw)) - 1] = '\0';
	}				/* if */
    }					/* if */

    /* Change "." to the same label as the "." key */
    dot_locc = strchr(Numstr(mw), '.');
    if (dot_locc != NULL)
	*dot_locc = ChrPoint(mw);

    if (strcmp(Numstr(mw), "") == 0)
	strcpy(Numstr(mw), "0");

    Strmax(mw) = oldstrmax;
}

/*
**++
**  ROUTINE NAME:
**	strip_leading_characters (str_in, pad_char)
**
**  FUNCTIONAL DESCRIPTION:
**  	remove all characters of type pad_char from the start of str_in
**
**  FORMAL PARAMETERS:
**	char *str_in - the string to strip of a padding chars
**	char pad_char - the a pad character
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
strip_leading_characters (str_in, pad_char)
    char *str_in;
    char pad_char;
{
    char cur_char;
    int i, old_len;

    while ((str_in[0] == pad_char) && (strlen(str_in) > 1)) {
	old_len = strlen(str_in);
	for (i = 0; i < old_len; i++)
	    str_in[i] = str_in[i + 1];
    }
}

/*
**++
**  ROUTINE NAME:
**	mem_value_dis (mw)
**
**  FUNCTIONAL DESCRIPTION:
** 	display the current memory value in the memory window.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**      mem_value, memwinid
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void mem_value_dis(mw)
    CalcWidget mw;
{
    char mem_str[INT_MAX_STR];

    if (MemValue(mw) > HighNo(mw) || MemValue(mw) < LowNo(mw))
	mem_error(mw);
    else {
	getnumstring(mw, MemValue(mw));
	strcpy(mem_str, Numstr(mw));
	display_in_window(mw, mem_str, &Memory(mw));
    }
}

/*
**++
**  ROUTINE NAME:
**	calc_FindMainWidget (w)
**
**  FUNCTIONAL DESCRIPTION:
** 	Returns the id of the calculator widget
**
**  FORMAL PARAMETERS:
**	w - the child
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	CalcWidget - the resulting calc widget id
**
**  SIDE EFFECTS:
**--
**/
CalcWidget calc_FindMainWidget(w)
    Widget w;
{
    Widget p;

    p = w;
    while (!XtIsSubclass(p, (WidgetClass) calcwidgetclass))
    {
	Widget temp;
	temp = XtParent(p);
	if (temp == NULL)
	{
	    int i;
	    WidgetList foo;

	    foo = DXmChildren (p);
	    for (i = 0; i < DXmNumChildren(p); i++)
	    {
		if (XtIsSubclass(foo[i], (WidgetClass) calcwidgetclass))
		{
		    p = (Widget)foo[i];
		    return ((CalcWidget)p);
		}
	    }
	    return (NULL);
	}
	p = temp;
    }
    return ((CalcWidget) p);
}

/*
**++
**  ROUTINE NAME:
**	FindWidgetId (key)
**
**  FUNCTIONAL DESCRIPTION:
** 	Determines the widget Id based on index
**
**  FORMAL PARAMETERS:
**	key - the key index
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	w - the key widget id
**
**  SIDE EFFECTS:
**--
**/
keyinfo *FindWidgetId(mw, key)
    CalcWidget mw;
    int key;
{
    keyinfo *keyid;

    if ((key >= MEM_BASE) && (key < (MEM_BASE + NUMMEMS)))
	keyid = &Mems(mw)[key - MEM_BASE];

    if ((key >= DIGIT_BASE) && (key < (DIGIT_BASE + NUMKEYS)))
	keyid = &Keys(mw)[key - DIGIT_BASE];

    if ((key >= OPS_BASE) && (key < (OPS_BASE + NUMOPS)))
	keyid = &Ops(mw)[key - OPS_BASE];

    if ((key >= CLEAR_BASE) && (key < (CLEAR_BASE + NUMCLEARS)))
	keyid = &Clears(mw)[key - CLEAR_BASE];

    if ((key >= FUNC_BASE) && (key < (FUNC_BASE + NUMFUNC)))
	keyid = &Func(mw)[key - FUNC_BASE];

    if ((key >= INV_FUNC_BASE) && (key < (INV_FUNC_BASE + NUMINVFUNC)))
	keyid = &InvFunc(mw)[key - INV_FUNC_BASE];

    return (keyid);
}

/*
**++
**  ROUTINE NAME:
**	error 
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void error(mw)
    CalcWidget mw;
{
    ErrorFlag(mw) = TRUE;
    display_in_window(mw, ErrorString(mw), &Disp(mw));
}

/*
**++
**  ROUTINE NAME:
**	mem_error
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void mem_error(mw)
    CalcWidget mw;
{
    MemErrorFlag(mw) = TRUE;
    display_in_window(mw, ErrorString(mw), &Memory(mw));
}

#ifdef DWTVMS
#define ReadDesc(source_desc,addr,len)	\
    LIB$ANALYZE_SDESC(source_desc,&len,&addr)
#endif

/*
**++
**  ROUTINE NAME:
**	calc_ChangeWindowGeometry
**
**  FUNCTIONAL DESCRIPTION:
**	change the subwidget's window geometry
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void calc_ChangeWindowGeometry(w, size, margin)
    Widget w;
    XtWidgetGeometry *size;
    Dimension margin;
{
    int ac = 0;
    Arg al[2];

    /* split the changes into the two modes the intrinsic's understand. */

    if (margin > 0) {
	margin = Half(margin);
	XtSetArg(al[0], XmNmarginTop, margin);
	XtSetValues(w, al, 1);
    }

    if (size->request_mode & (CWX | CWY)) {
	Position newx, newy;

	if (size->request_mode & CWX)
	    newx = size->x;
	else
	    newx = w->core.x;
	if (size->request_mode & CWY)
	    newy = size->y;
	else
	    newy = w->core.y;
	XtMoveWidget(w, newx, newy);
    }

    if (size->request_mode & (CWWidth | CWHeight | CWBorderWidth)) {
	if (size->request_mode & CWWidth)
	    w->core.width = size->width;
	if (size->request_mode & CWHeight)
	    w->core.height = size->height;
	if (size->request_mode & CWBorderWidth)
	    w->core.border_width = size->border_width;
	XtResizeWindow(w);
    }
}

/*
**++
**  ROUTINE NAME:
**	LayoutKey 
**
**  FUNCTIONAL DESCRIPTION:
**	Change the key's layout information
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void LayoutKey(key, size, margin, inv)
    keyinfo *key;
    XtWidgetGeometry *size;
    Dimension margin;
    Boolean inv;
{

    /* split the changes into the two modes the intrinsic's understand. */

    if (margin > 0)
	margin = Half(margin);
    else
	margin = 0;

    key->margin = margin;
    key->x = size->x;
    key->y = size->y;
    key->width = size->width;
    key->height = size->height;
    key->inv = inv;
}

/*
**++
**  ROUTINE NAME:
**	CalcLoadFontFamilies (mw)
**
**  FUNCTIONAL DESCRIPTION:
** 	Loads all of the fonts based on user request
**
**  FORMAL PARAMETERS:
**	mw - the clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CalcLoadFontFamilies(mw)
    CalcWidget mw;
{
    char **fontnames, pattern[100], sqrt_char[2];
    int i, j, temp, len, font_height, num_found, KFontHeights[10],
      OrderedArray[10], maxnames = 10;
    int direction = NULL;	/* Return direction value */
    int ascent = NULL;		/* Return ascent value */
    int descent = NULL;		/* Return descent value */
    XCharStruct overall;	/* Return extent values */
    XFontStruct *KFonts[10];

    strcpy(pattern, mw->calc. key_font_family);
    fontnames = XListFonts(Dpy(mw), pattern, maxnames, &num_found);

    for (i = 0; i < num_found; i++) {
	KFonts[i] = XLoadQueryFont(Dpy(mw), fontnames[i]);
	KFontHeights[i] =
	  (KFonts[i]->max_bounds.ascent + KFonts[i]->max_bounds.descent);
	OrderedArray[i] = i;
    }

    for (i = 0; i < num_found; i++)
	for (j = 0; j < num_found - 1; j++)
	    if (KFontHeights[OrderedArray[j]] >
	      KFontHeights[OrderedArray[j + 1]]) {
		temp = OrderedArray[j];
		OrderedArray[j] = OrderedArray[j + 1];
		OrderedArray[j + 1] = temp;
	    }

    NumFonts(mw) = num_found;
    for (i = 0; i < num_found; i++) {
	CFonts(mw)[i] = KFonts[OrderedArray[i]];
	FontHeights(mw)[i] = KFontHeights[OrderedArray[i]];
	XTextExtents(CFonts(mw)[i], "MR", 2, &direction, &ascent, &descent,
	  &overall);
	FontWidths(mw)[i] = overall.width;
    }

    XFreeFontNames(fontnames);
}

/*
**++
**  ROUTINE NAME:
**	CalcLoadSqrtFontFamilies (mw)
**
**  FUNCTIONAL DESCRIPTION:
** 	Loads all of the fonts based on user request
**
**  FORMAL PARAMETERS:
**	mw - the clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void CalcLoadSqrtFontFamilies(mw)
    CalcWidget mw;
{
    char **fontnames, pattern[100], sqrt_char[2];
    int i, j, temp, len, font_height, num_found, KFontHeights[10],
      OrderedArray[10], maxnames = 10;
    int direction = NULL;	/* Return direction value */
    int ascent = NULL;		/* Return ascent value */
    int descent = NULL;		/* Return descent value */
    XCharStruct overall;	/* Return extent values */
    XFontStruct *KFonts[10];

    strcpy(pattern, mw->calc. sqrt_font_family);
    fontnames = XListFonts(Dpy(mw), pattern, maxnames, &num_found);

    for (i = 0; i < num_found; i++) {
	KFonts[i] = XLoadQueryFont(Dpy(mw), fontnames[i]);
	KFontHeights[i] =
	  (KFonts[i]->max_bounds.ascent + KFonts[i]->max_bounds.descent);
	OrderedArray[i] = i;
    }

    for (i = 0; i < num_found; i++)
	for (j = 0; j < num_found - 1; j++)
	    if (KFontHeights[OrderedArray[j]] >
	      KFontHeights[OrderedArray[j + 1]]) {
		temp = OrderedArray[j];
		OrderedArray[j] = OrderedArray[j + 1];
		OrderedArray[j + 1] = temp;
	    }

    sqrt_char[0] = 86;
    sqrt_char[1] = 0;

    NumSqrtFonts(mw) = num_found;
    for (i = 0; i < num_found; i++) {
	SqrtFonts(mw)[i] = KFonts[OrderedArray[i]];
	SqrtFontHeights(mw)[i] = KFontHeights[OrderedArray[i]];
	XTextExtents(SqrtFonts(mw)[i], sqrt_char, 1, &direction, &ascent,
	  &descent, &overall);
	SqrtFontWidths(mw)[i] = overall.width;
    }

    XFreeFontNames(fontnames);
}

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME:
**	DescToNull
**
**  FUNCTIONAL DESCRIPTION:
**	This routine converts a string descriptor to a null
**	terminated string
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
char *DescToNull(desc)
    struct dsc$descriptor_s *desc;
{
    char *nullterm_string, *temp_string;
    unsigned short temp_length;

    ReadDesc(desc, temp_string, temp_length);

    if (temp_length == 0)
	nullterm_string = NULL;
    else {
	nullterm_string = (char *) XtMalloc(temp_length + 1);

	/* bcopy(temp_string,nullterm_string,temp_length); */

	/* Make it null-terminated */
	*(nullterm_string + temp_length) = '\0';
    }
    return (nullterm_string);
}
#endif

/*
**++
**  ROUTINE NAME:
**	ParseCS
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void ParseCS(key_label, uil_string)
    char *key_label;
    XmString *uil_string;
{
    int dont_care;
    char *tmp;
    Cardinal status;
    XmStringCharSet char_set;
    XmStringContext CScontext;

    status = XmStringInitContext(&CScontext, uil_string);
    status = XmStringGetNextSegment(CScontext, &tmp, &char_set, &dont_care,
      &dont_care);
    strcpy(key_label, tmp);
    XtFree(tmp);
}

/*
**++
**  ROUTINE NAME:
**	DumpCS
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
DumpCS(p)
    XmString *p;
{
    char *text_return;
    int iok, dirrtol_return, rend_return;
    unsigned long charset_return, lang_return;
    XmStringContext context;

    {
	printf("<<<");
	iok = XmStringInitContext(&context, *p);
	while (iok == TRUE) {
	    if ((iok = XmStringGetNextSegment(context, &text_return,
	      &charset_return, &dirrtol_return, &rend_return)) == TRUE) {
		printf("(%d) [%s]", charset_return, text_return);
	    }
	}
	printf(">>>\n");
    }
}
