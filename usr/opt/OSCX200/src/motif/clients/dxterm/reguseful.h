/* #module reguseful.h "X0.0" */
/*
 *  Title:	reguseful.h
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
*	REGUSEFUL.H		include file containing useful macros and 
*				literals.  Should be included by all
*				modules of the ReGIS parser. 
*
*	Modification history:
*
*  Eric Osman		30-Jul-1993	BL-D
*	- Merge vxt and vms decterm sources.
*
* 004    9-May-84	\RDM
* Make reentrant (put err_code and err_char into RS struct).
*
* 003	??-???-83	\RFD
* Translated the origional Bliss code into C.
*
* 002	 2-May-83	\AFV
* skip_coord will become a global routine, not a macro
*
* 001	21-Feb-83	\DCL
* Change skip_coord to use the argument block for routine COORDINATES.
**/

#ifndef H_REGUSEFUL_H
#define H_REGUSEFUL_H 0

#include "regdevdep.h"

/**
*	Useful literals
**/

#define		XRELATIVE		256
#define		YRELATIVE		512

#define		SHADE_OFF		0
#define		Y_SHADE_TO		1
#define		X_SHADE_TO		2
#define		SHADE_TO_POINT		3
#define		WANT_POLYGON_FILL	4
#define		POLYGON_FILL		5

#define		BLANK			' '

/**
*	Useful macros
**/

#define		IS_TRUE( flag )		((flag) != FALSE)
#define		IS_FALSE( flag )	((flag) == FALSE)
#define		INCH_Q( dummy )		inch_q_unq( FALSE )
#define		INCH_UNQUOTED( dummy )	inch_q_unq( TRUE )
							   /* continue marks */
#define		ERROR( code, character ) if (ERROR_RPRT_ENABLED)	\
			{						\
			rs->err_code = code;				\
			rs->err_char = character;			\
			}

#define		CH_FOLD( c )		((c < 128) ? (c - 32) : (c - 64))
#define		MULTIPLY_BY_SIGN(a, b)	((b < 0) ? (-a) : (a))

#define		pmf			first_process_me = TRUE

#define		MASK_OFF_SIGN_BIT	0X7FFF
#define		SIGN_BIT		15,1
#define		RED_BITS		8,4
#define		GREEN_BITS		4,4
#define		BLUE_BITS		0,4
#define		COLOR_BITS		0,12
#define		MONO_BITS		12,4

#define		CARRIAGE_RETURN		"\013",1

/**
*	Error codes
**/

#define		ATTRIBUTE_ALPHABET_ERROR	5
#define		ALPHABET_OUT_OF_RANGE		4
#define		BEGIN1_START_OVERFLOW		7	/*  *  */
#define		BEGIN2_START_UNDERFLOW		8	/*  *  */
#define		EXTRA_OPTION_COORDINATES	2
#define		EXTRA_COORDINATE_ELEMENTS	3
#define		UNEXPECTED_CHARACTER		1	/*  *  */
#define		TEXT_STANDARD_SIZE_ERROR	9

/**
*	Those error codes that are marked with * above return the ASCII 
*	code of the offending character along with the error code.  Those
*	error codes that are not so marked return 0 as the second 
*	parameter.
**/

/**
*
*	Debugging macros
*
**/
#define		VARIANT			DEBUG_ENABLED
#define		PUT_REPORT( loc, len )	puts_regis( loc, len )

#if DEBUG_ENABLED
#define		PUT_LABELLED_DECIMAL( NumberLabel, number )		\
		Put_String( ("\n  NumberLabel"), PutChar );		\
		PutDecimal( number, PutChar );				\
		PutBreak();

#define		PUT_MESSAGE( message )					\
		Put_String( ( "\n  message"), PutChar );			\
		PutBreak();
#endif

/**
*	End of include file REGUSEFUL.REQ
**/
/**
* 	CMS REPLACEMENT HISTORY 
**/
/**
*	*1 A_VESPER 14-SEP-1983 14:03:06 ""
**/

#endif
