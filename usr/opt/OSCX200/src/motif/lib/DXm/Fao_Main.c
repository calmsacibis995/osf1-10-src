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
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1988 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**			   ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
*/
/*

**++
**
**  FILE:
**
**	fao_main_module.c
**
**  ABSTRACT:
**
**  	Part of the C version of SYS$FAOL (VMS), portable across VMS and UNIX.
**
**	The main module.
**
**  MODIFICATION HISTORY:
**
**	V1-001			Steve Burns, Ann Gargan,	28-Jan-1988
**
**		First version, written for both the VMS and UNIX operating
**		systems.
**
**	V1-002			Steve Burns, Ann Gargan,	18-Feb-1988
**
**		Changed public routines FAO$ and Fao to FAOL$ and FaoL.
**		Provided exactly the same interface as SYS$FAOL for FAOL$,
**		and an equivalent C version for FaoL.
**
**		Provided extra public routines FAO$ and Fao to give
**		the same interface as SYS$FAO.
**
**      V1-003                  Dave Curley                     14-Mar-1988
**
**              Changed the routines to make the code reentrant.   No change
**              to functionality.  Took out all global variables from header
**              files, making them automatic within FAOL$ and FaoL.
**                      Changed the internal mechanism for calling the direc-
**              tive functions.  Provided the general-purpose formatting 
**              function FaoFormat.
**--
*/



/*
 * INCLUDE VMS DEFINITION FILES
 */


#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#ifdef VMS
#include <descrip.h>
#include <time.h>
#endif
#ifdef WIN32
#include <sys\types.h>
#include <sys\timeb.h>
#endif
#include "DXmPrivate.h"
#include "DXmMessI.h"

/************************************************************************/
/*									*/
/* Message definitions for character arrays				*/
/*									*/
/************************************************************************/
#define FAOUNDEFSTAT	_DXmMsgFAO_0000

#define FAOMSGNAME0	_DXmMsgFAOName_0000

#if defined(unix) || defined(UNIX) || defined(__unix__) || defined(WIN32)
/* Time.h is now included by VendorP.h */
struct timeb 
	{
	time_t		time;
	unsigned short 	millitm;
	short		timezone;
	short		dstflag;
	};

typedef struct timeb timeb_t;
#endif

#ifdef WIN32 
#define ftime _ftime
#endif

#ifdef _NO_PROTO
extern char *ctime ();
extern void ftime ();
#else
extern char *ctime (const time_t *bintim);
#ifdef WIN32
extern void ftime (struct _timeb *time_pointer);
#else
extern void ftime (timeb_t *time_pointer);
#endif
#endif
/*
 * DEFINITIONS
 */

#define		FaoStatusNormal		1
#define		FaoStatusBufferOvf	2
#define		FaoStatusAccVio		4
#define		FaoStatusBadDirec	6
#define		FaoStatusBadDescrip	8
#define		FaoStatusNoFreeMem	10
#define		FaoStatusBadCS		12

/*
 * TWO SYMBOL DIRECTIVES
 */

#define DIR_AC		0
#define	DIR_AD		1
#define	DIR_AF		2

#ifdef	VMS

#define	DIR_AS		3

#endif

#define	DIR_AZ 		4
#define	DIR_OB 		5
#define	DIR_OW 		6
#define	DIR_OL 		7
#define	DIR_XB 		8
#define	DIR_XW 		9
#define	DIR_XL 		10
#define	DIR_ZB 		11
#define	DIR_ZW 		12
#define	DIR_ZL 		13
#define	DIR_UB 		14
#define	DIR_UW 		15
#define	DIR_UL 		16
#define	DIR_SB 		17
#define	DIR_SW 		18
#define	DIR_SL 		19
#define	DIR_PS 		20	/* %S */
#define	DIR_PT 		21	/* %T */

#ifdef	VMS

#define	DIR_PU 		22	/* %U */

#endif

#define	DIR_PD 		24	/* %D */
#define	DIR_PC 		25	/* %C */
#define	DIR_PE 		26	/* %E */
#define DIR_CS          27      /* CS - DDIS compound string */
#define	DIR_PF 		28	/* %F */

/*
 * ONE SYMBOL DIRECTIVES
 */

#define	DIR_NS 		29  	/* <  */
#define	DIR_ST		30	/* *  */
#define	DIR_SS 		31	/* /  */
#define	DIR_UN		32	/* _  */
#define	DIR_CI		33	/* ^  */
#define	DIR_EX 		34	/* !  */
#define	DIR_GT 		35	/* >  */
#define	DIR_MI 		36	/* -  */
#define	DIR_PL 		37	/* +  */

#define		LEFT_JUSTIFY		1
#define		RIGHT_JUSTIFY		0
#define		SUPPRESS_ZERO		1
#define		NO_SUPPRESS_ZERO	0
#define		BYTE			256
#define		WORD			65536
#define		LONGWORD		0
#define		OCTAL			8
#define		DECIMAL			10
#define		HEXADECIMAL		16
#define		FAO_TRUE		1
#define		FAO_FALSE		0
#define		NOT_USED		-1
#define		MAX_INT			65535
#define		MAX_LONG		4294967295


/*
 * GENERAL FUNCTIONS
 */

/*
 * FaoAddToBuffer
 *
 * Given a pointer to a char pointer, and a string, etc, the string
 * is inserted into the buffer, and the original char pointer is updated.
 * A status is returned, either FaoStatusNormal, or FaoStatusBufferOvf.
 */

int FaoAddToBuffer(buffer, string, field_len, str_len, justification,
                             char_count, max_char_count, field_width_restriction,
                             field_width_count, last_char)
char		**buffer,
		*string;
int		field_len,
		str_len,
		justification;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status,
		difference,
		num_of_chars,
		num_of_spaces;
char		*start_of_string;

status = FaoStatusNormal;
start_of_string = string;

/*
 * HANDLE CASE WHEN FIELD LENGTH IS "NOT_USED"
 */

if( field_len == NOT_USED )
	{
	field_len = str_len;
	}

difference = field_len - str_len;

difference = (difference < 0) ? 0 : difference;

/*
 * IF RIGHT JUSTIFICATION IS SPECIFIED
 */

if( justification == RIGHT_JUSTIFY )
	{
	/*
	 * CHECK FOR BUFFER OVERFLOW
	 */

	if( ( (*field_width_restriction == FAO_TRUE) &&
	      (difference > (*max_char_count - *char_count)) &&
	      (*field_width_count > (*max_char_count - *char_count)) )  ||
	    ( (*field_width_restriction == FAO_FALSE ) &&
	      (difference > (*max_char_count - *char_count)) ) )
		{
		status = FaoStatusBufferOvf;
		}

	/*
	 * CALCULATE HOW MANY SPACES ARE NEEDED
	 */

	num_of_spaces = (difference < (*max_char_count - *char_count))
			? difference : (*max_char_count - *char_count);

	if(*field_width_restriction == FAO_TRUE)
		{
		num_of_spaces = (num_of_spaces < *field_width_count)
				? num_of_spaces : *field_width_count;

		*field_width_count -= num_of_spaces;
		}

	/*
	 * ADJUST VARIABLES ACCORDINGLY
	 */

	*char_count += num_of_spaces;

	while(num_of_spaces > 0)
		{
		**buffer = ' ';
		(*buffer)++;
		num_of_spaces--;
		}
	}

/*
 * INSERT THE STRING INTO THE USER'S BUFFER
 */

/*
 * CHECK FOR BUFFER OVERFLOW
 */

if( ( (*field_width_restriction == FAO_TRUE) &&
      (str_len > (*max_char_count - *char_count)) &&
      (*field_width_count > (*max_char_count - *char_count)) &&
      (field_len > (*max_char_count - *char_count)) )  ||
    ( (*field_width_restriction == FAO_FALSE ) &&
      (str_len > (*max_char_count - *char_count)) &&
      (field_len > (*max_char_count - *char_count)) ) )
	{
	status = FaoStatusBufferOvf;
	}

/*
 * CALCULATE HOW MANY CHARACTERS ARE TO BE COPIED TO THE OUTPUT BUFFER
 */

num_of_chars = (str_len < field_len) ? str_len : field_len;

num_of_chars = (num_of_chars < (*max_char_count - *char_count))
	       ? num_of_chars : (*max_char_count - *char_count);

if(*field_width_restriction == FAO_TRUE)
	{
	num_of_chars = (num_of_chars < *field_width_count)
			? num_of_chars : *field_width_count;

	*field_width_count -= num_of_chars;
	}

/*
 * INCREMENT VARIABLES ACCORDINGLY
 */

*char_count += num_of_chars;

while(num_of_chars > 0)
	{
	**buffer = *string;
	(*buffer)++;
	string++;
	num_of_chars--;
	}

/*
 * STORE THE LAST CHARACTER ADDED TO BUFFER FOR USE IN %S DIRECTIVE
 */

if(string != start_of_string)
	{
	*last_char = *(string - 1);
	}

/*
 * FILL REST OF BUFFER UPTO MAX LENGTH WITH SPACE, IF LEFT JUSTIFICATION
 */

if( justification == LEFT_JUSTIFY )
	{
	/*
	 * CHECK FOR BUFFER OVERFLOW
	 */

	if( ( (*field_width_restriction == FAO_TRUE) &&
	      (difference > (*max_char_count - *char_count)) &&
	      (*field_width_count > (*max_char_count - *char_count)) )  ||
	    ( (*field_width_restriction == FAO_FALSE ) &&
	      (difference > (*max_char_count - *char_count)) ) )
		{
		status = FaoStatusBufferOvf;
		}

	/*
	 * CALCULATE HOW MANY SPACES ARE NEEDED
	 */

	num_of_spaces = (difference < (*max_char_count - *char_count))
			? difference : (*max_char_count - *char_count);

	if(*field_width_restriction == FAO_TRUE)
		{
		num_of_spaces = (num_of_spaces < *field_width_count)
				? num_of_spaces : *field_width_count;

		*field_width_count -= num_of_spaces;
		}

	/*
	 * INCREMENT VARIABLES ACCORDINGLY
	 */

	*char_count += num_of_spaces;

	while(num_of_spaces > 0)
		{
		**buffer = ' ';
		(*buffer)++;
		num_of_spaces--;
		}
	}

/*
 * RETURN THE NORMAL STATUS
 */

return(status);
}



/*
 * FaoPower(num, power)
 */

unsigned long FaoPower(num, power)
unsigned long 	num,
		power;
{
unsigned long 	result;

for(result = 1; power > 0; --power)
	{
	result *= num;
	}

return(result);
}



/*
 * FaoBaseConvert
 *
 * Given the number to convert, the base to convert to (1 - 16), a pointer
 * to 15 free bytes for storing the string, the number of digits, and whether
 * zero suppression is required or not.  The string representing the num
 * in the new base is stored in the provided area, with no leading spaces.
 */

void FaoBaseConvert(num, base, string, num_digits, suppression, last_value)
unsigned long 	num;
int		base;
char		*string;
int		num_digits,
		suppression;
int             *last_value;
{
static char	digit[20]	= "0123456789ABCDEF",
		new_string[15],
		*new_string_ptr;
unsigned long 	divisor;

/*
 * STORE THE NUMBER IN THE 'last_value' GLOBAL VARIABLE FOR USE
 * IN THE %S DIRECTIVE
 */

*last_value = (int)num;

/*
 * INITIALIZE THE STRING POINTER
 */

new_string_ptr = new_string;

/*
 * HANDLE THE SIMPLE CASE OF NUM = ZERO AND ZERO SUPPRESSION SPECIFIED
 */

if(suppression == SUPPRESS_ZERO && num == 0)
	{
	FaoStrCpy(string, "0");
	return;
	}

divisor = FaoPower((unsigned long)base, (unsigned long)(num_digits - 1));

/*
 * REMOVE UNWANTED DIGITS, IF ANY
 */

if( (base != HEXADECIMAL || num_digits != 8 ) &&
    (base != OCTAL	 || num_digits != 11) &&
    (base != DECIMAL	 || num_digits != 10) )
	{
	num = num % FaoPower((unsigned long)base, (unsigned long)(num_digits));
	}

/*
 * BUILD THE STRING REPRESENTING THE NUMBER IN THE NEW BASE
 */

while(divisor >= 1)
	{
	*new_string_ptr = digit[ (unsigned long)(num / divisor) ];
	num = num % divisor;
	new_string_ptr++;
	divisor /= base;
	}
*new_string_ptr = '\0';

/*
 * RESET NEW STRING PTR
 */

new_string_ptr = new_string;

/*
 * REMOVE UNWANTED ZEROS, IF ZERO SUPPRESSION IS SET
 */

if(suppression == SUPPRESS_ZERO)
	{
	while(*new_string_ptr == '0')
		{
		new_string_ptr++;
		}
	}

/*
 * COPY NEW STRING TO SUPPLIED STRING AREA
 */

FaoStrCpy(string, new_string_ptr);
}



/*
 * FaoStrLen
 *
 * Given a pointer to a null terminated string,
 * it passes back the string's length
 */

int FaoStrLen(string)
char		*string;
{
char		*string_ptr;

string_ptr = string;

while( *string_ptr )
	{
	string_ptr++;
	}

return(string_ptr - string);
}



/*
 * FaoStrCpy
 *
 * Given two pointers to strings, copy string 2 into string 1
 */

FaoStrCpy(string_1, string_2)
char		*string_1,
		*string_2;
{
while( *string_1++ = *string_2++);
}



/*
 * FaoStrnCpy
 *
 * Given two pointers to strings and the number of chars to copy
 * copy string 2 into string 1
 */

FaoStrnCpy(string_1, string_2, num_of_chars)
char		*string_1,
		*string_2;
int		num_of_chars;
{
while(num_of_chars--)
	{
	*string_1++ = *string_2++;
	}
}



/*
 * FaoStrnCmp
 *
 * Given pointers to two strings, and the number of characters to check,
 * it returns 0 if they are equal, else a non zero value
 */

int FaoStrnCmp(string_1, string_2, len)
char		*string_1,
		*string_2;
int		len;
{
while( (*string_1 == *string_2) && (len > 0) )
	{
	string_1++;
	string_2++;
	len--;
	}

return(len);
}


/*
 * FAO DIRECTIVE FUNCTIONS
 */

/*
 * Generally these functions have 3 parameters:
 *
 * 1:	buffer,   this is a pointer to a pointer which points to the current
 *		  place in the user's buffer.
 * 2:	field_len,  this is an int which indicates the maximum length the
 *		  inserted string can be.
 * 3:	prmlist	  this is a pointer to a pointer which points to the current
 *		  parameter in the parameter list
 *
 * All of these functions return a status.
 */


/*
 * FAO STRING DIRECTIVE FUNCTIONS
 */

/*
 * FaoAC
 *
 * Given the standard directive function parameters, add to the output
 * buffer the counted string from the prmlist.
 * This function requires one parameter, the address of the string.
 * The first byte of the string holds the length of the string.
 */

int FaoAC(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
unsigned long	str_len;
int		status;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * OBTAIN THE STRING LENGTH, AND ADD THE STRING TO THE OUTPUT BUFFER
 */

str_len = (unsigned long)*((unsigned char *)**prmlist);

status = FaoAddToBuffer(buffer, (char *)((**prmlist) + 1), field_len,
			str_len, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FaoAD
 *
 * Given the standard directive function parameters, add to the output
 * buffer the string from the prmlist.
 * This function requires two parameters, the length of the string, and
 * the address of the string.
 */

int FaoAD(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
unsigned long	str_len;
int		status;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * FIND THE STRING LENGTH, AND INCREMENT THE PARAMETER LIST POINTER
 */

str_len = (unsigned long)(**prmlist);

(*prmlist)++;

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, (char *)(**prmlist), field_len, 
                        str_len, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FaoAF
 *
 * Given the standard directive function parameters, add to the output
 * buffer the string from the prmlist, suppressing all non printable
 * characters to a '.'.
 * This function requires two parameters, the length of the string, and
 * the address of the string.
 */

int FaoAF(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
unsigned long	str_len,
		string_len;
int		status;
unsigned char	*string;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * FIND THE STRING LENGTH, AND INCREMENT THE PARAMETER LIST POINTER
 */

str_len = (unsigned long)(**prmlist);

(*prmlist)++;

/*
 * REPLACE ALL NON PRINTABLES WITH '.'
 */

string = (unsigned char *)**prmlist;
string_len = str_len;

while(string_len > 0)
	{
	if( ((*string & 0xF0) == 0x00) || ((*string & 0xF0) == 0x10) ||
	    ((*string & 0xF0) == 0x80) || ((*string & 0xF0) == 0x90) ||

	    ( *string	      == 0x7F) || ( *string         == 0xA0) ||
	    ((*string & 0xEF) == 0xA4) || ( *string         == 0xA6) ||
	    ((*string & 0xFC) == 0xAC) || ( *string         == 0xB8) ||
	    ( *string 	      == 0xBE) || ( *string 	    == 0xD0) ||
	    ( *string 	      == 0xDE) || ( *string 	    == 0xF0) ||
	    ((*string & 0xFE) == 0xFE) )
		{
		*string = '.';
		}
	string++;
	string_len--;
	}

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, **prmlist, field_len, str_len, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FaoAS
 *
 * Given the standard directive function parameters, add to the output
 * buffer the string from the prmlist
 * This function requires one parameter, the address of descriptor of
 * the string
 * 
 * This is a VMS only function
 */

#ifdef	VMS

int FaoAS(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
struct dsc$descriptor_s	*dsc;
int		status;
unsigned long	str_len;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * OBTAIN THE ADDRESS OF THE DESCRIPTOR
 */

dsc = (struct dsc$descriptor_s *)**prmlist;

/*
 * CHECK THE DESCRIPTORS
 */

if( (dsc->dsc$b_class != DSC$K_CLASS_S) &&
    ( (dsc->dsc$b_dtype == DSC$K_DTYPE_V) ||
      (dsc->dsc$b_dtype == DSC$K_DTYPE_P) ||
      (dsc->dsc$b_dtype == DSC$K_DTYPE_T) ) )
	{
	return(FaoStatusAccVio);
	}

/*
 * FIND THE STRING LENGTH
 */

str_len = (unsigned long)(dsc->dsc$w_length);

if (dsc->dsc$b_dtype == DSC$K_DTYPE_V)	     /* str len in bits */
	{
	str_len /= 8;
	}
else if (dsc->dsc$b_dtype == DSC$K_DTYPE_P)  /* str len in nibbles */
	{
	str_len /= 2;
	}
else if (dsc->dsc$b_dtype != DSC$K_DTYPE_T)  /* str len in bytes */
	{
	return(FaoStatusAccVio);
	}

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, dsc->dsc$a_pointer, field_len,
			str_len, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}

#endif


/*
 * FaoAZ
 *
 * Given the standard directive function parameters, add to the output
 * buffer the string from the prmlist.
 * This function requires one parameter, the address of the null terminated
 * string.
 */

int FaoAZ(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, (char *)**prmlist, field_len,
			FaoStrLen(**prmlist), LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FAO ZERO FILLED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoZFConversion
 *
 * Given the output buffer, the field length, the parameter list, the
 * base, size, and number of digits, add to the output buffer the
 * string that represents the number obtained from the parameter list
 * under the given base, with the appropriate size (byte, word, or longword),
 * and number of digits, and with no zero suppression.
 */

int FaoZFConversion(buffer, field_len, prmlist, base, size, digits, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int		base,
		size,
		digits;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
static char	number_string[15];
int		str_len,
		offset,
		status;
unsigned long 	num;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * EXTRACT THE NUMBER TO CONVERT
 */ 

num = (unsigned long)(**prmlist);

/*
 * EXTRACT THE REQUIRED NUMBER OF BITS
 */

if(size != LONGWORD)
	{
	num = num % size;
	}

/*
 * CONVERT THE NUMBER TO A STRING IN THE NEW BASE, ETC.
 */

FaoBaseConvert(num, base, number_string, digits, NO_SUPPRESS_ZERO, last_value);

/*
 * IF STRING LENGTH IS LARGER THAN FIELD LENGTH, THEN TRUNCATE, FROM LEFT
 * OF NUMBER
 */

str_len = FaoStrLen(number_string);

offset = ((str_len > field_len) && (field_len != NOT_USED))
	 ? (str_len - field_len) : 0;

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, &number_string[offset], field_len, str_len,
	                RIGHT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FAO OCTAL ZERO FILLED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoOB, FaoOW, FaoOL
 *
 * Given the standard directive function parameters, add to the output
 * buffer the octal {byte, word, longword} string from the prmlist.
 * This function requires one parameter, the address of the {byte, word
 * longword}.
 */

/*
 * FaoOB
 */

int FaoOB(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, OCTAL, BYTE, 3, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoOW
 */

int FaoOW(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, OCTAL, WORD, 6, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoOL
 */

int FaoOL(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, OCTAL, LONGWORD, 11, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}



/*
 * FAO HEXADECIMAL ZERO FILLED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoXB, FaoXW, FaoXL
 *
 * Given the standard directive function parameters, add to the output
 * buffer the hexadecimal {byte, word, longword} string from the prmlist.
 * This function requires one parameter, the address of the {byte, word
 * longword}.
 */

/*
 * FaoXB
 */

int FaoXB(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, HEXADECIMAL, BYTE, 2, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoXW
 */

int FaoXW(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, HEXADECIMAL, WORD, 4, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoXL
 */

int FaoXL(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, HEXADECIMAL, LONGWORD, 8, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}



/*
 * FAO DECIMAL ZERO FILLED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoZB, FaaZW, FaoZL
 *
 * Given the standard directive function parameters, add to the output
 * buffer the decimal {byte, word, longword} string from the prmlist.
 * This function requires one parameter, the address of the {byte, word
 * longword}.
 */

/*
 * FaoZB
 */

int FaoZB(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, DECIMAL, BYTE, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoZW
 */

int FaoZW(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, DECIMAL, WORD, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoZL
 */

int FaoZL(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoZFConversion(buffer, field_len, prmlist, DECIMAL, LONGWORD, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}


/*
 * FAO BLANK FILLED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoBFConversion
 *
 * Given the output buffer, the field length, the parameter list, the
 * base, size, and number of digits, add to the output buffer the
 * string that represents the number obtained from the parameter list
 * under the given base, with the appropriate size (byte, word, or longword),
 * and number of digits, and with zero suppression.
 */

int FaoBFConversion(buffer, field_len, prmlist, base, size, digits, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int		base,
		size,
		digits;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
static char	number_string[15];
int		str_len,
		offset,
		status;
unsigned long	num;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * EXTRACT THE NUMBER TO CONVERT
 */ 

num = (unsigned long)(**prmlist);

/*
 * EXTRACT THE REQUIRED NUMBER OF BITS
 */

if(size != LONGWORD)
	{
	num = num % size;
	}

/*
 * CONVERT THE NUMBER INTO THE STRING REPRESENTING THE NUMBER IN
 * THE NEW BASE
 */

FaoBaseConvert(num, base, number_string, digits, SUPPRESS_ZERO, last_value);


/*
 * IF STRING LENGTH IS LARGER THAN FIELD LENGTH, THEN TRUNCATE, FROM LEFT
 * OF NUMBER
 */

str_len = FaoStrLen(number_string);

offset = ((str_len > field_len) && (field_len != NOT_USED))
	 ? (str_len - field_len) : 0;

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, &number_string[offset], field_len, str_len,
			RIGHT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FAO DECIMAL BLANK FILLED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoUB, FaaUW, FaoUL
 *
 * Given the standard directive function parameters, add to the output
 * buffer the decimal {byte, word, longword} string from the prmlist.
 * This function requires one parameter, the address of the {byte, word
 * longword}.
 */

/*
 * FaoUB
 */

int FaoUB(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoBFConversion(buffer, field_len, prmlist, DECIMAL, BYTE, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoUW
 */

int FaoUW(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoBFConversion(buffer, field_len, prmlist, DECIMAL, WORD, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoUL
 */

int FaoUL(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoBFConversion(buffer, field_len, prmlist, DECIMAL, LONGWORD, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}



/*
 * FAO BLANK FILLED, AND SIGNED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoBFSConversion
 *
 * Given the output buffer, the field length, the parameter list, the
 * base, size, and number of digits, add to the output buffer the
 * string that represents the number obtained from the parameter list
 * under the given base, with the appropriate size (byte, word, or longword),
 * and number of digits, and with zero suppression, and with a sign.
 */

int FaoBFSConversion(buffer, field_len, prmlist, base, size, digits, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int		base,
		size,
		digits;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
static char	number_string[15],
		minus_string[20];
int		str_len,
		offset,
		negative,
		status;
long		num,
		old_num;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * EXTRACT THE NUMBER TO CONVERT, AND STORE IT IN LAST VALUE FOR USE
 * IN !%S AND THE !%C!%E!%F CASE STATEMENT
 */ 

num = (long)(**prmlist);

old_num = num;

/*
 * CHECK THE SIGN OF THE NUMBER, AND IF -VE REMOVE THE SIGN FROM NUM
 */

negative = FAO_FALSE;

switch(size)
	{
	case BYTE: if ( (num & 0xFF) >= 128 )
			{
			negative = FAO_TRUE;
			num = (0x100 - (num & 0xFF));
			}
		   break;
	case WORD: if ( (num & 0xFFFF) >= 32768 )
			{
			negative = FAO_TRUE;
			num = (0x10000 - (num & 0xFFFF));
			}
		   break;
	case LONGWORD: if (num < 0)
			{
			negative = FAO_TRUE;
			num = num * -1;
			}
	}

/*
 * EXTRACT THE REQUIRED NUMBER OF BITS
 */

if(size != LONGWORD)
	{
	num = num & (size - 1);
	}

/*
 * CONVERT THE NUMBER TO A STRING IN THE NEW BASE
 */

FaoBaseConvert(num, base, number_string, digits, SUPPRESS_ZERO, last_value);

/*
 * STORE THE NUMBER IN last_value FOR USE WITH THE %S, AND %C%E%F DIRECTIVES
 */

*last_value = old_num;

/*
 * IF THE NUMBER IS NEGATIVE THEN ADD THE "-" SIGN
 */

if(negative == FAO_TRUE)
	{
	minus_string[0] = '-';
	FaoStrCpy(&minus_string[1], number_string);
	FaoStrCpy(number_string, minus_string);
	}

/*
 * IF STRING LENGTH IS LARGER THAN FIELD LENGTH, THEN TRUNCATE, FROM LEFT
 * OF NUMBER
 */

str_len = FaoStrLen(number_string);

offset = ((str_len > field_len) && (field_len != NOT_USED))
	 ? (str_len - field_len) : 0;

/*
 * ADD THE STRING TO THE OUTPUT BUFFER
 */

status = FaoAddToBuffer(buffer, &number_string[offset], field_len, str_len,
			RIGHT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * INCREMENT THE PARAMETER LIST POINTER, AND RETURN THE STATUS
 */

(*prmlist)++;

return(status);
}



/*
 * FAO DECIMAL BLANK FILLED, AND SIGNED CONVERSION DIRECTIVE FUNCTIONS
 */

/*
 * FaoSB, FaaSW, FaoSL
 *
 * Given the standard directive function parameters, add to the output
 * buffer the signed decimal {byte, word, longword} string from the prmlist.
 * This function requires one parameter, the address of the {byte, word
 * longword}.
 */

/*
 * FaoSB
 */

int FaoSB(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoBFSConversion(buffer, field_len, prmlist, DECIMAL, BYTE, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoSW
 */

int FaoSW(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoBFSConversion(buffer, field_len, prmlist, DECIMAL, WORD, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}

/*
 * FaoSL
 */

int FaoSL(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		**buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoBFSConversion(buffer, field_len, prmlist, DECIMAL, LONGWORD, 10, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char) );
}



/*
 * OUTPUT STRING FORMATTING DIRECTIVE FUNCTIONS
 */

/*
 * FaoSS, FaoUN, FaoCI, FaoEX, FaoST, FaoNS, FaoGT, FaoPS, FaoPU,
 * FaoPC, FaoPE, FaoPF, FaoPT, FaoPD
 *
 * All of these take no parameters, and add a { carriage return & line feed,
 * tab, form feed, !, ...etc} to the output buffer, respectively.
 */

/*
 * FaoSS
 *
 * This function adds a carriage return to the output buffer
 */

int FaoSS(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoAddToBuffer(buffer, "\n", field_len, 1, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char) );
}



/*
 * FaoUN
 *
 * This function adds a tab to the output buffer
 */

int FaoUN(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoAddToBuffer(buffer, "\t", field_len, 1, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char) );
}



/*
 * FaoCI
 *
 * This function inserts a form feed into the output buffer
 */

int FaoCI(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
char		form_feed_string[2];

form_feed_string[0] = 12;
form_feed_string[1] = '\0';

return( FaoAddToBuffer(buffer, form_feed_string, field_len, 1, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char) );
}



/*
 * FaoEX
 *
 * This function adds an exclamation mark to the output buffer
 */

int FaoEX(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
return( FaoAddToBuffer(buffer, "!", field_len, 1, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char) );
}



/*
 * FaoST
 *
 * This functions adds 'field_length' number of 'repeat_char' characters
 * to the output buffer
 */

int FaoST(buffer, field_len, prmlist, repeat_char,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
char            *repeat_char;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status;
char		repeat_string[2];

status = FaoStatusNormal;

repeat_string[0] = *repeat_char;
repeat_string[1] = '\0';

field_len = (field_len == NOT_USED) ? 1 : field_len;

while( (field_len--) > 0 )
	{
	status = FaoAddToBuffer(buffer, repeat_string, NOT_USED, 1,
				LEFT_JUSTIFY,
                                char_count, max_char_count, field_width_restriction,
                                field_width_count, last_char);

	if(status != FaoStatusNormal)
		{
		return(status);
		}
	}

return(status);
}



/*
 * FaoNS
 *
 * This function imposes a field length restriction until the matching
 * !> appears in the intput buffer
 */

int FaoNS(field_len, field_width_restriction, field_width_count)
int             field_len,
                *field_width_restriction,
                *field_width_count;
{
*field_width_count = field_len;
*field_width_restriction = FAO_TRUE;

return(FaoStatusNormal);
}



/*
 * FaoGT
 *
 * This function end the field length restriction imposed by !n>
 */

int FaoGT(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status;
/*
 * FILL THE REST OF THE FIELD WITH BLANKS
 */

status = FaoStatusNormal;

if(*field_width_count > 0)
	{
	while(status == FaoStatusNormal && *field_width_count)
		{
		status = FaoAddToBuffer(buffer, " ", NOT_USED, 1, LEFT_JUSTIFY,
                                        char_count, max_char_count, field_width_restriction,
                                        field_width_count, last_char);
		}
	}
else
	{
	status = FaoStatusNormal;
	}

*field_width_count = 0;
*field_width_restriction = FAO_FALSE;

return(status);
}



/*
 * FaoPU
 *
 * Adds to the output buffer the uic representation of the longword
 * in the parameter list
 */

#ifdef	VMS

/*
 * UIC DIRECTIVE FUNCTIONS.  ONLY FOR VMS
 */

int FaoPU(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
unsigned long	*uic_longword,
		uic_hi_word,
		uic_lo_word;
int		status;
char		uic_string[20],
		*uic_ptr;

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

uic_ptr = uic_string;

/*
 * EXTRACT THE UIC LONGWORD FROM THE PARAMETER LIST,
 * AND INCREMENT THE PARAMETER LIST POINTER
 */

uic_longword = (unsigned long *)*prmlist;
uic_hi_word = (unsigned long)((*uic_longword) / WORD);
uic_lo_word = ((*uic_longword) % WORD);

(*prmlist)++;

/*
 * BUILD THE UIC STRING
 */

*uic_ptr = '[';
uic_ptr++;

/*
 * CONVERT THE uic_hi_word, AND TAKE INTO ACCOUNT THE uic_hi_word = 65535 CASE
 */

if(uic_hi_word == WORD - 1)
	{
	*uic_ptr = '*';
	uic_ptr++;
	*uic_ptr = '\0';
	}
else
	{
	FaoBaseConvert( uic_hi_word, OCTAL, uic_ptr, 6,
		        SUPPRESS_ZERO, last_value);
	}

uic_ptr = (char *)(uic_string + FaoStrLen(uic_string));

*uic_ptr = ',';
uic_ptr++;

/*
 * CONVERT THE uic_lo_word, AND TAKE INTO ACCOUNT THE uic_hi_word = 65535 CASE
 */

if(uic_lo_word == WORD - 1)
	{
	*uic_ptr = '*';
	uic_ptr++;
	*uic_ptr = '\0';
	}
else
	{
	FaoBaseConvert( uic_lo_word, OCTAL, uic_ptr, 6,
		        SUPPRESS_ZERO, last_value);
	}

uic_ptr = (char *)(uic_string + FaoStrLen(uic_string));

*uic_ptr = ']';
uic_ptr++;
*uic_ptr = '\0';

/*
 * ADD STRING TO THE OUTPUT BUFFER, AND RETURN THE STATUS
 */

status = FaoAddToBuffer(buffer, uic_string, field_len, FaoStrLen(uic_string),
			LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

return(status);
}

#endif


/*
 * FaoPS
 *
 * This function adds an 's' if the last user number != 1
 */

int FaoPS(buffer, field_len, prmlist, last_value,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
int             *last_value;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status;
char		s_string[1];

status = FaoStatusNormal;

if(*last_value != 1)
	{
	/*
 	 * ADD AN 'S' TO THE OUTPUT BUFFER IF THE LAST CHAR WAS AN UPPER
	 * CASE CHARACTER, ELSE ADD 's'
	 */

	if( (*last_char & 0x60) == 0x40 )
		{
		s_string[0] = 'S';
		}
	else
		{
		s_string[0] = 's';
		}

		status = FaoAddToBuffer(buffer, s_string, field_len, 1, LEFT_JUSTIFY,
                                        char_count, max_char_count, field_width_restriction,
                                        field_width_count, last_char);
	}

/*
 * RETURN THE STATUS
 */

return(status);
}



/*
 * FaoPC
 *
 * This function copies text from the input buffer upto the next '!' if
 * the value passed in 'field_len' equals 'last_value'
 */

int FaoPC(buffer, field_len, prmlist, input_buffer,
            last_value, case_satisfied, in_case_clause,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
char		**input_buffer;
int             *last_value,
                *case_satisfied,
                *in_case_clause;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
char		*string;
int		status,
		str_len;

str_len = 0;
string = *input_buffer;

/*
 * SET THE GLOBAL VARIABLE THAT INDICATES A CASE CLAUSE HAS STARTED
 */

*in_case_clause = FAO_TRUE;

/*
 * FIND THE END OF THE STRING TO INSERT FROM THE INPUT BUFFER
 * CHECKING THAT WE DO NOT REACH THE END OF THE INPUT BUFFER
 */

while( (**input_buffer != '!') &&
       (**input_buffer != '\0') )
	{
	(*input_buffer)++;
	str_len++;
	}

/*
 * IF THE LAST USER VALUE MATCHES THE VALUE TO COMPARE WITH
 * THEN INSERT THE CHARACTERS FROM THE INPUT BUFFER UPTO THE NEXT '!'
 * INTO THE OUTPUT BUFFER
 */

if( field_len == *last_value )
	{
	/*
	 * ADD THE STRING TO THE OUTPUT BUFFER
	 */

	status = FaoAddToBuffer(buffer, string, NOT_USED, str_len,
				LEFT_JUSTIFY,
                                char_count, max_char_count, field_width_restriction,
                                field_width_count, last_char);

	*case_satisfied = FAO_TRUE;
	}
else
	{
	status = FaoStatusNormal;
	}

return(status);
}



/*
 * FaoPE
 *
 * This function copies text from the input buffer upto the next '!' if
 * no previous %C case clause was satisfied. i.e %E = else clause
 */

int FaoPE(buffer, field_len, prmlist, input_buffer,
             case_satisfied, in_case_clause,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
char		**input_buffer;
int             *case_satisfied,
                *in_case_clause;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
char		*string;
int		status,
		str_len;

/*
 * MAKE SURE WE ARE IN A CASE CLAUSE, ELSE ERROR
 */

if( *in_case_clause == FAO_FALSE )
	{
	return(FaoStatusBadDirec);
	}

str_len = 0;
string = *input_buffer;

/*
 * FIND THE END OF THE STRING TO INSERT FROM THE INPUT BUFFER
 * CHECKING THAT WE DO NOT REACH THE END OF THE INPUT BUFFER
 */

while( (**input_buffer != '!') &&
       (**input_buffer != '\0') )
	{
	(*input_buffer)++;
	str_len++;
	}

/*
 * IF NO PREVIOUS CASE CLAUSE WAS SATISTIED
 * THEN INSERT THE CHARACTERS FROM THE INPUT BUFFER UPTO THE NEXT '!'
 * INTO THE OUTPUT BUFFER
 */

if( *case_satisfied == FAO_FALSE )
	{
	/*
	 * ADD THE STRING TO THE OUTPUT BUFFER
	 */

	status = FaoAddToBuffer(buffer, string, NOT_USED, str_len,
				LEFT_JUSTIFY,
                                char_count, max_char_count, field_width_restriction,
                                field_width_count, last_char);
	}
else
	{
	status = FaoStatusNormal;
	}

return(status);
}



/*
 * FaoPF
 *
 * This function ends the case clause
 */

int FaoPF(case_satisfied, in_case_clause)
int             *case_satisfied,
                *in_case_clause;
{
if( *in_case_clause == FAO_FALSE )
	{
	return(FaoStatusBadDirec);
	}
else
	{
	/*
	 * RESET THE GLOBAL VARIABLES ASSOCIATED WITH THE CASE CLAUSE
	 */

	*case_satisfied = FAO_FALSE;
	*in_case_clause = FAO_FALSE;

	return(FaoStatusNormal);
	}
}



/*
 * FaoPT
 *
 * Using one parameter, it inserts into the buffer the string representing
 * the current time if the parameter value = 0, else absolute time given
 * by parameter (number of seconds since 00:00 1-JAN-1970)
 */

/*
 * VMS FaoPT
 */

FaoPT(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status;
time_t		bin_time;
#ifndef __osf__
timeb_t		time_struct;
#endif
char		time_string[30];

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * GET THE USER PARAMETER, AND INCREMENT THE PARAMETER LIST POINTER
 */

bin_time = (time_t)(**prmlist);

(*prmlist)++;

/*
 * IF GIVEN TIME = 0, i.e. THE CURRENT TIME IS WANTED THEN OBTAIN IT
 */

if( bin_time == 0)
	{
#ifdef __osf__
	bin_time = time((time_t *)NULL) ;
#else
#ifdef WIN32
	ftime((struct _timeb *) &time_struct);
#else
	ftime(&time_struct);
#endif
	bin_time = time_struct.time;
#endif
	}

/*
 * OBTAIN THE STRING REPRESENTATION OF THE CURRENT TIME
 */

FaoStrCpy(time_string, (char *)ctime(&bin_time));

status = FaoAddToBuffer(buffer, &time_string[11], field_len, 8,
	                LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * RETURN THE STATUS
 */

return(status);
}



/*
 * FaoPD
 *
 * Using one parameter, it inserts into the buffer the string representing
 * the current date if the parameter value = 0, else absolute date given
 * by parameter (number of seconds since 00:00 1-JAN-1970)
 */


/*
 * VMS FaoPD
 */

FaoPD(buffer, field_len, prmlist,
          char_count,
          max_char_count,
          field_width_restriction,
          field_width_count,
          last_char)
char		*buffer;
int		field_len;
unsigned long	**prmlist;
unsigned long   *char_count,
                *max_char_count;
int             *field_width_restriction,
                *field_width_count;
char            *last_char;
{
int		status;
time_t		bin_time;
#ifndef __osf__
timeb_t		time_struct;
#endif
char		time_string[30],
		date_string[30];

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

/*
 * GET THE USER PARAMETER, AND INCREMENT THE PARAMETER LIST POINTER
 */

bin_time = (time_t)(**prmlist);

(*prmlist)++;

/*
 * IF GIVEN DATE = 0, i.e. THE CURRENT DATE IS WANTED THEN OBTAIN IT
 */

if( bin_time == 0)
	{
#ifdef __osf__
	  bin_time = time((time_t *)NULL) ;
#else
#ifdef WIN32
	ftime((struct _timeb *) &time_struct);
#else
	ftime(&time_struct);
#endif
	bin_time = time_struct.time;
#endif
	}

/*
 * OBTAIN THE STRING REPRESENTATION OF THE CURRENT TIME AND DATE
 */

FaoStrCpy(time_string, (char *)ctime(&bin_time));

/*
 * BUILD THE NORMAL DATE FORMAT
 */

FaoStrnCpy(&date_string[0], &time_string[8], 2);
FaoStrnCpy(&date_string[2], "-", 1);
FaoStrnCpy(&date_string[3], &time_string[4], 3);
FaoStrnCpy(&date_string[6], "-", 1);
FaoStrnCpy(&date_string[7], &time_string[20], 4);
date_string[11] = '\0';

/*
 * CONVERT THE MONTH FROM Mmm FORMAT TO MMM FORMAT
 */

date_string[4] &= 0xDF;
date_string[5] &= 0xDF;

status = FaoAddToBuffer(buffer, date_string, field_len, 11, LEFT_JUSTIFY,
                        char_count, max_char_count, field_width_restriction,
                        field_width_count, last_char);

/*
 * RETURN THE STATUS
 */

return(status);
}



/*
 * DIRECTIVES FOR PARAMETER INTERPRETATION
 *
 * FaoMI, FaoPL
 */

/*
 * FaoMI
 *
 * Reuse the last user parameter
 */

int FaoMI(prmlist, first_param)
unsigned long	**prmlist,
                **first_param;
{

/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

if( *prmlist == *first_param )
	{
	return(FaoStatusAccVio);
	}
else
	{
	(*prmlist)--;
	return(FaoStatusNormal);
	}
}



/*
 * FaoPL
 *
 * Skip to the next user parameter
 */

int FaoPL(prmlist)
unsigned long	**prmlist;
{
/*
 * CHECK FOR A NULL PARAM LIST
 */

if( *prmlist == 0 )
	{
	return(FaoStatusAccVio);
	}

(*prmlist)++;

return(FaoStatusNormal);
}

    

/*
 * VMS INTERFACE FAO FUNCTION
 */

#ifdef VMS

int FAO$(ctrstr, outlen, outbuf,
	 p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
	 p11, p12, p13, p14, p15, p16, p17, p18, p19, p20)
struct dsc$descriptor_s		*ctrstr;
int				*outlen;
struct dsc$descriptor_s		*outbuf;
long	p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
	p11, p12, p13, p14, p15, p16, p17, p18, p19, p20;
{
long		*prmlist;
unsigned long	status;

/*
 * ALLOCATE A PARAM LIST FOR THE 20 PARAMETERS
 */

prmlist = (long *)malloc( sizeof(long) * 20 );

*(prmlist)	= p1;
*(prmlist + 1)	= p2;
*(prmlist + 2)	= p3;
*(prmlist + 3)	= p4;
*(prmlist + 4)	= p5;
*(prmlist + 5)	= p6;
*(prmlist + 6)	= p7;
*(prmlist + 7)	= p8;
*(prmlist + 8)	= p9;
*(prmlist + 9)	= p10;
*(prmlist + 10)	= p11;
*(prmlist + 11)	= p12;
*(prmlist + 12)	= p13;
*(prmlist + 13)	= p14;
*(prmlist + 14)	= p15;
*(prmlist + 15)	= p16;
*(prmlist + 16)	= p17;
*(prmlist + 17)	= p18;
*(prmlist + 18)	= p19;
*(prmlist + 19)	= p20;

/*
 * MAKE THE CALL TO THE FAOL$ FUNCTION
 */

status = FAOL$(ctrstr, outlen, outbuf, prmlist);

/*
 * FREE prmlist SPACE
 */

free(prmlist);

/*
 * RETURN THE STATUS
 */

return(status);
}

#endif



/*
 * C INTERFACE FAO FUNCTION
 */

int Fao(input_buffer, max_out_len, output_buffer,
	 p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
	 p11, p12, p13, p14, p15, p16, p17, p18, p19, p20)

char				*input_buffer;
unsigned long			max_out_len;
char				*output_buffer;
long	p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
	p11, p12, p13, p14, p15, p16, p17, p18, p19, p20;
{
long		*prmlist;
unsigned long	status;

/*
 * ALLOCATE A PARAM LIST FOR THE 20 PARAMETERS
 */

prmlist = (long *)malloc( sizeof(long) * 20 );

*(prmlist)	= p1;
*(prmlist + 1)	= p2;
*(prmlist + 2)	= p3;
*(prmlist + 3)	= p4;
*(prmlist + 4)	= p5;
*(prmlist + 5)	= p6;
*(prmlist + 6)	= p7;
*(prmlist + 7)	= p8;
*(prmlist + 8)	= p9;
*(prmlist + 9)	= p10;
*(prmlist + 10)	= p11;
*(prmlist + 11)	= p12;
*(prmlist + 12)	= p13;
*(prmlist + 13)	= p14;
*(prmlist + 14)	= p15;
*(prmlist + 15)	= p16;
*(prmlist + 16)	= p17;
*(prmlist + 17)	= p18;
*(prmlist + 18)	= p19;
*(prmlist + 19)	= p20;

/*
 * MAKE THE CALL TO THE FAOL$ FUNCTION
 */

status = FaoL(input_buffer, max_out_len, output_buffer, prmlist);

/*
 * FREE prmlist SPACE
 */

free(prmlist);

/*
 * RETURN THE STATUS
 */

return(status);
}



/*
 * VMS INTERFACE FAOL FUNCTION
 */

#ifdef	VMS

int FAOL$(ctrstr, outlen, outbuf, prmlist)
struct dsc$descriptor_s		*ctrstr;
int				*outlen;
struct dsc$descriptor_s		*outbuf;
unsigned long			*prmlist;
{
int				status;
char				*input_buffer,
				*output_buffer;
unsigned long			input_buffer_len,
				output_buffer_len;

unsigned long		char_count,	    /* count of chars in o/p buf */ 	
			max_char_count,     /* max # of chars allowed in    	
					        output buffer */             	
			*first_param;       /* ptr to first parameter in    	
					        list */ 
char			repeat_char;	    /* char to repeat for !n*c */   	
int			field_width_restriction, /* boolean for !n< */  	
			field_width_count,	 /* count for !n< */    	
			last_value,	/* last user number parameter */
			case_satisfied,	/* whether a case clause has
					   been satisfied */
			in_case_clause; /* in a case clause or not */
char			last_char;	/* last char printed to buf */  	
unsigned long           *new_prmlist_head;
                        

/* 
 * Make up default values to give to the main routine.
 */

char_count              = 0;
max_char_count          = 0;
first_param             = prmlist;
repeat_char             = 0;
field_width_restriction = FAO_FALSE;
field_width_count       = 0;
last_value              = NOT_USED;
case_satisfied          = FAO_FALSE;
in_case_clause          = FAO_FALSE;
last_char               = 'a';


/*
 * CHECK FOR WRONGLY DEFAULTED PARAMETERS
 */

if( (ctrstr == 0) || (outbuf == 0) )
	{
	return(FaoStatusAccVio);
	}

/*
 * CHECK THE DESCRIPTORS
 */

if( (outbuf->dsc$b_class != DSC$K_CLASS_S) ||
    (ctrstr->dsc$b_class != DSC$K_CLASS_S) )
	{
	return(FaoStatusBadDescrip);
	}

/*
 * BUILD THE NULL TERMINATED STRINGS
 */

input_buffer_len  = ctrstr->dsc$w_length;
output_buffer_len = outbuf->dsc$w_length;

if (ctrstr->dsc$b_dtype == DSC$K_DTYPE_V)	/* str len in bits */
	{
	input_buffer_len /= 8;
	}
else if (ctrstr->dsc$b_dtype == DSC$K_DTYPE_P)  /* str len in nibbles */
	{
	input_buffer_len /= 2;
	}
else if (ctrstr->dsc$b_dtype != DSC$K_DTYPE_T)  /* str len in bytes */
	{
	return(FaoStatusBadDescrip);
	}

if (outbuf->dsc$b_dtype == DSC$K_DTYPE_V)	/* str len in bits */
	{
	output_buffer_len /= 8;
	}
else if (outbuf->dsc$b_dtype == DSC$K_DTYPE_P)	/* str len in nibbles */
	{
	output_buffer_len /= 2;
	}
else if (outbuf->dsc$b_dtype != DSC$K_DTYPE_T)	/* str len in bytes */
	{
	return(FaoStatusBadDescrip);
	}

input_buffer  = (char *)malloc(input_buffer_len  + 1);
output_buffer = (char *)malloc(output_buffer_len + 1);

if( (input_buffer == 0) || (output_buffer == 0) )
	{
	return(FaoStatusNoFreeMem);
	}

FaoStrnCpy(input_buffer,  ctrstr->dsc$a_pointer, input_buffer_len);

*(input_buffer  + input_buffer_len)  = '\0';

/*
 * Get the status of FaoFormat, the formatting routine.
 */

status = FaoFormat(  input_buffer,
                    output_buffer_len + 1, 
                    output_buffer, 
                    prmlist,
                    &char_count,
                    &max_char_count,
                    &first_param,
                    &repeat_char,
                    &field_width_restriction,
                    &field_width_count,
                    &last_value, 
                    &case_satisfied, 
                    &in_case_clause, 
                    &last_char,
                    &new_prmlist_head               );


/*
 * BUILD THE DESCRIPTOR FOR THE OUTPUT BUFFER
 */

if (outbuf->dsc$b_dtype == DSC$K_DTYPE_V)
	{
	outbuf->dsc$w_length = char_count * 8;
	}
else if (outbuf->dsc$b_dtype == DSC$K_DTYPE_P)
	{
	outbuf->dsc$w_length = char_count * 4;
	}
else 
	{
	outbuf->dsc$w_length = char_count;
	}

FaoStrnCpy(outbuf->dsc$a_pointer, output_buffer, char_count);

/*
 * PASS BACK OUTLEN, IF THE OUTLEN VARIABLE HAS NOT BEEN DEFAULTED
 */

if(outlen != 0)
	{
	*outlen = char_count;
	}

/*
 * FREE THE SPACE ALLOCATED BY THE MEM ALLOCATOR FUNCTION malloc()
 */

free(input_buffer);
free(output_buffer);

/*
 * RETURN THE STATUS
 */

return(status);
}

#endif


/*
 *  FaoL   Use Standalone, or via the other interfaces FAO$, FAOL$, Fao, FaoD
 *         This formats the parameters supplied in the manner specified by the
 *         control string.
 */

int FaoL(input_buffer, max_out_len, output_buffer, prmlist)

char			*input_buffer;                                  	
unsigned long		max_out_len;                                    	
char			*output_buffer;                                 	
unsigned long		*prmlist;                                        	
                                                                        
{                                                                       
unsigned long		char_count,	    /* count of chars in o/p buf */ 	
			max_char_count,     /* max # of chars allowed in    	
					        output buffer */             	
			*first_param;       /* ptr to first parameter in    	
					        list */ 
char			repeat_char;	    /* char to repeat for !n*c */   	
int			field_width_restriction, /* boolean for !n< */  	
			field_width_count,	 /* count for !n< */    	
			last_value,	/* last user number parameter */
			case_satisfied,	/* whether a case clause has
					   been satisfied */
			in_case_clause; /* in a case clause or not */
char			last_char;	/* last char printed to buf */  	
unsigned long           *new_prmlist_head;                        

/* 
 * Make up default values to give to the main routine.
 */

char_count              = 0;
max_char_count          = 0;
first_param             = prmlist;
repeat_char             = 0;
field_width_restriction = FAO_FALSE;
field_width_count       = 0;
last_value              = NOT_USED;
case_satisfied          = FAO_FALSE;
in_case_clause          = FAO_FALSE;
last_char               = 'a';

/*
 * Return the status of FaoFormat, the formatting routine.
 */

return( FaoFormat(  input_buffer,
                    max_out_len,
                    output_buffer,
                    prmlist,
                    &char_count,
                    &max_char_count,
                    &first_param,
                    &repeat_char,
                    &field_width_restriction,
                    &field_width_count,
                    &last_value, 
                    &case_satisfied, 
                    &in_case_clause, 
                    &last_char,
                    &new_prmlist_head)              );

}


/*
 * FaoExtract
 *
 * Given the input buffer, index and the parameter list, the function
 * parses the directive in the input string, and returns the function
 * code, repeat count and max len associated with the directive.
 * It updates the input buffer pointer, and index. (and prmlist if
 * necessary).
 */

int FaoExtract(input_buffer,repeat_count,max_len,func_code,prmlist,repeat_char)
    char		**input_buffer;
    int	        	*repeat_count,
			*max_len, 
    			*func_code;
    unsigned long	**prmlist;
    char                *repeat_char;
{

 char		*directive;
 int		repeat_flag,
		number_flag,
		max_len_flag,
		number;

/*
 * INITIALIZE THE VARIABLES
 */

 number = 0;
 repeat_flag = 0;
 (*repeat_count) =  0;
 (*max_len) = 0;
 number_flag = FAO_FALSE;
 max_len_flag = FAO_FALSE;

 /* 
  * IF NOT END OF INPUT BUFFER, MOVE  
  * POINTER PAST THE EXCLAMATION MARK
  */
 if ((**input_buffer) != '\0')
	{
	 (*input_buffer)++;
	}
 else 
	/*
	 * RETURN ERROR STATUS VALUE
	 */
	{
	return(FaoStatusBadDirec);
	}


 /*
  * IF NOT END OF INPUT BUFFER
  */
 if ((**input_buffer) != '\0')
	{
	/*
	 * IF THE CHAR IS A NUMBER OR A HASH SIGN
	 */
	 if (( ((**input_buffer) >= '0') && ((**input_buffer) <= '9')) || 
			((**input_buffer) == '#')) 
		{
		number = 0;
		number_flag = FAO_TRUE;
		/*
		 * IF CHAR IS A HASH SIGN
		 */
		if ((**input_buffer) == '#')
			{
			/*
			 * CHECK FOR A NULL PARAMETER LIST
			 */
			 if (*prmlist == 0)
				{
				return(FaoStatusAccVio);
				}
			 else
				{
				/*
				 * GET THE NEXT PARAMETER
				 */

				number = (int)(**prmlist);
				(*prmlist)++;
				(*input_buffer)++;
				}
			}
		else    
			/*
			 * IF CHAR IS NOT A HASH SIGN
			 * CONSTRUCT THE NUMBER
			 */
			{
			while ( ((**input_buffer) >= '0') && 
				((**input_buffer) <= '9') &&
				((**input_buffer) != '\0') )
				{
					number = (number * 10) 	+ 
						(**input_buffer - '0');
					(*input_buffer)++;
				}
			}
		}
	}
 else 
	/*
	 * IF END OF INPUT BUFFER
	 */
	{
	return(FaoStatusBadDirec);
	};
 


 /*
  * IF NOT END OF INPUT BUFFER
  */ 
 if ((**input_buffer) != '\0')
	{
	/*
	 * IF CHAR IS AN OPEN BRACKET
	 */
	 if ((**input_buffer) == '(')
 		{
		repeat_flag = 1;

		/*
		 * IF REPEAT COUNT SPECIFIED
		 */
		if (number_flag == FAO_TRUE)
			{
			(*repeat_count) = number;
			}
		else 
			/*
			 * IF NO REPEAT COUNT SPECIFIED
			 */
			{
			(*repeat_count) = 1;
			}

		(*input_buffer)++;
	
		/*
		 * IF NOT END OF INPUT BUFFER
		 */
		if ((**input_buffer) != '\0')
			{
			/*
			 * IF CHAR IS A NUMBER OR A HASH SIGN
			 */
			if (( ((**input_buffer) >= '0') && 
				((**input_buffer) <= '9')) ||
				((**input_buffer) == '#'))
				{
				max_len_flag = FAO_TRUE;
				/*
				 * IF CHAR IS A HASH SIGN
				 */
 				if ((**input_buffer) == '#')
					{
					/*
					 * CHECK FOR A NULL PARAM LIST
					 */
					 if( *prmlist == 0 )
						{
						 return(FaoStatusAccVio);
						}
					 else
						{
					        /*
					         * GET THE NEXT PARAMETER
					         */
	   				        (*max_len) = (int)(**prmlist);
					        (*prmlist)++;
	        			        (*input_buffer)++;
						}
					}
				else
					/*
					 * IF CHAR IS NOT A HASH SIGN
					 */
					{
					while ( ((**input_buffer) >= '0') 
						&& ((**input_buffer) <= '9')
						&& ((**input_buffer) != '\0') )
					{
						(*max_len) = ((*max_len) * 10) 
						    + ((**input_buffer) - '0');
						(*input_buffer)++;
					}
					}
				}
 	        	else 
				/*
				 * IF CHAR IS NOT A NUMBER OR HASH SIGN
				 */
				{
				(*max_len) = NOT_USED;
				}
       			}
		else 
			/*
			 * IF END OF INPUT BUFFER
			 */
			{
			return(FaoStatusBadDirec);
			}
		}
	else	
		/*
		 * IF CHAR IS NOT AN OPEN BRACKET
		 */
		{
		/*
		 * IF FIELD WIDTH SPECIFIED
		 */
		if (number_flag == FAO_TRUE)
			{
			(*repeat_count) = 1;
			(*max_len) = number;
			}
		else 
			/*
			 * IF NO REPEAT COUNT SPECIFIED
			 */
			{
			(*repeat_count) = 1;
			(*max_len) = NOT_USED;
	         	}
      		}
	}
 else 
	/*
	 * IF END OF INPUT BUFFER
	 */
	{
	return(FaoStatusBadDirec);
	};



/*
 * IF NOT END OF INPUT BUFFER
 */
if ((**input_buffer) != '\0')
	{
	/*
	 * COMPARE THIS CHAR WITH ALL ONE CHAR DIRECTIVES
	 * ASSIGN THE CORRECT FUNCTION CODE
	 */ 
	if (!FaoStrnCmp(*input_buffer,"*",1))
		{
		(*func_code) = DIR_ST;
		(*input_buffer)++;
		if(**input_buffer != '\0')
			{
			*repeat_char = *(*input_buffer);
			}
		else
			{
			return(FaoStatusBadDirec);
			}
		}
	else if (!FaoStrnCmp(*input_buffer,"/",1))
		(*func_code) = DIR_SS; 
	else if (!FaoStrnCmp(*input_buffer,"_",1))
		(*func_code) = DIR_UN; 
	else if (!FaoStrnCmp(*input_buffer,"^",1))
		(*func_code) = DIR_CI; 
	else if (!FaoStrnCmp(*input_buffer,"!",1))
		(*func_code) = DIR_EX; 
	else if (!FaoStrnCmp(*input_buffer,">",1))
		(*func_code) = DIR_GT; 
	else if (!FaoStrnCmp(*input_buffer,"+",1))
		(*func_code) = DIR_PL;
	else if (!FaoStrnCmp(*input_buffer,"<",1))
		(*func_code) = DIR_NS; 
	else if (!FaoStrnCmp(*input_buffer,"-",1))
		(*func_code) = DIR_MI; 
	else if ( *( (*input_buffer) + 1) != '\0')
		/*
		 * IF NEXT CHAR NOT END OF INPUT BUFFER
		 * COMPARE THIS CHAR AND NEXT CHAR WITH
		 * ALL TWO CHAR DIRECTIVES AND ASSIGN
		 * CORRECT FUNCTION CODE
		 */
		{
		if (!FaoStrnCmp(*input_buffer,"AC",2))
			(*func_code) = DIR_AC;
		else if (!FaoStrnCmp(*input_buffer,"AD",2))
			(*func_code) = DIR_AD;
		else if (!FaoStrnCmp(*input_buffer,"AF",2))
			(*func_code) = DIR_AF;
#ifdef	VMS
		else if (!FaoStrnCmp(*input_buffer,"AS",2))
/* VMS ONLY */		(*func_code) = DIR_AS;
#endif
		else if (!FaoStrnCmp(*input_buffer,"AZ",2))
			(*func_code) = DIR_AZ; 
		else if (!FaoStrnCmp(*input_buffer,"OB",2))
			(*func_code) = DIR_OB; 
		else if (!FaoStrnCmp(*input_buffer,"OW",2))
			(*func_code) = DIR_OW; 
		else if (!FaoStrnCmp(*input_buffer,"OL",2))
			(*func_code) = DIR_OL; 
		else if (!FaoStrnCmp(*input_buffer,"XB",2))
			(*func_code) = DIR_XB; 
		else if (!FaoStrnCmp(*input_buffer,"XW",2))
			(*func_code) = DIR_XW; 
		else if (!FaoStrnCmp(*input_buffer,"XL",2))
			(*func_code) = DIR_XL; 
		else if (!FaoStrnCmp(*input_buffer,"ZB",2))
			(*func_code) = DIR_ZB; 
		else if (!FaoStrnCmp(*input_buffer,"ZW",2))
			(*func_code) = DIR_ZW; 
		else if (!FaoStrnCmp(*input_buffer,"ZL",2))
			(*func_code) = DIR_ZL; 
		else if (!FaoStrnCmp(*input_buffer,"UB",2))
			(*func_code) = DIR_UB; 
		else if (!FaoStrnCmp(*input_buffer,"UW",2))
			(*func_code) = DIR_UW; 
		else if (!FaoStrnCmp(*input_buffer,"UL",2))
			(*func_code) = DIR_UL; 
		else if (!FaoStrnCmp(*input_buffer,"SB",2))
			(*func_code) = DIR_SB; 
		else if (!FaoStrnCmp(*input_buffer,"SW",2))
			(*func_code) = DIR_SW; 
		else if (!FaoStrnCmp(*input_buffer,"SL",2))
			(*func_code) = DIR_SL; 
		else if (!FaoStrnCmp(*input_buffer,"%S",2))
			(*func_code) = DIR_PS; 
		else if (!FaoStrnCmp(*input_buffer,"%T",2))
			(*func_code) = DIR_PT; 
		else if (!FaoStrnCmp(*input_buffer,"%D",2))
			(*func_code) = DIR_PD; 
#ifdef	VMS
/* VMS ONLY */	else if (!FaoStrnCmp(*input_buffer,"%U",2))
			(*func_code) = DIR_PU; 
#endif
		else if (!FaoStrnCmp(*input_buffer,"%C",2))
			(*func_code) = DIR_PC; 
		else if (!FaoStrnCmp(*input_buffer,"%E",2))
			(*func_code) = DIR_PE; 
		else if (!FaoStrnCmp(*input_buffer,"CS",2))
			(*func_code) = DIR_CS; 
		else if (!FaoStrnCmp(*input_buffer,"%F",2))
			(*func_code) = DIR_PF; 
		else return(FaoStatusBadDirec);

		}
	     else 
		/*
		 * THESE CHARS ARE NOT A VALID DIRECTIVE
		 * RETURN AN ERROR STATUS VALUE
		 */
		{
		return(FaoStatusBadDirec);
		}
	}
else 
	/*
	 * THIS CHAR IS NOT A VALID IDRECTIVE
	 * RETURN AN ERROR STATUS VALUE
	 */
	{
	return(FaoStatusBadDirec);
	};

 /*
  * IF A TWO CHAR DIRECTIVE THEN MOVE 
  * POINTER PAST THE FIRST CHAR
  */
 if ((*func_code) <= DIR_PF)
	{
	 (*input_buffer)++;	
	}

 /*
  * FOR ONE AND TWO CHAR DIRECTIVES
  * MOVE THE POINTER PAST ONE CHAR
  */
 (*input_buffer)++;	

 /*
  * IF THERE WAS AN OPENING BRACKET
  */ 
 if (repeat_flag == FAO_TRUE)
	{
	/*
  	 * IF NOT END OF INPUT BUFFER
	 */
	 if ((**input_buffer) != '\0')
		{
		/*
		 * IF THIS CHAR IS A CLOSING BRACKET
		 */
		if ((**input_buffer) == ')')
			{
			(*input_buffer)++;
			}
		else 
			/*
			 * IF THIS CHAR IS NOT A CLOSING BRAKCET
			 * RETURN AN ERROR STATUS VALUE
			 */
			{
			return(FaoStatusBadDirec);
			}
		}
 	else 
		/*
		 * IF END OF INPUT BUFFER
		 */
		{
		return	(FaoStatusBadDirec);
		}

	}

return(FaoStatusNormal);
}



/*
 *  FaoFormat 
 *         This formats the parameters supplied in the manner specified by the
 *         control string.
 */

int FaoFormat(      input_buffer,
                    max_out_len,
                    output_buffer,
                    prmlist,
                    char_count,
                    max_char_count,
                    first_param,
                    repeat_char,
                    field_width_restriction,
                    field_width_count,
                    last_value, 
                    case_satisfied, 
                    in_case_clause, 
                    last_char,
                    new_prmlist_head)

char			*input_buffer;                                  	
unsigned long		max_out_len;                                    	
char			*output_buffer;                                 	
unsigned long		*prmlist;
unsigned long		*char_count,	    /* count of chars in o/p buf */ 	
			*max_char_count,     /* max # of chars allowed in    	
					        output buffer */             	
			**first_param;       /* ptr to ptr to first parameter
                                                 in list */ 
char			*repeat_char;	    /* char to repeat for !n*c */   	
int			*field_width_restriction, /* boolean for !n< */  	
			*field_width_count,	 /* count for !n< */    	
			*last_value,	/* last user number parameter */
			*case_satisfied,	/* whether a case clause has
					   been satisfied */
			*in_case_clause; /* in a case clause or not */
char			*last_char;	/* last char printed to buf */  	
unsigned long           **new_prmlist_head;
{

	char			*start_output_buffer,
				*old_input_buffer;

	int			max_len,
				repeat_count,
				i,
				status,
				func_code;

/*
 * DO SOME INITIAL CHECKING
 */

	if ( (input_buffer == 0) || (output_buffer == 0) )
		{
                *new_prmlist_head = prmlist;        
		return(FaoStatusAccVio);
		}

	*char_count = 0;

	if(max_out_len == 0)
		{
		*max_char_count = MAX_LONG;
		}
	else
		{
		*max_char_count = max_out_len - 1;	/* space for '\0' */
		}

	start_output_buffer = output_buffer;


/*
 * THE MAIN LOOP
 */

	/*
	 * WHILE NOT END OF INPUT BUFFER
	 */
	while ((*input_buffer) != '\0')
	{
	   /*
	    * IF NOT START OF A DIRECTIVE
	    */
	   if ((*input_buffer) != '!')
		{
		/*
		 * ADD THE CHARACTER FROM THE INPUT BUFFER TO THE OUTPUT
		 * BUFFER
		 */

		 status = FaoAddToBuffer(&output_buffer,
                                         input_buffer,
					 NOT_USED,
                                         1,
                                         LEFT_JUSTIFY,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);

		 input_buffer++;

		 if( status != FaoStatusNormal )
			{
			*output_buffer = '\0';
                        *new_prmlist_head = prmlist;        
			return(status);
			}
		}
	   else 
		/*
		 * IF THIS CHAR IS START OF A DIRECTIVE
		 */ 
		 {

		status = FaoExtract(&input_buffer,&repeat_count,
				&max_len,&func_code,&prmlist,repeat_char);

		if ((status == FaoStatusAccVio) || (status == FaoStatusBadDirec))
                        {
                        *new_prmlist_head = prmlist;        
			return(status);
                        }

		/*
		 * HANDLE THE CASE WHEN A DIRECTIVE OTHER THAN !%C,!%E, OR !%F
		 * IS FOUND IN THE MIDDLE OF A CASE CLAUSE
		 */

		if ((*in_case_clause == FAO_TRUE) &&
		    (func_code != DIR_PC) &&
		    (func_code != DIR_PE) &&
		    (func_code != DIR_PF))
			{
                        *new_prmlist_head = prmlist;        
			return(FaoStatusBadDirec);
			}

		else 
			{ 
			if (repeat_count != 0)
			{
			switch (func_code) {
			/*
			 * EXECUTE THE APPROPRIATE FUNCTION
			 */
	
				case DIR_AC : for (i=1; i<=repeat_count; i++)
						status =FaoAC(&output_buffer,
						      max_len, &prmlist,
                                                      char_count,
                                                      max_char_count,
                                                      field_width_restriction,
                                                      field_width_count,
                                                      last_char);
				         break;
				case DIR_AD : for (i=1; i<=repeat_count; i++)
						status = FaoAD(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	 break;
				case DIR_AF : for (i=1; i<=repeat_count; i++)
						status = FaoAF(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	 break;
#ifdef	VMS
				case DIR_AS : for (i=1; i<=repeat_count; i++)
						status = FaoAS(&output_buffer,
/* VMS ONLY FUNCTION */				      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				   	 break;
#endif
				case DIR_AZ : for (i=1; i<=repeat_count; i++)
						status = FaoAZ(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	 break;
				case DIR_OB : for (i=1; i<=repeat_count; i++)
						status = FaoOB(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				   	 break;
				case DIR_OW : for (i=1; i<=repeat_count; i++)
						status = FaoOW(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
			         	 break;
				case DIR_OL : for (i=1; i<=repeat_count; i++)
						status = FaoOL(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				         break;
				case DIR_XB : for (i=1; i<=repeat_count; i++)
						status = FaoXB(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				         break;
				case DIR_XW : for (i=1; i<=repeat_count; i++)
						status = FaoXW(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				         break;
				case DIR_XL : for (i=1; i<=repeat_count; i++)
						status = FaoXL(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				          break;
				case DIR_ZB : for (i=1; i<=repeat_count; i++)
						status = FaoZB(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
		         		  break;
				case DIR_ZW : for (i=1; i<=repeat_count; i++)
						status = FaoZW(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				          break;
				case DIR_ZL : for (i=1; i<=repeat_count; i++)
						status = FaoZL(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_UB : for (i=1; i<=repeat_count; i++)
						status = FaoUB(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				          break;
				case DIR_UW : for (i=1; i<=repeat_count; i++)
						status = FaoUW(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_UL : for (i=1; i<=repeat_count; i++)
						status = FaoUL(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_SB : for (i=1; i<=repeat_count; i++)
						status = FaoSB(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
					   break;
				case DIR_SW : for (i=1; i<=repeat_count; i++)
						status = FaoSW(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
					  break;
				case DIR_SL : for (i=1; i<=repeat_count; i++)
						status = FaoSL(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
					  break;
				case DIR_PS : for (i=1; i<=repeat_count; i++)
						status = FaoPS(&output_buffer,
						      max_len, &prmlist,
                                                      last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
					 last_char);
				          break;
				case DIR_PT : for (i=1; i<=repeat_count; i++)
						status = FaoPT(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				   	  break;
#ifdef	VMS
/* VMS ONLY FUNCTION */		case DIR_PU : for (i=1; i<=repeat_count; i++)
						status = FaoPU(&output_buffer,
						      max_len, &prmlist, last_value,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
					  break;
#endif
				case DIR_PD : for (i=1; i<=repeat_count; i++)
						status = FaoPD(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_PC : old_input_buffer = input_buffer;
					      for (i=1; i<=repeat_count; i++)
						{
						input_buffer = old_input_buffer;
						status = FaoPC(&output_buffer,
						      max_len, &prmlist,
						      &input_buffer, last_value,
                                                      case_satisfied,
                                                      in_case_clause,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
						}
				    	  break;
				case DIR_PE : old_input_buffer = input_buffer;
					      for (i=1; i<=repeat_count; i++)
						{
						input_buffer = old_input_buffer;
						status = FaoPE(&output_buffer,
						      max_len, &prmlist,
						      &input_buffer,
                                                      case_satisfied,
                                                      in_case_clause,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
						}
				    	  break;
				case DIR_PF : 	status = FaoPF(case_satisfied, in_case_clause);
				    	  break;
				case DIR_NS :	status = FaoNS(max_len, field_width_restriction, field_width_count);
				   	  break;
				case DIR_ST  : for (i=1; i<=repeat_count; i++)
						status = FaoST(&output_buffer,
						      max_len, &prmlist,
                                                      repeat_char,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_SS  : for (i=1; i<=repeat_count; i++)
						status = FaoSS(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				   	  break;
				case DIR_UN : for (i=1; i<=repeat_count; i++)
						status = FaoUN(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_CI : for (i=1; i<=repeat_count; i++)
						status = FaoCI(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
					  break;
				case DIR_EX : for (i=1; i<=repeat_count; i++)
						status = FaoEX(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
					  break;
				case DIR_GT : 	status = FaoGT(&output_buffer,
						      max_len, &prmlist,
                                         char_count,
                                         max_char_count,
                                         field_width_restriction,
                                         field_width_count,
                                         last_char);
				    	  break;
				case DIR_MI : for (i=1; i<=repeat_count; i++)
						status = FaoMI(&prmlist, first_param);
				    	  break;
				case DIR_PL : for (i=1; i<=repeat_count; i++)
						status = FaoPL(&prmlist);
				    	  break;
				default : status = FaoStatusBadDirec;
	   				    }
			if (status != FaoStatusNormal)
				{
				*output_buffer = '\0';
                                *new_prmlist_head = prmlist;        
				return(status);
				}
			}
			}
		    }
           }

	/*
	 * PUT NULL TERMINATOR ON END OF OUTPUT BUFFER
	 */

	*output_buffer = '\0';

	/*
	 * RETURN WITH A Success STATUS
	 */

	*new_prmlist_head = prmlist;        
	return(FaoStatusNormal);
}


/*
 * ULTRIX INTERFACE FAODL FUNCTION
 */

int FaoDL(ctrstr, outbuf, prmlst)
XmString		ctrstr,
			*outbuf;
unsigned long		*prmlst;
{
Boolean			ddis_status,
			fao_status;
XmStringContext		ctrstr_context;
char			*ctrstr_text,
			*ctrstr_text_start,
			*ctrstr_text_origin;
XmStringCharSet         ctrstr_charset;
Boolean			ctrstr_separator;
XmStringDirection       ctrstr_dir;

char            	*ext_input_buffer;
int            		ext_repeat_count,
            		ext_max_len,
			ext_original_max_len,
            		ext_func_code;
long        		ext_prmlst_space[2];
long			*ext_prmlst;
char            	ext_repeat_char;

unsigned long        	ff_char_count,    
            		ff_max_char_count,    
            		*ff_first_param,
            		*ff_new_prmlst;
char            	ff_repeat_char;    
int            		ff_field_width_restriction,    
            		ff_field_width_count,    
            		ff_last_value,
            		ff_case_satisfied,
            		ff_in_case_clause;
char            	ff_last_char;

XmString        	outbuf_cs,
			outbuf_cs_old,
            		outbuf_segment;
char            	*outbuf_text,
			outbuf_null_string[1];
unsigned long        	outbuf_len;
int            		outbuf_cs_empty;

XmString		dd_cs,
			dd_segment;
XmStringContext    	dd_context;
char			*dd_text;
XmStringCharSet		dd_charset;
Boolean			dd_separator;
XmStringDirection  	dd_dir;
char			*dd_outbuf_text;
int			dd_max_len;

int            		i;

/*
 * Check the passed parameters
 */

if ( (ctrstr == 0) || (outbuf == 0) )
	{
	return(FaoStatusAccVio);
	}

/* 
 * Initialize the variables for FaoFormat calls
 */

ff_char_count              = 0;
ff_max_char_count          = 0;
ff_first_param             = prmlst;
ff_repeat_char             = 0;
ff_field_width_restriction = FAO_FALSE;
ff_field_width_count       = 0;
ff_last_value              = NOT_USED;
ff_case_satisfied          = FAO_FALSE;
ff_in_case_clause          = FAO_FALSE;
ff_last_char               = 'a';

outbuf_cs_empty	= FAO_TRUE;
outbuf_len 	= 300 * sizeof(char);
outbuf_text 	= (char *)malloc(outbuf_len + (1 * sizeof(char)));
outbuf_segment 	= (XmString)XtMalloc(sizeof(char));
outbuf_null_string[0] = '\0';

dd_segment	= (XmString)XtMalloc(sizeof(char));

/*
 * Initialize the variables needed for FaoExtract calls
 */

ext_func_code = NOT_USED;
ext_prmlst_space[0] = 1;
ext_prmlst_space[1] = 1;
ext_prmlst = ext_prmlst_space;

/*
 * Initialize the get segment for the control string 'ctrstr'
 */

ddis_status = XmStringInitContext(&ctrstr_context, ctrstr);

/*
 * Main loop
 */

if (ddis_status)
    {
    /*
     * Take each segment of the control string one at a time
     */

    while ( (ddis_status = XmStringGetNextSegment(
			 ctrstr_context, &ctrstr_text,
                         &ctrstr_charset, &ctrstr_dir,
                         &ctrstr_separator)))
        {

	ctrstr_text_origin = ctrstr_text;

        /*
         * handle the text contained in each segment
         */

        ctrstr_text_start = ctrstr_text;

        while ( *ctrstr_text != '\0' )
            {
            /*
             * find a substring of ctrstr that can be processed
             */

            while ( ext_func_code != DIR_CS &&
                *ctrstr_text != '\0' )
                {
                /*
                 * move ptr along string checking for
                 * directives
                 */

                while ( *ctrstr_text != '!' &&
                    *ctrstr_text != '\0' )
                    {
                    ctrstr_text++;
                    }

                if ( *ctrstr_text == '!' )
                    {
                    /*
                     * call FaoExtract to extract the
                     * directive
                     */

                    ext_input_buffer = ctrstr_text;

                    fao_status = FaoExtract(
                            &ext_input_buffer,
                            &ext_repeat_count,
                            &ext_max_len,
                            &ext_func_code,
                            &ext_prmlst,
                            &ext_repeat_char);

    		    /*
		     * Reset ext_prmlst because FaoExtract may modify it
		     */

		    ext_prmlst = ext_prmlst_space;

                    /* can ignore fao_status as we want
                       FaoFormat to return the status */

                    /*
                     * if the directive wasn't !DD then
                     * we can skip over it
                     */

                    if ( ext_func_code != DIR_CS )
                        {
                        ctrstr_text = ext_input_buffer;
                        }
                    }
                }
            /*
              * we now have a substring to be processed by
             * FaoFormat
             */

            if ( ext_func_code == DIR_CS )
                {
                *ctrstr_text = '\0';
                }

            /*
             * Only add this string to the output if it is not null
             */

            if ( FaoStrLen(ctrstr_text_start) != 0 )
                {
                while (    (fao_status = FaoFormat(ctrstr_text_start,
                               outbuf_len,
                               outbuf_text,
                               prmlst,
                               &ff_char_count,
                               &ff_max_char_count,
                               &ff_first_param,
                               &ff_repeat_char,
                               &ff_field_width_restriction,
                               &ff_field_width_count,
                               &ff_last_value,
                               &ff_case_satisfied,
                               &ff_in_case_clause,
                               &ff_last_char,
                               &ff_new_prmlst))
                    == FaoStatusBufferOvf )
                    {
                    outbuf_len += 300 * sizeof(char);
                    free(outbuf_text);
                    outbuf_text = (char *)malloc(outbuf_len
						 + (1 * sizeof(char)));
                    }

		/*
		 * Check the returned status
		 */

		if (fao_status != FaoStatusNormal)
		    {
		    free(outbuf_text);
		    XmStringFree(outbuf_segment);
		    XmStringFree(dd_segment);
		    XtFree(ctrstr_text_origin);
		    XtFree(ctrstr_charset);
  		    XmStringFreeContext (ctrstr_context);

		    return(fao_status);
		    }

                prmlst = ff_new_prmlst;

		/*
		 * Only build a new segment if the outbuf_text != ""
		 */

		if (FaoStrLen(outbuf_text) != 0)
		    {
		    XmStringFree(outbuf_segment);

                    outbuf_segment = XmStringSegmentCreate(outbuf_text,
                                                 ctrstr_charset, ctrstr_dir,
						 ctrstr_separator);


                    if ( outbuf_cs_empty == FAO_TRUE )
                        {
                        outbuf_cs = XmStringCopy(outbuf_segment);
                        outbuf_cs_empty = FAO_FALSE;
                        }
                    else
                        {
		        outbuf_cs_old = outbuf_cs;
                        outbuf_cs = XmStringConcat(outbuf_cs, outbuf_segment);
		        XmStringFree(outbuf_cs_old);
                        }
		    }
                }

            if ( ext_func_code == DIR_CS )
                {
                *ctrstr_text = '!';
                }

            /*
             * Handle the !DD parameter
             */
            if ( ext_func_code == DIR_CS )
                {
                /*
                 * find the field width and repeat count
                 */

                fao_status = FaoExtract(&ctrstr_text,
                            &ext_repeat_count,
                            &ext_original_max_len,
                            &ext_func_code,
                            &prmlst,
                            &ext_repeat_char);

		/*
		 * Check the returned status
		 */

		if (fao_status != FaoStatusNormal)
		    {
		    free(outbuf_text);
		    XmStringFree(outbuf_segment);
		    XmStringFree(dd_segment);
		    XtFree(ctrstr_text_origin);

		    return(fao_status);
		    }

                /*
                 * Handle the repeat count of the directive
                 */

                for (i = 1; i <= ext_repeat_count; i++)
                    {
		    ext_max_len = ext_original_max_len;

                    dd_cs = (XmString)*prmlst;
		    prmlst++;

                    ddis_status = XmStringInitContext(&dd_context, dd_cs);

		    /*
		     * Check the returned status
		     */

		    if (!ddis_status)
		        {
			free(outbuf_text);
			XmStringFree(outbuf_segment);
			XmStringFree(dd_segment);
			XtFree(ctrstr_text_origin);

		        return(FaoStatusBadCS);
		        }

                    while( (ddis_status =
                            XmStringGetNextSegment(
					      dd_context, &dd_text,
                            		      &dd_charset, &dd_dir,
					      &dd_separator)))
                        {

			dd_outbuf_text = outbuf_text;

			/*
			 * Handle the case when ext_max_len = -1
			 */

			if ( ext_max_len == NOT_USED )
			    {
			    ext_max_len = FaoStrLen(dd_text);
			    }

			/*
			 * Restrict the size of string to insert if necessary
			 */

			if ( FaoStrLen(dd_text) >= ext_max_len )
			    {
			    dd_max_len = ext_max_len;
			    }
			else
			    {
			    dd_max_len = FaoStrLen(dd_text);
			    }

			/*
			 * Dont add the segment to the output if it is
			 * going to be null
			 */

			if ( !( (dd_max_len == 0) ||
			      (ff_field_width_restriction == FAO_TRUE) &&
			      (ff_field_width_count == 0) ) )
			    {
                            while ( (fao_status = FaoAddToBuffer(
						  &dd_outbuf_text,
						  dd_text,
						  dd_max_len,
						  FaoStrLen(dd_text),
						  LEFT_JUSTIFY,
						  &ff_char_count,
                               			  &outbuf_len,
                               			  &ff_field_width_restriction,
                               			  &ff_field_width_count,
						  &ff_last_char))
                    		    == FaoStatusBufferOvf )
		                {
                    	        outbuf_len += 300 * sizeof(char);
                    	        free(outbuf_text);
                    	        outbuf_text = (char *)malloc(outbuf_len
							 + (1 * sizeof(char)));
			        dd_outbuf_text = outbuf_text;
		                }

			    *dd_outbuf_text = '\0';

			    XmStringFree(dd_segment);

			    dd_segment = XmStringSegmentCreate(
					outbuf_text, dd_charset,
					dd_dir, FALSE);

                    	    if ( outbuf_cs_empty == FAO_TRUE )
                                {
                                outbuf_cs = XmStringCopy(dd_segment);
                                outbuf_cs_empty = FAO_FALSE;
                                }
                    	    else
                                {
				outbuf_cs_old = outbuf_cs;
                                outbuf_cs = XmStringConcat(outbuf_cs, dd_segment);
				XmStringFree(outbuf_cs_old);
                                }

			    /*
			     * Adjust the field width for the next segment
			     */

			    if ( ext_original_max_len == NOT_USED )
				{
				ext_max_len = NOT_USED;
				}
			    else
				{
				ext_max_len -= dd_max_len;
				}
			    }

			XtFree(dd_charset);
			XtFree(dd_text);
                        }
		  XmStringFreeContext (dd_context);

		    /*
		     * Pad out the !DD segments if greater field width
		     * was specified, num of spaces held in ext_max_len.
		     */

		    if ( ext_max_len > 0 )
			{
		    	dd_outbuf_text = outbuf_text;

                    	while ( (fao_status = FaoAddToBuffer(
						  &dd_outbuf_text,
						  dd_text,
						  ext_max_len,
						  0,
						  LEFT_JUSTIFY,
						  &ff_char_count,
                               			  &outbuf_len,
                               			  &ff_field_width_restriction,
                               			  &ff_field_width_count,
						  &ff_last_char))
                    		== FaoStatusBufferOvf )
		            {
                            outbuf_len += 300 * sizeof(char);
                            free(outbuf_text);
                            outbuf_text = (char *)malloc(outbuf_len
							 + (1 * sizeof(char)));
		            dd_outbuf_text = outbuf_text;
		            }

		    	*dd_outbuf_text = '\0';

		    	XmStringFree(dd_segment);

		    	dd_segment = XmStringLtoRCreate(outbuf_text, "ISO8859-1");

                    	if ( outbuf_cs_empty == FAO_TRUE )
                            {
                            outbuf_cs = XmStringCopy(dd_segment);
                            outbuf_cs_empty = FAO_FALSE;
                            }
                    	else
                            {
			    outbuf_cs_old = outbuf_cs;
                            outbuf_cs = XmStringConcat(outbuf_cs, dd_segment);
			    XmStringFree(outbuf_cs_old);
                            }
			}
                    }


                /*
                 * reset the ext_func_code
                 */

                ext_func_code = NOT_USED;
                }

            /*
             * Update the start of ctrstr text pointer
             */

            ctrstr_text_start = ctrstr_text;
            }

	/*
	 * Free the space allocated to ctrstr_text
	 */

	XtFree(ctrstr_charset);
	XtFree(ctrstr_text_origin);
        }
    XmStringFreeContext (ctrstr_context);
    }

/*
 * Check to see the result of the above operations
 *
 * Something has gone wrong if we reach this point with a ddis_status
 * of TRUE
 */

/*
 * ddis_status must now be FALSE
 */

if (!ddis_status)
    {
    /*
     * return the new compound string to the caller if one was generated
     */


    if (outbuf_cs_empty == FAO_TRUE)
	{
	*outbuf = XmStringLtoRCreate(outbuf_null_string, "ISO8859-1");
	}
    else
	{
	*outbuf = outbuf_cs;
	}

    free(outbuf_text);
    XmStringFree(outbuf_segment);
    XmStringFree(dd_segment);

    return(FaoStatusNormal);
    }
else
    {

/* commented out prinf and exit and replaced with call to XtError...

   Jay

    printf("\nDDIS CS functions have returned an undefined status\n");

    exit(1); 
*/

DXMERROR(FAOMSGNAME0,FAOUNDEFSTAT);

    }
}



/*
 * VMS INTERFACE FAODL$ FUNCTION
 */

#ifdef VMS

int FAODL$(ctrstr, outbuf, prmlst)
XmString		ctrstr,
			*outbuf;
unsigned long		*prmlst;
{
unsigned long		status;

/*
 * Make the call to the FaoDL function
 */

status = FaoDL(ctrstr, outbuf, prmlst);

/*
 * Return the status
 */

return(status);
}

#endif


/*
 * ULTRIX INTERFACE FaoD FUNCTION
 */

int FaoD(ctrstr, outbuf,
         p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
         p11, p12, p13, p14, p15, p16, p17, p18, p19, p20)
XmString		ctrstr,
			*outbuf;
long    p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
        p11, p12, p13, p14, p15, p16, p17, p18, p19, p20;
{
long            *prmlist;
unsigned long   status;

/*
 * ALLOCATE A PARAM LIST FOR THE 20 PARAMETERS
 */

prmlist = (long *)malloc( sizeof(long) * 20 );

*(prmlist)      = p1;
*(prmlist + 1)  = p2;
*(prmlist + 2)  = p3;
*(prmlist + 3)  = p4;
*(prmlist + 4)  = p5;
*(prmlist + 5)  = p6;
*(prmlist + 6)  = p7;
*(prmlist + 7)  = p8;
*(prmlist + 8)  = p9;
*(prmlist + 9)  = p10;
*(prmlist + 10) = p11;
*(prmlist + 11) = p12;
*(prmlist + 12) = p13;
*(prmlist + 13) = p14;
*(prmlist + 14) = p15;
*(prmlist + 15) = p16;
*(prmlist + 16) = p17;
*(prmlist + 17) = p18;
*(prmlist + 18) = p19;
*(prmlist + 19) = p20;

/*
 * MAKE THE CALL TO THE FaoDL FUNCTION
 */

status = FaoDL(ctrstr, outbuf, prmlist);

/*
 * FREE prmlist SPACE
 */

free(prmlist);

/*
 * RETURN THE STATUS
 */

return(status);
}


/*
 * VMS INTERFACE FAOD$ FUNCTION
 */

#ifdef VMS

int FAOD$(ctrstr, outbuf,
         p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
         p11, p12, p13, p14, p15, p16, p17, p18, p19, p20)
XmString		ctrstr,
			*outbuf;
long    p1,  p2,  p3,  p4,  p5,  p6,  p7,  p8,  p9,  p10,
        p11, p12, p13, p14, p15, p16, p17, p18, p19, p20;
{
long            *prmlist;
unsigned long   status;

/*
 * ALLOCATE A PARAM LIST FOR THE 20 PARAMETERS
 */

prmlist = (long *)malloc( sizeof(long) * 20 );

*(prmlist)      = p1;
*(prmlist + 1)  = p2;
*(prmlist + 2)  = p3;
*(prmlist + 3)  = p4;
*(prmlist + 4)  = p5;
*(prmlist + 5)  = p6;
*(prmlist + 6)  = p7;
*(prmlist + 7)  = p8;
*(prmlist + 8)  = p9;
*(prmlist + 9)  = p10;
*(prmlist + 10) = p11;
*(prmlist + 11) = p12;
*(prmlist + 12) = p13;
*(prmlist + 13) = p14;
*(prmlist + 14) = p15;
*(prmlist + 15) = p16;
*(prmlist + 16) = p17;
*(prmlist + 17) = p18;
*(prmlist + 18) = p19;
*(prmlist + 19) = p20;

/*
 * MAKE THE CALL TO THE FaoDL FUNCTION
 */

status = FaoDL(ctrstr, outbuf, prmlist);

/*
 * FREE prmlist SPACE
 */

free(prmlist);

/*
 * RETURN THE STATUS
 */

return(status);
}

#endif
