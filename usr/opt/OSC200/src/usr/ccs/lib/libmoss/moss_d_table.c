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
static char *rcsid = "@(#)$RCSfile: moss_d_table.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:31:50 $";
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
 *    This module performs comparisons of avl data type elements. The entire
 *    process is table driven. Depending on the table linked into the moss
 *    library, different application specific comparisons can be made.
 *
 * Routines:
 *
 *    moss_dna_latin1_str_cmp()
 *    unsigned_cmp()
 *    moss_dna_unsigned16_cmp()
 *    moss_dna_unsigned32_cmp()
 *    moss_dna_unsigned64_cmp()
 *    moss_dna_version_cmp()
 *    moss_dna_version_with_edit_cmp()
 *    simplename_matching
 *    match_simple_name()
 *    moss_dna_octet_cmp()
 *    moss_dna_hex_str_cmp()
 *
 * Author:
 *
 *     Wim Colgate
 *
 * Date:
 *
 *    May 1st, 1990.
 *
 * Revision History :
 *
 *    Miriam Amos Nihart, May 14th, 1990.
 *
 *    Change the file names to reflect the 14 character restriction.
 *
 *    Wim Colgate
 *
 *    Changed parameters in comparison routines to 1. Used to pass in a secondary 
 *    operand. Only 1 is passed.
 *
 *    Miriam Amos Nihart, June 18th, 1990
 *
 *    Correct to match_simple_name() - increment the pointer not the contents.
 *
 *    Miriam Amos Nihart, April 10th, 1991.
 *
 *    Correct datatype to strncpy for OSF/1.
 *
 *    Wim Colgate, November 6th, 1991.
 *
 *    Modified match_simple_name: made sure we don't run past the end of the 
 *    source or destination strings, as well as we make sure we skip over
 *    the first two control bytes of the simple name. Also corrected castings.
 *
 *    Ed Tan, June 15, 1992.
 *
 *    Changes from now on will be under CMS control.
 *    
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 */

#ifndef NOIPC
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

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
#include "moss.h"
#include "moss_d_table.h"


man_status
moss_dna_latin1_str_cmp(
                         handle1 ,
                         handle2 ,
                         cmp_operand 
                        )
/*
 *
 * Function description:
 *
 *    This compares two latin1 strings.  A latin1 string is equal if the lengths are the
 *    same and the members are the same.  Str1 is less than str2 if str1's length is less
 *    than str's length or a member of str1 is less than the corresponding member of str2.
 *
 *    WARNING - NOTE THIS IS USING THE ENGLISH COLLATING ALGORITHM.  THIS SHOULD BE CHANGED
 *    TO USE THE LOCAL COLLATING SEQUENCE WHEN AVAILABLE, IE IT SHOULD COLLATE BASE ON THE
 *    USER'S LOCALE.  WHILE THE LOGIC IS THE SAME AS THE ROUTINE moss_asn1_octet_str_cmp()
 *    IT IS BROKEN OUT FOR EASIER MODIFICATION FOR INTERNATIONIZATION.
 *
 * Arguments:
 *
 *    handle1             Avl handle of the first latin1 string.
 *    handle2             Avl handle of the second latin1 string.
 *    comp_operand        The first comparison operator.
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
    int stat = 0 ;
    int str_length ;
    int shorter_length ;
    char *rstr1 ;
    char *rstr2 ;
    char *sptr ;
    octet_string *os1 ;
    octet_string *os2 ;
    int last_one1 ;
    int last_one2 ;
    man_status status ;

    /*
     * First thing to do is get the data pointed to from the passed in AVL handle
     * for both comparitors.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, &last_one1 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, &last_one2 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = MAN_C_TRUE ;

    rstr1 = os1->string ;
    rstr2 = os2->string ;
    
    if ( cmp_operand == CMIS_C_GTE ||
         cmp_operand == CMIS_C_EQUALITY ||
         cmp_operand == CMIS_C_LTE )
    {
        shorter_length = ( os1->length > os2->length ) ? os2->length : os1->length ;
        stat = strncmp( rstr2, rstr1, shorter_length ) ;

        if ( stat == 0 )
            if ( os1->length > os2->length )
                stat = -1 ;
            else
            if ( os1->length < os2->length )
                stat = 1 ;

	if (((stat < 0)  && (cmp_operand != CMIS_C_GTE)) ||
	    ((stat > 0)  && (cmp_operand != CMIS_C_LTE)))
	    return (MAN_C_FALSE);
	  
        return (MAN_C_TRUE);
    }
    else
    {
        switch ( cmp_operand )
        {
            case CMIS_C_INITIAL_STRING :
                if ( strncmp( rstr2, rstr1, os2->length - 1 ) == 0 )
                    return( MAN_C_TRUE ) ;
                else
                    return( MAN_C_FALSE ) ;
                break ;

            case CMIS_C_ANY_STRING :
                sptr = rstr1 ;
                do
                {
#if defined(SYSV) || defined(sun) || defined(sparc)
                    sptr = ( char * )strchr( sptr, ( int )rstr2[ 0 ] ) ;
#else
                    sptr = strchr( sptr, rstr2[ 0 ] ) ;
#endif /* SYSV */
                    if ( sptr == NULL )
                        return( MAN_C_FALSE ) ;

                    if ( strncmp( sptr, rstr2, os2->length - 1 ) == 0 )
                        return( MAN_C_TRUE ) ;
                    sptr++ ;
                } while( TRUE ) ;
                break ;


            case CMIS_C_FINAL_STRING :
                sptr = rstr1 + ( strlen( rstr1 ) - strlen( rstr2 ) ) ;
                if ( strncmp( sptr, rstr2, os2->length - 1 ) == 0 )
                    return( MAN_C_TRUE ) ;
                else
                    return( MAN_C_FALSE ) ;
                break ;

            default :
                return( MAN_C_INVALID_OPERATOR ) ;
                break ;

        }
    }
}  /* end of moss_dna_latin1_str_cmp() */


man_status
moss_dna_version_cmp(
                      handle1 ,
                      handle2 ,
                      cmp_operand 
                     )
/*
 *
 * Function description:
 *
 *    This compares two version numbers as defined in the Network Management T5.0.0.
 *    The ordering of the version number is:
 *        status - V > T > X
 *        the other fields are follow the normal ordering of integers.
 *
 * Arguments:
 *
 *    handle1             Avl handle of the first version number.
 *    handle2             Avl handle of the second version number.
 *    comp_operand        The first comparison operator.
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
    int stat = 0 ;
    int str_length ;
    char *rstr1 ;
    char *rstr2 ; 
    octet_string *os1 ;
    octet_string *os2 ;
    int last_one1 ;
    int last_one2 ;
    man_status status ;

    /*
     * First thing to do is get the data pointed to from the passed in AVL handle
     * for both comparitors.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, &last_one1 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, &last_one2 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    rstr1 = os1->string ;
    rstr2 = os2->string ;

    switch ( rstr1[ 0 ] )
    {
        case 'X' :
            if ( rstr2[ 0 ] != 'X' )
                stat = -1 ;
            break ;

        case 'V' :
            if ( rstr2[ 0 ] != 'V' )
                stat = 1 ;
            break ;

        case 'T' :
            if ( rstr2[ 0 ] == 'V' )
                stat = -1 ;
            if ( rstr2[ 0 ] == 'X' )
                stat = 1 ;
            break ;

        default :
            return ( MAN_C_INVALID_OPERATOR ) ;
    }

    for ( str_length = 1 ; str_length < DNA_C_VERSION_SIZE ; str_length++ )
    {
        if ( rstr1[ str_length ] == rstr2[ str_length ] )
            continue ;

        if ( rstr1[ str_length ] < rstr2[ str_length ] )
            stat = -1 ;
        else
            stat = 1 ;
        break ;
    }

    switch ( stat )
    {
        case -1 :
            if ( cmp_operand != CMIS_C_LTE )
                status = MAN_C_FALSE ;
            break ;

        case 1 :
            if ( cmp_operand != CMIS_C_GTE )
                status = MAN_C_FALSE ;
            break ;

        case 0 :
            if ( cmp_operand != CMIS_C_EQUALITY )
                status = MAN_C_FALSE ;
            break ;

        default :
            status = MAN_C_INVALID_OPERATOR ;
            break ;
    }

    return( status );

}  /* end of moss_dna_version_cmp() */


man_status
moss_dna_version_with_edit_cmp(
                                handle1 ,
                                handle2 ,
                                cmp_operand 
                               )
/*
 *
 * Function description:
 *
 *    This compares two version with edit structures.  The structure is defined to be:
 *        VersionEditNumber ::= [ PRIVATE 50 ] IMPLICIT SEQUENCE {
 *            version VersionNumber ,
 *            editNumber integer }
 *
 * Arguments:
 *
 *    handle1             Avl handle of the first version with edit.
 *    handle2             Avl handle of the second version with edit.
 *    comp_operand        The first comparison operator.
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
    int edit_number1 ;
    int int1 = 0 ;
    int int2 = 0 ;
    char *rstr1 ;
    char *rstr2 ;
    octet_string *os1 ;
    octet_string *os2 ;
    int last_one1 ;
    int last_one2 ;
    man_status status ;

    /*
     *  First compare the VersionNumber within the structure.
     */

    status = moss_dna_version_cmp( handle1, handle2, cmp_operand ) ;
    if ( status != MAN_C_TRUE )
        return ( status ) ;

    /*
     * backup to the version with edit (the above procedure necessarily moves
     * the avl handle pointers to the next avl element. It is assumed that if
     * the above procedure returned MAN_C_TRUE, then we can disregard the 
     * status of the next two calls (because there is valid data one avl
     * element back...)
     */

    moss_avl_backup( handle1 ) ;
    moss_avl_backup( handle2 ) ;

    /*
     * Next, get the data pointed to from the passed in AVL handle
     * for both comparitors.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &os1, &last_one1 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &os2, &last_one2 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    rstr1 = os1->string ;
    rstr2 = os2->string ;

    /*
     *  Now check the second element, editNumber.
     */

#ifdef OSF
    strncpy( rstr1 + DNA_C_VERSION_SIZE, (const char *)&int1, 2 ) ;
    strncpy( rstr2 + DNA_C_VERSION_SIZE, (const char *)&int2, 2 ) ;
#else /* OSF */
    strncpy( rstr1 + DNA_C_VERSION_SIZE, (char *)&int1, 2 ) ;
    strncpy( rstr2 + DNA_C_VERSION_SIZE, (char *)&int2, 2 ) ;
#endif /* OSF */
    edit_number1 = int1 - int2 ;


    switch ( cmp_operand )
    {
        case CMIS_C_LTE :
            if ( edit_number1 > 0 )
                status = MAN_C_FALSE ;
            break ;

        case CMIS_C_GTE :
            if ( edit_number1 < 0 )
                status = MAN_C_FALSE ;
            break ;

        case CMIS_C_EQUALITY :
            if ( edit_number1 != 0 )
                status = MAN_C_FALSE ;
            break ;

        default :
            status = MAN_C_INVALID_OPERATOR ;
            break ;
    }

    return( status ) ;

}  /* end of moss_dna_version_with_edit_cmp() */


static
man_status
unsigned_cmp(
             ptr1 ,
             ptr2 ,
             length ,
             cmp_operator
            )

/*
 *
 * Function description:
 *
 *    This compares two unsigned quantities.
 *
 * Arguments:
 *
 *    ptr1                Pointer to the first unsigned quantity.
 *    ptr2                Pointer to the second unsigned quantity.
 *    length              Length of the unsigned quantities.
 *    cmp_operator        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operator was true
 *    MAN_C_FALSE               Comparison based on the operator was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type
 *
 * Side effects:
 *
 */

unsigned char *ptr1 ;
unsigned char *ptr2 ;
unsigned int length ;
cmise_filter_relation cmp_operator ;

{
    /*
     * Loop to compare the two unsigned quantities one byte at a time.
     */

    for( ; length > 0; length-- )
    {

        int cmp_status ;

        /*
         * Continue according to the comparison operator if the two bytes are
         * not equal.
         */

        cmp_status = *ptr1++ - *ptr2++ ;
        if ( cmp_status != 0 )
            switch ( cmp_operator )
            {
                case CMIS_C_EQUALITY :
                    return( MAN_C_FALSE ) ;
                    break ;

                case CMIS_C_GTE :
                    if ( cmp_status < 0 )
                        return( MAN_C_FALSE ) ;
                    break ;

                case CMIS_C_LTE :
                    if ( cmp_status > 0 )
                        return( MAN_C_FALSE ) ;
                    break ;

                default :
                    return( MAN_C_INVALID_OPERATOR ) ;
                    break ;
            }
    }

    /*
     * The quantities are equal - return TRUE for all comparisions.
     */

    return ( MAN_C_TRUE ) ;

}   /* unsigned_cmp */


man_status
moss_dna_unsigned16_cmp(
                        handle1 ,
                        handle2 ,
                        cmp_operator
                       )
/*
 *
 * Function description:
 *
 *    This compares two 16 bit unsigned quantities.
 *
 * Arguments:
 *
 *    handle1             Avl handle of the first 16 bit unsigned quantity.
 *    handle2             Avl handle of the second 16 bit unsigned quantity.
 *    cmp_operator        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operator was true
 *    MAN_C_FALSE               Comparison based on the operator was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operator ;

{
    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Verify that both quantities are 16 bits long.
     */

    if ( ( os1->length != 2 ) || ( os2->length != 2 ) )
        return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Call unsigned_cmp to perform the comparison.
     */

    return( unsigned_cmp( os1->string, os2->string, 2, cmp_operator ) ) ;

}   /* moss_dna_unsigned16_cmp */


man_status
moss_dna_unsigned32_cmp(
                        handle1 ,
                        handle2 ,
                        cmp_operator
                       )
/*
 *
 * Function description:
 *
 *    This compares two 32 bit unsigned quantities.
 *
 * Arguments:
 *
 *    handle1             Avl handle of the first 32 bit unsigned quantity.
 *    handle2             Avl handle of the second 32 bit unsigned quantity.
 *    cmp_operator        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operator was true
 *    MAN_C_FALSE               Comparison based on the operator was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operator ;

{
    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Verify that both quantities are 32 bits long.
     */

    if ( ( os1->length != 4 ) || ( os2->length != 4 ) )
        return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Call unsigned_cmp to perform the comparison.
     */

    return( unsigned_cmp( os1->string, os2->string, 4, cmp_operator ) ) ;

}   /* moss_dna_unsigned32_cmp */


man_status
moss_dna_unsigned64_cmp(
                        handle1 ,
                        handle2 ,
                        cmp_operator
                       )
/*
 *
 * Function description:
 *
 *    This compares two 64 bit unsigned quantities.
 *
 * Arguments:
 *
 *    handle1             Avl handle of the first 64 bit unsigned quantity.
 *    handle2             Avl handle of the second 64 bit unsigned quantity.
 *    cmp_operator        The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                Comparison based on the operator was true
 *    MAN_C_FALSE               Comparison based on the operator was false
 *    MAN_C_INVALID_OPERATOR    An invalid comparison for this data type
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operator ;

{
    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Verify that both quantities are 64 bits long.
     */

    if ( ( os1->length != 8 ) || ( os2->length != 8 ) )
        return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Call unsigned_cmp to perform the comparison.
     */

    return( unsigned_cmp( os1->string, os2->string, 8, cmp_operator ) ) ;

}   /* moss_dna_unsigned64_cmp */



man_status
simplename_matching(
                    pattern,
                    plen,
                    candidate,
                    clen
                   )

/*
 *
 * Function Description:
 *
 *     This routine is called by match_simple_name() to perform the general
 *     embedded wild card matching algorithm. This routine uses a recursive
 *     algorithm.
 *
 *     Wildcard '*' matches 0 or more characters. Wildcard '?' matches
 *     exactly one character.
 *      
 *
 * Arguments:
 *
 *    pattern	      pointer to the pattern character string not terminated
 *                    by '\0'.
 *    plen            length of pattern string.
 *    candidate       pointer to the candidate character string not terminated
 *                    by '\0'.
 *    clen            length of candidate string.
 *
 * Return Values:
 *
 *      MAN_C_FALSE   Indicates no match.
 *      MAN_C_TRUE    Indicates that the candidate matches the pattern.
 *
 * Side Effects:
 *
 */

char *pattern ;
int plen ;
char *candidate ;
int clen ;

{
    man_status status ;
    char *local_pattern ;
    char *local_candidate ;
    char wild_string = '*' ;
    char wild_character = '?' ;
    int i ;

    /*
     * Loop until we run out of character in candidate.
     */

    while ( clen > 0 )
    {
	/*
	 * If the corresponding characters don't match and the pattern
	 * character is not a wildcard '?', do the following.
         */

	if ( ( *pattern != *candidate ) &&
	     ( *pattern != wild_character ) )
	{
	    /*
	     * If current pattern character is not '*', they
	     * don't match.
	     */

	    if ( *pattern != wild_string )
		return( MAN_C_FALSE ) ;

	    /*
	     * At this point, the current pattern character
	     * is a '*'. If this is the only character remaining
	     * in pattern, it matches everything remaining in
	     * candidate. Therefore we have a match.
	     */

	    if ( plen == 1 )
		return( MAN_C_TRUE ) ;

	    /*
	     * Since '*' matches 0 or more characters, we cannot just
	     * simply advance to the next character since we don't know
	     * how many characters this '*' should match in the
	     * candidate. We use a recursive algorithm to solve this problem.
	     *
	     * For example, to match '*BAR' to 'FOOBAR', we want to compare
	     * all of the following. If any one of them matches, we have a
	     * match.
	     *
	     *   BAR         FOOBAR
	     *   BAR         OOBAR
	     *   BAR         OBAR
             *   BAR         BAR         <--- a match
             *   BAR         AR
             *   BAR         R
	     *
	     * In this case, there is a match. So in the original string
	     * '*BAR' and 'FOOBAR', '*' matches 'FOO' and 'BAR' matches
	     * 'BAR'. This algorithm is implemented in the following
	     * for loop. The reason we implement this routine recursively
	     * is because there might be more '*' and '?' in the
	     * remaining pattern. This way we can correctly match
	     * strings like
	     *
	     *   '*FOO*BAR*'   and   'AFOOANDABAR'
	     */

	    /*
	     * At this point pattern should have at least two characters
	     * remaining, starting with a '*'. Point local_pattern to the
	     * next character after '*'.
	     */

	    local_pattern = pattern ;
	    local_pattern++ ;
	    plen-- ;
	    local_candidate = candidate ;

	    for ( i = 0 ; i < clen ; i++ )
	    {
		/*
		 * Call recursively with the remaining pattern with the
		 * the '*' and the remaining candidate, striping the
		 * beginning character one at a time through this loop.
		 */

		status = simplename_matching( local_pattern, plen,
					      local_candidate,
					      clen - i ) ;

		/*
		 * If we get a match, we are done.
		 */

		if ( status == MAN_C_TRUE )
		    return( MAN_C_TRUE ) ;

		local_candidate++ ;
	    }

	    /*
	     * If we come here, there is no match. For example the following
	     * don't match even though there is a '*'.
	     *
	     *   '*FOO'      and       'FO'
	     */

	    return( MAN_C_FALSE ) ;
	}

	/* If we are here, either characters matches or pattern character is
	 * a wildcard '?' which matches everything. Now advance to the next
	 * character for each.
	 */

	pattern++ ;
	candidate++ ;
	plen-- ;
	clen-- ;

	/*
	 * If we have no pattern character left but there are still more
	 * candidate characters, they don't match.
	 */

	if ( ( plen == 0 ) && ( clen != 0 ) )
	    return( MAN_C_FALSE ) ;

	/*
	 * If we run out of character in both string, they match.
	 */

	if ( ( plen == 0 ) && ( clen == 0 ) )
	    return( MAN_C_TRUE ) ;
    }

    /*
     * The only way we can come here is when both string match
     * up to the end of candidate, and we run out of character
     * in candidate but there are still characters in pattern.
     * If all the remaining characters in pattern are the
     * wildcard '*', they match. (Because '*', '**', '***' etc.
     * matches 0 or more characters.)
     */

    for ( i = 0 ; i < plen ; i++ )
    {
	/*
	 * If at least one remaining pattern character is not '*',
	 * they don't match.
	 */

	if ( *pattern != wild_string )
	    return( MAN_C_FALSE ) ;
	pattern++ ;
    }

    return( MAN_C_TRUE ) ;

}  /* end of simplename_matching() */



man_status
match_simple_name(
                  handle1 ,
                  handle2 ,
                  cmp_operand
                 )

/*
 *
 * Function Description:
 *
 *     This routine performs the general embedded wild card matching algorithm.
 *
 *     There must be two bytes of control information. Wildcard '*' matches
 *     0 or more characters. Wildcard '?' matches exactly one character.
 *      
 *
 * Arguments:
 *
 *    handle1             Avl handle of the source pattern.
 *    handle2             Avl handle of the candidate pattern.
 *    comp_operand        The first comparison operator.
 *
 * Return Values:
 *
 *      MAN_C_FALSE   Indicates no match.
 *      MAN_C_TRUE    Indicates that the candidate matches the pattern.
 *
 * Side Effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{
    char *pattern ;
    char *candidate ;
    char wild_string = '*' ;
    char wild_character = '?' ;
    int plen ;
    int clen ;
    octet_string *pattern_os ;
    octet_string *candidate_os ;
    int last_one1 ;
    int last_one2 ;
    man_status status ;
    int i ;

    /*
     * First thing to do is get the data pointed to from the passed in AVL handle
     * for both comparitors.
     */

    status = moss_avl_point( handle1, NULL, NULL, NULL, &pattern_os, &last_one1 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    status = moss_avl_point( handle2, NULL, NULL, NULL, &candidate_os, &last_one2 ) ;
    if (status != MAN_C_SUCCESS)
        return( status ) ;

    /*
     * Make sure the two string fields are not NULL. They at least have to
     * point to the two bytes of control information.
     */

    if ( ( pattern_os->string == NULL ) ||
			( candidate_os->string == NULL ) )
	return( MAN_C_BAD_PARAMETER ) ;

    pattern = &pattern_os->string[2] ;
    candidate = &candidate_os->string[2] ;

    /*
     * plen and clen (lengths of the string) are adjusted to skip
     * over the two bytes of control information in DNA simple names.
     */

    plen = pattern_os->length - 2;
    clen = candidate_os->length - 2;

    /*
     * We need at least two bytes of control information. If either of
     * the lengths is less than 0, return error.
     */

    if ( ( plen < 0 ) || ( clen < 0 ) )
	return( MAN_C_BAD_PARAMETER ) ;

    /*
     * If both lengths are 0, they match.
     */

    if ( ( plen == 0 ) && ( clen == 0 ) )
	return( MAN_C_TRUE ) ;

    /*
     * If pattern has length 0 they don't match.
     */

    if ( plen == 0 )
	return( MAN_C_FALSE ) ;

    /*
     * If candidate has length 0 but every one of pattern's character is
     * a single '*', they match. Else they don't.
     */

    if ( clen == 0 )
    {
	for ( i = 0 ; i < plen ; i++ )
	{
	    if ( *pattern != wild_string )
		return( MAN_C_FALSE ) ;
	    pattern++ ;
	}

	return( MAN_C_TRUE ) ;
    }

    /*
     * Call the simplename_matching() routine to match. simplename_matching()
     * is a recursive routine.
     */

    return( simplename_matching( pattern, plen, candidate, clen ) ) ;

}  /* end of match_simple_name() */



man_status
moss_dna_octet_cmp(
                   handle1 ,
                   handle2 ,
                   cmp_operand
                  )
/*
 *
 * Function Description:
 *
 *    This compares two octets. An octet is a single byte (8 bits) of data.
 *    The ordering of octet is defined by considering an octet as an unsigned
 *    eight bit quantity. Two octets are equal if they have the same 8 bits
 *    of data.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_FALSE                 Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    int cmp_status ;
    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Make sure that each has a length of 1 byte (8 bits) before comparing
     * the octets.
     */

    if ( ( os1->length != 1 ) || ( os2->length != 1 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    cmp_status = memcmp( os1->string, os2->string, 1 ) ;

    /*
     * Continue according to the comparison operator.
     */

    switch ( cmp_operand )
    {

	case CMIS_C_LTE :
	    if ( cmp_status <= 0 )
		status = MAN_C_TRUE ;
	    else
		status = MAN_C_FALSE ;
	    break ;

	case CMIS_C_EQUALITY :
	    if ( cmp_status == 0 )
		status = MAN_C_TRUE ;
	    else
		status = MAN_C_FALSE ;
	    break ;

	case CMIS_C_GTE :
	    if ( cmp_status >= 0 )
		status = MAN_C_TRUE ;
	    else
		status = MAN_C_FALSE ;
	    break ;

	default :
	    status = MAN_C_INVALID_OPERATOR ;
	    break ;
    
    }

    return( status ) ;

}    /* end of moss_dna_octet_cmp() */


man_status
moss_dna_hex_str_cmp(
                     handle1 ,
                     handle2 ,
                     cmp_operand
                    )
/*
 *
 * Function Description:
 *
 *    This routine compares two hex strings. A hex string is used to represent
 *    a string of zero or more hexadecimal digits. It differs from an octet
 *    string only in that it allows for an odd number of hexadecimal digits.
 *    Two hex strings are equal if they have the same length and hexadecimal
 *    digits. Ordering is defined as with an octet string, except the
 *    comparison is by hexadecimal digit rather than by octet. A hex string
 *    is encoded in ASN.1 as a BIT STRING whose length (in bits) is a multiple
 *    of 4.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_FALSE                 Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *    MAN_C_PROCESSING_FAILURE    Error in memory allocation
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    int cmp_length ;
    int cmp_status ;
    man_status status ;
    octet_string *os1 ;
    octet_string *os2 ;
    char *bs1 ;
    char *bs2 ;
    int unused1 ;
    int unused2 ;
    int real_len1 ;
    int real_len2 ;
    char *local_os1 ;
    char *local_os2 ;
    int i ;
    char temp1 ;
    char temp2 ;
    char *temp_os ;

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
     * The first byte of the string is the number of unused bits in the last
     * octet of the string.
     */

    bs1 = os1->string ;
    bs2 = os2->string ;
    unused1 = bs1[0] ;
    unused2 = bs2[0] ;

    /*
     * Advance to the first real byte.
     */

    bs1++ ;
    bs2++ ;

    /*
     * The number of unused bits must be either 0 or 4. If not, return error.
     */

    if ( ( unused1 != 0 ) && ( unused1 != 4 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    if ( ( unused2 != 0 ) && ( unused2 != 4 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Calculate the real number of hex digits in each. Note that the first
     * byte is the number of unused bits. Each byte can contain two hex digits.
     * If there are 4 unused bits, subtract one.
     */

    real_len1 = ( os1->length - 1 ) * 2 ;
    if ( unused1 == 4 )
	real_len1-- ;

    real_len2 = ( os2->length - 1 ) * 2 ;
    if ( unused2 == 4 )
	real_len2-- ;

    /*
     * Allocate some memory for copying the hex digits (zero-extended to 8 bits)
     * to faciliate comparison.
     */

    MOSS_MALLOC( local_os1, char, real_len1 ) ;
    if ( local_os1 == NULL )
	return( MAN_C_PROCESSING_FAILURE ) ;

    i = real_len1 ;
    temp_os = local_os1 ;

    /*
     * Copy the hex digits two at a time through the while loop. Stop when there
     * is one hex digit left or none left.
     */

    while ( i > 1 )
    {
	/*
	 * Get one byte.
	 */

	temp1 = *bs1 ;

	/*
	 * Right shift the byte to get the high-order hex digit. Then copy it.
	 */

	temp2 = temp1 >> 4 ;
	memcpy( temp_os, &temp2, 1 ) ;
	i-- ;
	temp_os++ ;

	/*
	 * Mask out the high-order hex digit to get the lower-order hex digit.
	 * Then copy it.
	 */

	temp2 = temp1 & 0x0f ;
	memcpy( temp_os, &temp2, 1 ) ;
	i-- ;
	temp_os++ ;

	/*
	 * Increment the pointer to point at the next byte.
	 */

	bs1++ ;
    }

    /*
     * If only one hex digit left, it is in the high-order of the byte. Right
     * shift the byte to get to it and copy it.
     */

    if ( i == 1 )
    {
	temp1 = *bs1 ;
	temp2 = temp1 >> 4 ;
	memcpy( temp_os, &temp2, 1 ) ;
    }

    /*
     * Do the same for the second hex string.
     */

    /*
     * Allocate some memory for copying the hex digits (zero-extended to 8 bits)
     * to faciliate comparison.
     */

    MOSS_MALLOC( local_os2, char, real_len2 ) ;
    if ( local_os2 == NULL )
    {
	MOSS_FREE( local_os1 ) ;
	return( MAN_C_PROCESSING_FAILURE ) ;
    }

    i = real_len2 ;
    temp_os = local_os2 ;

    /*
     * Copy the hex digits two at a time through the while loop. Stop when there
     * is one hex digit left or none left.
     */

    while ( i > 1 )
    {
	/*
	 * Get one byte.
	 */

	temp1 = *bs2 ;

	/*
	 * Right shift the byte to get the high-order hex digit. Then copy it.
	 */

	temp2 = temp1 >> 4 ;
	memcpy( temp_os, &temp2, 1 ) ;
	i-- ;
	temp_os++ ;

	/*
	 * Mask out the high-order hex digit to get the lower-order hex digit.
	 * Then copy it.
	 */

	temp2 = temp1 & 0x0f ;
	memcpy( temp_os, &temp2, 1 ) ;
	i-- ;
	temp_os++ ;

	/*
	 * Increment the pointer to point at the next byte.
	 */

	bs2++ ;
    }

    /*
     * If only one hex digit left, it is in the high-order of the byte. Right
     * shift the byte to get to it and copy it.
     */

    if ( i == 1 )
    {
	temp1 = *bs2 ;
	temp2 = temp1 >> 4 ;
	memcpy( temp_os, &temp2, 1 ) ;
    }

    /*
     * Get the less of the two lengths and compare the two.
     */

    cmp_length = ( ( real_len1 < real_len2 ) ? real_len1 : real_len2 ) ;
    cmp_status = memcmp( local_os1, local_os2, cmp_length ) ;

    /*
     * Continue according to the comparison operator.
     */

    switch ( cmp_operand )
    {

	case CMIS_C_LTE :
	    if ( ( cmp_status < 0 ) ||
		 ( ( cmp_status == 0 ) && ( real_len1 <= real_len2 ) ) )
		status = MAN_C_TRUE ;
	    else
		status = MAN_C_FALSE ;
	    break ;

	case CMIS_C_EQUALITY :
	    if ( ( cmp_status == 0 ) && ( real_len1 == real_len2 ) )
		status = MAN_C_TRUE ;
	    else
		status = MAN_C_FALSE ;
	    break ;

	case CMIS_C_GTE :
	    if ( ( cmp_status > 0 ) ||
		 ( ( cmp_status == 0 ) && ( real_len1 >= real_len2 ) ) )
		status = MAN_C_TRUE ;
	    else
		status = MAN_C_FALSE ;
	    break ;

	default :
	    status = MAN_C_INVALID_OPERATOR ;
	    break ;

    }

    /*
     * Free the memory.
     */

    MOSS_FREE( local_os1 ) ;
    MOSS_FREE( local_os2 ) ;

    return( status ) ;

}    /* end of moss_dna_hex_str_cmp() */



man_status
moss_dna_known_cmp(
                   handle1 ,
                   handle2 ,
                   cmp_operand
                  )
/*
 *
 * Function Description:
 *
 *    This routine compares the first avl which is of DNA_C_KNOWN
 *    [Application 29] data type to the second avl. The DNA_C_KNOWN data type
 *    is a way to specify widecard and the first avl matches anything in
 *    the second avl.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Make sure the KNOWN data type is coded correctly.
     */

    if ( ( os1->length != 0 ) || ( os1->data_type != ASN1_C_NULL ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    return( MAN_C_TRUE ) ;

}    /* end of moss_dna_known_cmp() */



man_status
moss_dna_ip_address_cmp(
                        handle1,
                        handle2,
                        cmp_operand
                       )
/*
 *
 * Function Description:
 *
 *    This routine compares the two IP Addresses. The length of each must be
 *    exactly 4 bytes. Comparison is done using moss_asn1_octet_str_cmp()
 *    routine. Only Equality is supported.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_FALSE                 Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Make sure that each has length of 4 bytes.
     */

    if ( ( os1->length != 4 ) || ( os2->length != 4 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Move back the pointers before we compare.
     */

    status = moss_avl_backup( handle1 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    status = moss_avl_backup( handle2 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    /*
     * Only equality is supported.
     */

    if ( cmp_operand == CMIS_C_EQUALITY )
    {
	status = moss_asn1_octet_str_cmp( handle1, handle2, cmp_operand ) ;
    }
    else
    {
	return( MAN_C_INVALID_OPERATOR ) ;
    }

    return( status ) ;

}    /* end of moss_dna_ip_address_cmp() */



man_status
moss_dna_id802_cmp(
                   handle1,
                   handle2,
                   cmp_operand
                  )
/*
 *
 * Function Description:
 *
 *    This routine compares the two ID802 Addresses. The length of each must be
 *    exactly 6 bytes. Comparison is done using moss_asn1_octet_str_cmp()
 *    routine. Only Equality is supported.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_FALSE                 Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Make sure that each has length of 6 bytes.
     */

    if ( ( os1->length != 6 ) || ( os2->length != 6 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Move back the pointers before we compare.
     */

    status = moss_avl_backup( handle1 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    status = moss_avl_backup( handle2 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    /*
     * Only equality is supported.
     */

    if ( cmp_operand == CMIS_C_EQUALITY )
    {
	status = moss_asn1_octet_str_cmp( handle1, handle2, cmp_operand ) ;
    }
    else
    {
	return( MAN_C_INVALID_OPERATOR ) ;
    }

    return( status ) ;

}    /* end of moss_dna_id802_cmp() */



man_status
moss_dna_idenet_type_cmp(
                         handle1,
                         handle2,
                         cmp_operand
                        )
/*
 *
 * Function Description:
 *
 *    This routine compares two Ethernet Protocol Type. The length of
 *    each must be exactly 2 bytes. Comparison is done using
 *    moss_asn1_octet_str_cmp() routine. Only Equality is supported.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_FALSE                 Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Make sure that each has length of 2 bytes.
     */

    if ( ( os1->length != 2 ) || ( os2->length != 2 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Move back the pointers before we compare.
     */

    status = moss_avl_backup( handle1 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    status = moss_avl_backup( handle2 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    /*
     * Only equality is supported.
     */

    if ( cmp_operand == CMIS_C_EQUALITY )
    {
	status = moss_asn1_octet_str_cmp( handle1, handle2, cmp_operand ) ;
    }
    else
    {
	return( MAN_C_INVALID_OPERATOR ) ;
    }

    return( status ) ;

}    /* end of moss_dna_idenet_type_cmp() */



man_status
moss_dna_id802_snap_cmp(
                        handle1,
                        handle2,
                        cmp_operand
                       )
/*
 *
 * Function Description:
 *
 *    This routine compares two IEEE 802 Sub-Network Access Protocol
 *    (SNAP), Protocol Identification. The length of each must be
 *    exactly 5 bytes. Comparison is done using moss_asn1_octet_str_cmp()
 *    routine. Only Equality is supported.
 *
 * Arguments:
 *
 *    handle1               Avl handle of the first octet.
 *    handle2               Avl handle of the second octet.
 *    cmp_operand           The comparison operator.
 *
 * Return value:
 *
 *    MAN_C_TRUE                  Comparison based on the operand was true
 *    MAN_C_FALSE                 Comparison based on the operand was false
 *    MAN_C_INVALID_OPERATOR      An invalid comparison for this data type.
 *
 * Side effects:
 *
 */

avl *handle1 ;
avl *handle2 ;
cmise_filter_relation cmp_operand ;

{

    octet_string *os1 ;
    octet_string *os2 ;
    man_status status ;

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
     * Make sure that each has length of 5 bytes.
     */

    if ( ( os1->length != 5 ) || ( os2->length != 5 ) )
	return( MAN_C_INVALID_OPERATOR ) ;

    /*
     * Move back the pointers before we compare.
     */

    status = moss_avl_backup( handle1 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    status = moss_avl_backup( handle2 ) ;
    if (status != MAN_C_SUCCESS)
	return( status ) ;

    /*
     * Only equality is supported.
     */

    if ( cmp_operand == CMIS_C_EQUALITY )
    {
	status = moss_asn1_octet_str_cmp( handle1, handle2, cmp_operand ) ;
    }
    else
    {
	return( MAN_C_INVALID_OPERATOR ) ;
    }

    return( status ) ;

}    /* end of moss_dna_id802_snap_cmp() */

/* end of moss_d_table.c */
