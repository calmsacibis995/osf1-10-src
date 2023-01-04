/* #module fields.c "X0.0" */
/*
 *  Title:	fields.c
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
 *	<short description of module contents>
 *
 *  Procedures contained in this module:
 *
 *	<list of procedure names and abstracts>
 *
 *  Author:	<original author>
 *
 *  Modification history:
 *
 *  <modifier's name>	<date>		<ident of revised code>
 *	<description of change and purpose of change>
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 */



/*****	FIELDS *****/

/*  include file to make regis reentrant. RDM 8-May-84 */

#include "regstruct.h"

/*****		F U N C T I O N S    I N    T H I S    F I L E		 *****/
/*									     */
/*	FUNCTION		DESCRIPTION		      VALUE RETURNED */
/*	--------		-----------		      -------------- */
/**
*    get_field(v,v,v)		Select a bit field			yes
*    put_field(v,v,v,a)		Store a bit field			no
**/

get_field( valu, bit_position, nbw_num_bits_wanted )
int	valu;
int	bit_position;
int	nbw_num_bits_wanted;
{
    /**
    *		author:		RFD
    *		creation date:  11/18/83
    *
    * FUNCTION:	To access an arbitrary bit field within a word.
    *
    * EXPLICIT INPUTS:
    *		value		--  value containing the bit field.
    *		bit_position	--  value of starting position of bit field.
    *		num_bits_wanted	--  number of bits wanted to extract.
    *
    * EXPLICIT OUTPUTS:		none, except return value.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * RETURN VALUE:		Value of the extracted bits (or bit field).
    *
    * SIDE EFFECTS:		none
    **/
    int mask;

    mask = 0;
    for ( ; nbw_num_bits_wanted--  > 0; )
    {
	mask = (mask << 1) | 1;	
    }
    return((valu >> bit_position) & mask);
}

put_field( valu, bit_position, nbg_num_bits_given, where_to )
int	valu;
int	bit_position;
int	nbg_num_bits_given;
int	*where_to;
{
    /**
    *		author:		RFD
    *		creation date:  11/18/83
    *
    * FUNCTION:	To store an arbitrary bit field within a word.
    *
    * EXPLICIT INPUTS:
    *		value		--  value of bit field to be stored.
    *		bit_position	--  value of starting position of bit field.
    *		num_bits_given	--  number of bits wanted to store.
    *		where_to	--  address of the location to store bit field.
    *
    * EXPLICIT OUTPUTS:
    *		*where_to	--  the contents of the location pointed to by
    *				    where_to will get as output the stored bits
    *				    in the appropriate position without loss of
    *				    the contents of the remaining bits.
    *
    * IMPLICIT INPUTS:		none
    *
    * IMPLICIT OUTPUTS:		none
    *
    * RETURN VALUE:		none
    *
    * SIDE EFFECTS:		none
    **/
    int mask;

    mask = 0;
    for ( ; nbg_num_bits_given--  > 0; )
    {
	mask = (mask << 1) | 1;	
    }
   *where_to = (*where_to & ~(mask << bit_position)) | (valu << bit_position);
}
