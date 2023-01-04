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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: istring.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/19 08:45:07 $";
#endif
/*
 * %W%  %G%  (ULTRIX "sccs" id data)
 *
 * COPYRIGHT (c) DIGITAL EQUIPMENT CORPORATION 1991.
 * ALL RIGHTS RESERVED.
 * 
 * THIS  SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE
 * USED  AND  COPIED ONLY IN ACCORDANCE WITH THE TERMS OF
 * SUCH  LICENSE  AND  WITH  THE  INCLUSION  OF THE ABOVE
 * COPYRIGHT  NOTICE.   THIS SOFTWARE OR ANY OTHER COPIES
 * THEREOF   MAY   NOT  BE  PROVIDED  OR  OTHERWISE  MADE
 * AVAILABLE  TO  ANY  OTHER  PERSON.   NO  TITLE  TO AND
 * OWNERSHIP OF THE SOFTWARE IS HEREBY TRANSFERRED.
 * 
 * THE  INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE
 * WITHOUT  NOTICE  AND  SHOULD  NOT  BE  CONSTRUED  AS A
 * COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * 
 * DIGITAL  ASSUMES  NO  RESPONSIBILITY  FOR  THE  USE OR
 * RELIABILITY  OF  ITS SOFTWARE ON EQUIPMENT THAT IS NOT
 * SUPPLIED BY DIGITAL.
 */

/*
 * MODULE DESCRIPTION:
 *
 *      Functions to help with strings containing characters from the
 *      ASCII international character set.
 *
 *
 * HISTORY:
 *
 *      21-May-1991     Initial version.
 */

/*
 * Macro definitions
 */

#define UCHR(cval) (((int)(cval)) & 0xFF)

#define TRUE  1
#define FALSE 0

/*
 * Tables defining the international character ASCII codes.
 */

static char international_upper[30] =
    {
        '\300', '\301', '\302', '\303', '\304', '\305',     /* À Á Â Ã Ä Å  */
        '\306',                                             /* Æ            */
        '\307',                                             /* Ç            */
        '\310', '\311', '\312', '\313',                     /* È É Ê Ë      */
        '\314', '\315', '\316', '\317',                     /* Ì Í Î Ï      */
        '\0',                                               /*              */
        '\321',                                             /* Ñ            */
        '\322', '\323', '\324', '\325', '\326',             /* Ò Ó Ô Õ Ö    */
        '\327',                                             /* ×            */
        '\330',                                             /* Ø            */
        '\331', '\332', '\333', '\334',                     /* Ù Ú Û Ü      */
        '\335'                                              /* Ý            */
    };


static char international_lower[30] =
    {
        '\340', '\341', '\342', '\343', '\344', '\345',     /* à á â ã ä å  */
        '\346',                                             /* æ            */
        '\347',                                             /* ç            */
        '\350', '\351', '\352', '\353',                     /* è é ê ë      */
        '\354', '\355', '\356', '\357',                     /* ì í î ï      */
        '\0',                                               /*              */
        '\361',                                             /* ñ            */
        '\362', '\363', '\364', '\365', '\366',             /* ò ó ô õ ö    */
        '\367',                                             /* ÷            */
        '\370',                                             /* ø            */
        '\371', '\372', '\373', '\374',                     /* ù ú û ü      */
        '\375'                                              /* ý            */
    };

/*
 * istr_isupper --- Global routine
 * istr_islower --- Global routine
 *
 * istr_isalpha --- Global routine
 * istr_isdigit --- Global routine
 * istr_isalnum --- Global routine
 *
 * istr_ispunct --- Global routine
 * istr_isspace --- Global routine
 *
 * istr_isprint --- Global routine
 *
 * istr_iscntrl --- Global routine (same as istr_isnprnt)
 * istr_isnprnt --- Global routine (same as istr_iscntrl)
 *
 * Test a char value for alphabetic (upper, lower, or either case), 
 * numeric, alphanumeric, punctuation, white space, printable 
 * character, or non-printable (control) character.
 *
 * Inputs:
 *      value - char - Value to test
 *
 * Outputs:
 *      TRUE (1) or FALSE (0) depending on test results
 *
 * Notes:
 *      This handles the standard ASCII international character set.
 */

int istr_isupper(value)
register char value;
{
    register unsigned int input;
    input = UCHR(value);
    if ( (input >= UCHR('A'))    && (input <= UCHR('Z'))    ) {return(TRUE);}
    if ( (input >= UCHR('\300')) && (input <= UCHR('\335')) ) {return(TRUE);}
    return(FALSE);
}



int istr_islower(value)
register char value;
{
    register unsigned int input;
    input = UCHR(value);
    if ( (input >= UCHR('a'))    && (input <= UCHR('z'))    ) {return(TRUE);}
    if ( (input >= UCHR('\340')) && (input <= UCHR('\375')) ) {return(TRUE);}
    return(FALSE);
}



int istr_isalpha(value)
register char value;
{
    if (istr_isupper(value) || istr_islower(value)) {return(TRUE);}
    return(FALSE);
}


int istr_isdigit(value)
register char value;
{
    register unsigned int input;
    input = UCHR(value);
    if ( (input >= UCHR('0')) && (input <= UCHR('9')) ) {return(TRUE);}
    return(FALSE);
}


int istr_isalnum(value)
register char value;
{
    if (istr_isalpha(value) || istr_isdigit(value)) {return(TRUE);}
    return(FALSE);
}



int istr_ispunct(value)
register char value;
{
    register unsigned int input;
    input = UCHR(value);
    if ( 
        ( (input >  UCHR(' '))    && (input < UCHR('0'))    ) ||
        ( (input >  UCHR('9'))    && (input < UCHR('A'))    ) ||
        ( (input >  UCHR('Z'))    && (input < UCHR('a'))    ) ||
        ( (input >  UCHR('z'))    && (input < UCHR('\177')) ) ||
        ( (input >  UCHR('\240')) && (input < UCHR('\244')) ) ||
        (  input == UCHR('\245'))                             ||
        ( (input >  UCHR('\246')) && (input < UCHR('\254')) ) ||
        ( (input >  UCHR('\257')) && (input < UCHR('\264')) ) ||
        ( (input >  UCHR('\264')) && (input < UCHR('\270')) ) ||
        ( (input >  UCHR('\270')) && (input < UCHR('\276')) ) ||
        (  input == UCHR('\277'))                             ||
        (  input == UCHR('\377'))
       )                                                        {return(TRUE);}
    return(FALSE);
}



int istr_isspace(value)
register char value;
{
    register unsigned int input;
    input = UCHR(value);
    if (
        (input == UCHR(' '))  || (input == UCHR('\t')) ||
        (input == UCHR('\v')) || (input == UCHR('\r')) || 
        (input == UCHR('\f')) || (input == UCHR('\n'))
       )                                                    {return(TRUE);}
    return(FALSE);
}



int istr_isprint(value)
register char value;
{
    if ( istr_isalnum(value) || istr_ispunct(value)  || istr_isspace(value) ) {return(TRUE);}
    return(FALSE);
}



int istr_isnprnt(value)
register char value;
{
    return(!(istr_isprint(value)));
}



int istr_iscntrl(value)
register char value;
{
    return(!(istr_isprint(value)));
}

/*
 * istr_toupper --- Global routine
 * istr_tolower --- Global routine
 *
 * Convert a char value from upper (or lower) case to lower (or upper) case.
 *
 * Inputs:
 *      value - char - Value to convert
 *
 * Outputs:
 *      Converted character, as an integer (with no sign extension).
 *
 * Notes:
 *      This handles the standard ASCII international character set.
 */

int istr_toupper(value)
register char value;
{
    register unsigned int input;
    register unsigned int output;

    input = UCHR(value);
    output = input;

    if ( (input >= UCHR('a')) && (input <= UCHR('z')) )
    {
        output = input - UCHR('a' - 'A');
    }

    else if ( (input >= UCHR('\340')) && (input <= UCHR('\375')) )
    {
        int index;
        index = input - UCHR('\340');
        if (international_upper[index] != '\0')
        {
            output = UCHR(international_upper[index]);
        }
    }

    return(output);
}



int istr_tolower(value)
register char value;
{
    register unsigned int input;
    register unsigned int output;

    input = UCHR(value);
    output = input;

    if ( (input >= UCHR('A')) && (input <= UCHR('Z')) )
    {
        output = input + UCHR('a' - 'A');
    }

    else if ( (input >= UCHR('\300')) && (input <= UCHR('\335')) )
    {
        int index;
        index = input - UCHR('\300');
        if (international_lower[index] != '\0')
        {
            output = UCHR(international_lower[index]);
        }
    }

    return(output);
}

/*
 * istr_strcmp --- Global routine
 * istr_strncmp --- Global routine
 *
 * Perform a case-insensitive string comparison.
 *
 * Inputs:
 *      str1    - char pointer - First string to compare
 *      str2    - char pointer - Second string to compare
 *      maxchar - integer      - max number of character to compare
 *
 * Outputs:
 *      1 if str1 > str2
 *      0 if str1 = str2
 *     -1 if str1 < str2
 *
 * Notes:
 *      This handles the standard ASCII international character set.
 */

#define STR_EQ  0
#define STR_GT  1
#define STR_LT -1

int istr_strcmp(str1, str2)

register char *str1;
register char *str2;

{
    register int compare;

    while ((*str1 != '\0') && (*str2 != '\0'))
    {
        compare = istr_toupper(*str1++) - istr_toupper(*str2++);

        if      (compare > 0) {return(STR_GT);}
        else if (compare < 0) {return(STR_LT);}
    }

    return(STR_EQ);
}


int istr_strncmp(str1, str2, maxchar)

register char *str1;
register char *str2;
register int   maxchar;

{
    register int compare;

    while ((*str1 != '\0') && (*str2 != '\0') && (maxchar-- > 0))
    {
        compare = istr_toupper(*str1++) - istr_toupper(*str2++);

        if      (compare > 0) {return(STR_GT);}
        else if (compare < 0) {return(STR_LT);}
    }

    return(STR_EQ);
}
