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
static char *rcsid = "@(#)$RCSfile: moss_cmp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:31:41 $";
#endif
#ifndef lint
static char *sccsid = "%W%	DECwest	%G%" ;
#endif
/****************************************************************************
 *
 * Copyright (c) Digital Equipment Corporation, 1989, 1990, 1991, 1992.
 * All Rights Reserved.  Unpublished rights reserved
 * under the copyright laws of the United States.
 *
 * The software contained on this media is proprietary
 * to and embodies the confidential technology of
 * Digital Equipment Corporation.  Possession, use,
 * duplication or dissemination of the software and
 * media is authorized only pursuant to a valid written
 * license from Digital Equipment Corporation.
 *
 * RESTRICTED RIGHTS LEGEND   Use, duplication, or
 * disclosure by the U.S. Government is subject to
 * restrictions as set forth in Subparagraph (c)(1)(ii)
 * of DFARS 252.227-7013, or in FAR 52.227-19, as
 * applicable.
 *
 ****************************************************************************
 *
 *
 *    Facility:
 *
 *       Management - POLYCENTER (tm) Common Agent
 *
 *    MOSS - Managed Object Support Routines
 *
 * Abstract:
 *
 *    The following routines are used to compare the values of attributes in the
 *    filter check in the managed objects.
 *
 * Routines:
 *
 *    moss_compare()
 *    moss_asn1_boolean_cmp()
 *    moss_asn1_integer_cmp()
 *    moss_asn1_octet_str_cmp()
 *    moss_asn1_null_cmp()
 *
 * Author:
 *
 *    Miriam Amos
 *
 * Date:
 *
 *    June 14, 1989
 *
 * Revision History :
 *
 *    Rewritten by Wim Colgate, May 8th, 1990.
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Wim Colgate, May 14th, 1990.
 *
 *    Removed '2nd' level relational operations, split out UNIVERSAL
 *    data types from all other (DNA and INET).
 *
 *    Wim Colgate, May 30th, 1990.
 *
 *    Fixed bit-string comparisons
 *
 *    Wim Colgate, May, 31st, 1990.
 *
 *    Changed how bit strings are done! The first byte now contains the
 *    the number of unused bits in the last octet.
 *
 *    Wim Colgate, June 14th, 1990.
 *
 *    Changed valid modifier value from MAN_C_SUCCESS to MAN_C__NORMAL_REPLY
 *
 *    Wim Colgate, November 15th, 1990.
 *
 *    Changed the call to lookup in moss_compare(). Instead of the data type in 
 *    the octet string being passed in, the AVL's tag is passed in.
 *
 *    Miriam Amos Nihart, March 13th, 1991.
 *
 *    Changed valid modifier value fromMAN_C__NORMAL_REPLY to MAN_C__SUCCESS.
 *
 *    Miriam Amos Nihart, June 13th, 1991.
 *
 *    Changes for merging of man status and reply codes.
 *
 *    Miriam Amos Nihart, October 30th, 1991.
 *
 *    Fix for prototyping.
 *
 *    Wim Colgate, November 18th, 1991.
 *
 *    Added moss_asn1_null_cmp().
 *
 *    Paul R. DeStefano, February 24th, 1992.
 *
 *    Remove check for null string in moss_asn1_null_cmp;
 *    string should be ignored if datatype = NULL.
 *
 *    Rich Bouchard, Jr.  August 4, 1992
 *
 *    Merge VMS and Ed Tan's TNSG code.
 *
 *    Ed Tan, August 5, 1992 [rjb]
 *
 *    Fixed ASN1 integer comparisons - they were ignoring most significant byte
 */

/*
 *  Support header files
 */

#ifndef VMS
#include <strings.h>
#endif /* VMS */

/*
 *  Facility header files
 */

#include "man_data.h"
#include "moss_private.h"
#include "moss_asn1.h"
#include "moss_cmp.h"
#include "moss_u_table.h"


comparison *
lookup (
         data_type ,
         table_ptr
       )
/*
 * Function description:
 *
 *    This routine lookups the data type in the correct table and
 *    returns a pointer to a comparison table entry (or NULL if none was found).
 *
 * Arguments:
 *
 *      data_type           The data type we are to search for.
 *      table_ptr           A pointer to the list of table pointers. (universal, etc...)
 *
 * Return value:
 *
 *      an address of a comparison table entry. NULL if none was found.
 *
 * Side effects:
 *
 */

unsigned int data_type ;
comparison **table_ptr ;
{

    comparison *table ;

    if ( table_ptr != NULL )
    {
        if ( ( table = table_ptr[ ( data_type >> 30 ) ] ) != NULL )
            while( table->code != NULL )
            {
                if ( table->code == data_type )
                    return( table ) ;
                table++ ;
            }
    }

    return( NULL ) ;
}


man_status
moss_compare(
             avl1 ,
             avl2 ,
             comp_operand ,
             table_ptr
            )
/*
 * Function description:
 *
 *    This routine dispatches the comparison operation for the filter check.
 *
 * Arguments:
 *
 *      avl1                A pointer to the first avl handle 
 *      avl2                A pointer to the second avl handle 
 *      comp_operand        The first comparison operator.
 *      table_ptr           A pointer to the array of comparison tables.
 *
 * Return value:
 *
 *      MAN_C_TRUE                    The comparison is true.
 *      MAN_C_FALSE                   The comparison was false.
 *      MAN_C_INVALID_OPERATOR        The operation was not valid for the data type specified.
 *
 * Side effects:
 *
 */

avl *avl1 ;
avl *avl2 ;
cmise_filter_relation comp_operand ;
comparison **table_ptr ;

{
    comparison *comparison_entry ;
    avl_element *avl_el ;

    /* 
     * get the current avl element of the first avl handle without
     * affecting the internal pointers.
     */

    avl_el = moss_avl_element( ( iavl * )avl1 ) ;
    if ( avl_el == NULL )
        return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Find the comparison table entry for this data type.
     */

    comparison_entry = lookup( avl_el->tag, table_ptr ) ;
    if ( comparison_entry == NULL )
        return( MAN_C_INVALID_OPERATOR ) ;

    /*
     *  If the test was CMIS_present and we got this far, then the assertion is TRUE
     *  and we can return without any further processing. (PRESENT IS ALWAYS ALLOWED) 
     */

    if ( comp_operand == CMIS_C_PRESENT )
        return ( MAN_C_TRUE ) ;

    /*
     *  Check the operation that we have been asked to carry out and
     *  return an error if it is not supported.  
     */

    if ( ( comparison_entry->valid_comparisons & ( 1 << (unsigned char) comp_operand ) ) == 0 )
        return( MAN_C_INVALID_OPERATOR ) ;

    return( comparison_entry->compare_routine( avl1, avl2, comp_operand ) ) ;

}  /* end of moss_compare() */


man_status
moss_asn1_bit_string_cmp(
                          handle1 ,
                          handle2 ,
                          comp_operand
                         )
/*
 *
 * Function description:
 *
 *    This compares two bit string values.  The only valid comparison is equals.
 *    Later should check for not equals.
 *
 *     NOTE: The first byte contains the number of bits in the bit string.
 *
 * Arguments:
 *
 *    handle1             The avl handle for the first operand.
 *    handle2             The avl handle for the second operand.
 *    comp_operand        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operand was true
 *    MAN_C_FALSE               Comparison based on the operand was false
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation comp_operand ;

{
    char *bs1 ;
    char *bs2 ;
    int stat ;
    int unused1 ;
    int unused2 ;
    int count ;
    man_status return_status = MAN_C_TRUE ;
    man_status status ;
    octet_string *os1 ;
    octet_string *os2 ;
    unsigned int last1 ;
    unsigned int last2 ;

    /*
     * extract the octet string information from the two 
     * avl handles.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    bs1 = os1->string ;
    bs2 = os2->string ;

    /*
     * Extract the length of unused bits (first byte of the bit string)
     * and set the character pointer to the first real bit
     */

    count = os1->length - 1 ; /* skip unused bit count */
    unused1 = bs1[0] ;
    unused2 = bs2[0] ;
    bs1++ ;
    bs2++ ;

    /*
     * If the two bit string lengths are not equal,
     * then the strings can not be equal. (or the number
     * of unused bits in the last byte are not equal).
     */

    if ( (unused1 != unused2)  ||
         (os1->length != os2->length ) )
        return_status = MAN_C_FALSE ;
    else
    {

        /*
         * Loop through each 8 bits, comparing
         * them - as soon as we detect one failure, exit.
         * Note that we handle things differently 
         * if the number of bits is less than 8...
         */

        while( count > 1 )
        {
            if (*bs1++ != *bs2++)
                return ( MAN_C_FALSE ) ;
            count-- ;
        }

        /*
         * now that we went through all the octets 
         * (except the last, check the last byte which
         * may or may not be filled
         */

        last1 = ( *bs1 ) >> (unused1) ;
        last2 = ( *bs2 ) >> (unused2) ;

        if ( last1 != last2 )
            return_status = MAN_C_FALSE ;
    }

    return ( return_status ) ;

}  /* end of moss_asn1_bit_string_cmp() */



man_status
moss_asn1_boolean_cmp(
                      handle1 ,
                      handle2 ,
                      comp_operand
                     )
/*
 *
 * Function description:
 *
 *    This compares two boolean values.  The only valid comparison is equals.
 *    Later should check for not equals.
 *
 * Arguments:
 *
 *    handle1             The avl handle for the first operand.
 *    handle2             The avl handle for the second operand.
 *    comp_operand        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operand was true
 *    MAN_C_FALSE               Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation comp_operand ;

{
    man_status return_status = MAN_C_TRUE ;
    man_status status ;
    octet_string *os1 ;
    octet_string *os2 ;

    /*
     * extract the octet string information from the two 
     * avl handles.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    if ( ( *os1->string && TRUE ) != ( *os2->string && TRUE ) )
        return_status = MAN_C_FALSE ;

    return ( return_status ) ;

}  /* end of moss_asn1_boolean_cmp() */


man_status
moss_asn1_null_cmp(
                      handle1 ,
                      handle2 ,
                      comp_operand
                     )
/*
 *
 * Function description:
 *
 *    This compares two NULL values.  The only valid comparison is equals.
 *
 * Arguments:
 *
 *    handle1             The avl handle for the first operand.
 *    handle2             The avl handle for the second operand.
 *    comp_operand        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operand was true
 *    MAN_C_FALSE               Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation comp_operand ;

{
    int stat ;
    man_status return_status = MAN_C_TRUE ;
    man_status status ;
    octet_string *os1 ;
    octet_string *os2 ;
    unsigned int tag1 ;
    unsigned int tag2 ;

    /*
     * extract the octet string information from the two 
     * avl handles.
     */

    status = moss_avl_point( handle1, NULL, NULL, &tag1, &os1, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, &tag2, &os2, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    if ( os1->data_type != ASN1_C_NULL ||
         os2->data_type != ASN1_C_NULL )
        return_status = MAN_C_FALSE ;

    return ( return_status ) ;

}  /* end of moss_asn1_null_cmp() */


man_status
moss_asn1_integer_cmp(
                      handle1 ,
                      handle2 ,
                      comp_operand
                     )
/*
 *
 * Function description:
 *
 *    This compares two integers. Integers are variable(!) length. Be careful to extend the sign ...
 *    The sign bit is the most significant bit of the most significant byte. Also remember that
 *    negative numbers are represented in two's complement...
 *
 * Arguments:
 *
 *    handle1             The avl handle for the first operand.
 *    handle2             The avl handle for the second operand.
 *    comp_operand        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operand was true
 *    MAN_C_FALSE               Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type.
 *    MAN_C_BAD_PARAMETER       Error found in the parameter handle1 or
 *                              handle2
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation comp_operand ;

{
    man_status return_status = MAN_C_TRUE ;
    int length1 ; 
    int length2 ;
    int result ;
    int sign1 = 0 ;
    int sign2 = 0 ;
    int count ;
    unsigned char byte1 ;
    unsigned char byte2 ;
    unsigned char tmp ;
    man_status status ;
    octet_string *os1 ;
    octet_string *os2 ;

    /*
     * extract the octet string information from the two 
     * avl handles.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, NULL ) ;
    if ( status != MAN_C_SUCCESS ) 
        return( status ) ;

    /*
     * Get the most significant byte, and out the lower order stuff, and
     * get the the sign of it.
     */

    length1 = os1->length ;
    length2 = os2->length ;

    /*
     * Make sure that both integer has length at least 1.
     */

    if ( ( length1 < 1 ) || ( length2 < 1 ) )
	return( MAN_C_BAD_PARAMETER ) ;

    tmp = os1->string[ length1 - 1 ] ;
    tmp = tmp & 0x80 ;
    sign1 = tmp >> 7 ;

    /*
     * Get the most significant byte, and out the lower order stuff, and
     * get the the sign of it.
     */

    tmp = os2->string[ length2 - 1 ] ;
    tmp = tmp & 0x80 ;
    sign2 = tmp >> 7 ;

    /*
     * Ok, the easy test first. If the signs are different, then we can figure
     * the result real easily. (remember, a negative sign is 1, positive is 0).
     */

    if ( sign1 != sign2 )
    {
        switch ( comp_operand )
        {
            case CMIS_C_LTE :
                if ( sign1 < sign2 )
                    return_status = MAN_C_FALSE ;
                break ;
    
            case CMIS_C_GTE :
                if ( sign1 > sign2 )
                    return_status = MAN_C_FALSE ;
                break ;
    
            case CMIS_C_EQUALITY :
                return_status = MAN_C_FALSE ;
                break ;
    
            default :
                return_status = MAN_C_INVALID_OPERATOR ;
                break ;
        }
        return( return_status ) ;
    }

    /*
     * OK. Now the real grunt work. looks like we have to go through byte by
     * byte trying to compare these (potential) monsters. We will start at the most
     * significant byte and work towards the least significant byte. In the case of
     * positive signs, the first byte reached with a '1' is larger. In the case
     * of negative signs, the first byte reached with a '0' is smaller.
     */

    count = ( ( length1 > length2 ) ? length1 : length2 ) ;

    while( count != 0 )
    {

        /*
         * If count is passed the end of the integer representation, 
         * set the byte we are going to use as a comparator to the
         * 'extended' representation. Basicaly sign extend the operand
         * in software.
         */

        if ( count > length1 )
        {
            if ( sign1 != 0 )
                byte1 = 0xff ;
            else
                byte1 = 0 ;
        }
        else
            byte1 = os1->string[ count - 1 ] ;

        /*
         * If count is passed the end of the integer representation, 
         * set the byte we are going to use as a comparator to the
         * 'extended' representation. Basicaly sign extend the operand
         * in software
         */

        if ( count > length2 )
        {
            if ( sign2 != 0 )
                byte2 = 0xff ;
            else
                byte2 = 0 ;
        }
        else
            byte2 = os2->string[ count - 1 ] ;

        /*
         * Now the comparison.
         */

        result = byte1 - byte2 ;

        switch ( comp_operand )
        {
            case CMIS_C_LTE :
                if ( result > 0 )
                {
		    return( MAN_C_FALSE ) ;
                }
                else
                if ( result < 0 )
                {
		    return( MAN_C_TRUE ) ;
                }
                break ;

            case CMIS_C_GTE :
                if ( result > 0 )
                {
		    return( MAN_C_TRUE ) ;
                }
                else
                if ( result < 0 )
                {
		    return( MAN_C_FALSE ) ;
                }
                break ;

            case CMIS_C_EQUALITY :
                if ( result != 0 )
                    return( MAN_C_FALSE ) ;
                break ;

            default :
                return( MAN_C_INVALID_OPERATOR ) ;
                break ;
        }
        --count ;
    }

    /*
     * If we have gotten all the way through the loop, the values are equal.
     */

    return( MAN_C_TRUE ) ;
    
}  /* end of moss_asn1_integer_cmp() */


man_status
moss_asn1_octet_str_cmp(
                        handle1 ,
                        handle2 ,
                        cmp_operand
                       )
/*
 *
 * Function description:
 *
 *    This compares two octet strings.  An octet string is equal if the lengths
 *    are the same and the members are the same.  Str1 is less than str2 if
 *    str1's length is less than str's length or a member of str1 is less than
 *    the corresponding member of str2.
 *
 * Arguments:
 *
 *    handle1             The avl handle for the first operand.
 *    handle2             The avl handle for the second operand.
 *    cmp_operand         The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operand was true
 *    MAN_C_FALSE               Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    int            cmp_length ;
    int            cmp_status ;
    octet_string * os1 ;
    octet_string * os2 ;
    man_status     status ;

    /*
     * Extract the octet string information from the two AVL handles.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, NULL ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, NULL ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    /*
     * Get the minimum length and compare the octet strings.
     */

    cmp_length = ( ( os1->length < os2->length ) ? os1->length : os2->length ) ;
    cmp_status = memcmp( os1->string, os2->string, cmp_length ) ;

    /*
     * Continue according to the comparison operator.
     */

    switch ( cmp_operand )
    {

        case CMIS_C_LTE :
            if ( ( cmp_status < 0 ) ||
                 ( ( cmp_status == 0 ) && ( os1->length <= os2->length ) ) )
                status = MAN_C_TRUE ;
            else
                status = MAN_C_FALSE ;
            break ;

        case CMIS_C_EQUALITY :
            if ( ( cmp_status == 0 ) && ( os1->length == os2->length ) )
                status = MAN_C_TRUE ;
            else
                status = MAN_C_FALSE ;
            break ;

        case CMIS_C_GTE :
            if ( ( cmp_status > 0 ) ||
                 ( ( cmp_status == 0 ) && ( os1->length >= os2->length ) ) )
                status = MAN_C_TRUE ;
            else
                status = MAN_C_FALSE ;
            break ;

        default :
            status = MAN_C_INVALID_OPERATOR ;
            break ;

    }

    return( status ) ;

}   /* moss_asn1_octet_str_cmp */
