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
static char *rcsid = "@(#)$RCSfile: moss_oid.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/04/26 20:32:56 $";
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
 * Facility:
 *
 *    Management - POLYCENTER (tm) Common Agent
 *
 * Abstract:
 *
 *    These routines are used to create and manipulate variables that represent
 *    object identifiers.  These routines are:
 *        moss_create_oid()
 *        moss_free_oid()
 *        moss_oid_len()
 *        moss_parse_oid()
 *        moss_compare_oid()
 *        moss_compare_partial_oid()
 *        moss_concat_oids()
 *        moss_text_to_oid()
 *        moss_oid_to_text()
 *        moss_oid_to_octet()
 *        moss_octet_to_oid()
 *	  moss_compare_oid_prefix
 *        moss_oid_append()
 *	  moss_oid_from_buf
 *	  moss_oid_length
 *	  moss_oid_to_buf
 *	  moss_oid_to_flat
 *	  moss_oid_to_mybuf
 *
 * Author:
 *
 *    Oscar Newkerk
 *
 * Date:
 *
 *    November 11, 1989  
 *
 * Revision History :
 *
 *	Uttam Shikarpur 	March 19th 1990
 *
 *	If a NULL was passed as a suffix in moss_oid_to_text, a null
 *	string was appened at the end. Corrected this by using a test.
 *
 *
 *      Miriam Amos Nihart	May 14th 1990
 *
 *      Change the file names to reflect the 14 character restriction.
 *
 *	Miriam Amos Nihart	May 16th 1990
 *
 *	Ifdef the sprintf() calls to account for the System V different
 *	return values.
 *
 *      Wim Colgate, May 30th, 1990.
 *
 *      added routines, moss_oid_to_octet and moss_octet_to_oid for
 *      ease of use.
 *
 *      Wim Colgate, June 11th, 1990.
 *
 *      changed all uses of 'index' with 'oid_index' so as not to confuse lint.
 *
 *      Uttam Shikarpur		June 18th 1990
 *
 *      In the moss_oid_to_text() routine, a value of zero or less in the
 *      count field caused the program to seg. fault.  Added a check and
 *      returned a NULL.
 *
 *      Kathy Faust             June 20th 1990
 *
 *      Modified moss_compare_oid to return MAN_C_EQUAL if both oids are NULL.
 *
 *      Kathy Faust             July 6th 1990
 *
 *      Modified moss_concat_oids to copy non null OID if one OID is NULL.
 *      Modified moss_oid_to_octet and moss_octet_to_oid to check for NULL
 *      oid or octet.
 *
 *      Kathy Faust             July 6th 1990
 *
 *      Modified moss_parse_oid to check for NULL element_number pointer.
 *
 *      Kathy Faust             July 17th 1990
 *
 *      Added moss_compare_partial_oid().
 *
 *      Miriam Amos Nihart      September 25th, 1990.
 *
 *      Fix to moss_concat_oids for dealing with null oids.
 *
 *      Ken Chapman             October 12th, 1990.
 *
 *      Add moss_oid_append.
 *
 *      Miriam Amos Nihart
 *
 *      Standardize the calls to return man_status.
 *
 *      Miriam Amos Nihart	November 14th, 1990.
 *
 *      Fix return code for moss_oid_to_octet().
 *
 *      Miriam Amos Nihart	May 22nd, 1991.
 *
 *      Correct moss_create_oid to allow the creation of null oids.
 *
 *      Miriam Amos Nihart	August 12th, 1991.
 *
 *	Put in parameter check on moss_free_oid.
 *
 *      Miriam Amos Nihart	October 16th, 1991.
 *
 *      Fix for prototyping.
 *
 *      Wim Colgate, November 18th, 1991.
 * 
 *      Changed casting of sprintf - returns an int.
 *
 *      Miriam Amos Nihart, December 2nd, 1991.
 *
 *      Put in cast for return value from sprintf.  This is
 *      to correct for the OS header file error.  This removes
 *      the need for ifdef'ing for SYSV as both routines should
 *      return integer.
 *
 *      Miriam Amos Nihart, December 3rd, 1991.
 *
 *      Put in macros for malloc and free.
 *
 *    Steve Pitcher, Feb 7, 1992
 *	This module no longer needs NIL.H.
 *
 *    Richard J. Bouchard Jr., July 24, 1992 (RJB1218)
 *	Add the following routines:
 *	  moss_compare_oid_prefix
 *        moss_oid_append()
 *	  moss_oid_from_buf
 *	  moss_oid_length
 *	  moss_oid_to_buf
 *	  moss_oid_to_flat
 *	  moss_oid_to_mybuf
 */

/*
 *  Function Prototypes to satisfy c89 -std warnings.
 *
 *   - atoi() is not part of the c89 -std conforming implementation
 *     (#ifdef __STDC__) so it is not defined in <math.h>
 *
 *   - cma_lib_malloc() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace malloc()
 *
 *   - cma_lib_free() has no prototype, it is defined in
 *     /usr/include/dce/cmalib_crtlx.h to replace free()
 */

extern int atoi() ;

#ifndef NOIPC
extern void *cma_lib_malloc() ;
extern void *cma_lib_free() ;
#else
#include <stdlib.h>
#endif /* NOIPC */

/*
 *  Header files
 */

#include "man_data.h"

#include <string.h>
#include <stdio.h>    

#include "man.h"
#include "moss.h"
#include "moss_private.h"

/*
 * Allow use of builtin functions for VMS.  This allows
 * the use of the _PROBEx instructions to check for
 * read and write access to data.
 */
#ifdef VMS
#pragma builtins
#endif

man_status
moss_create_oid(
                count ,
                values ,
                oid
               )  

/*
 *    Function Description:
 *
 *       This function allocates the storage for an object id variable and
 *       stores the values in it.
 *
 *    Arguments:
 *
 *       count    The number of elements in the object identifier.
 *       values   A pointer to an array of long int. The elements of the id.
 *       oid      The address of an object_id pointer
 *
 *
 *    Return Value:
 *
 *       MAN_C_SUCCESS        Successfully created the oid
 *       MAN_C_BAD_PARAMETER  Bad input parameter
 *       MAN_C_FAILURE        Unable to create the oid
 *
 *    Side Effects:
 *
 *       The oid pointer points to null if unable to create an oid.
 *
 *
 */

int count ;
unsigned int *values ;
object_id **oid ;

{
    int temp1 ;
    object_id *dummy_oid ;
    object_id *target_oid ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if( ( oid == NULL ) || ( ( values == NULL ) && ( count != 0 ) ) )
        return( MAN_C_BAD_PARAMETER ) ;

    *oid = ( object_id * )NULL ;

    /*
     *  Prepare the structure and allocate the memory for the elements.
     */

    MOSS_MALLOC( target_oid, object_id, sizeof( object_id ) + sizeof( unsigned int ) * count )

    /*
     *  If the memory could not be allocated, then return an error.
     */

    if ( target_oid == NULL )
        return( MAN_C_FAILURE ) ;
     
    /*
     *  Since we got the memory, let's copy the elements into the structure.
     *  But first set the pointer of the value to the space just following the oid....
     */

    target_oid->count = count ;
    dummy_oid = target_oid ;
    dummy_oid++ ;
    target_oid->value = ( unsigned int * )( dummy_oid ) ; 
    if ( count != 0 )
        for ( temp1 = 0; temp1 != count; temp1++ )
            ( target_oid->value )[ temp1 ] = values[ temp1 ] ;

    /*
     * Now lets return a success status code to the caller.
     */

    *oid = target_oid ;
    return( MAN_C_SUCCESS ) ;
    
}  /* end of moss_create_oid() */

man_status
moss_free_oid(
              oid
             )

/*
 *    Function Description:
 *
 *       This function deallocates the storage associated with the value field 
 *       of a variable of type object_id. The whole thing is freed.
 *
 *    Arguments:
 *
 *       oid      A pointer to a variable of type object_id.
 *
 *    Return Value:
 *
 *       MAN_C_SUCCESS          Oid successfully freed
 *       MAN_C_BAD_PARAMETER    Oid pointer was NULL
 *
 *    Side Effects:
 *
 *    None.
 */

object_id *oid ;

{
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if( oid == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    MOSS_FREE( oid ) ;
    return( MAN_C_SUCCESS ) ;
    
}  /* end of moss_free_oid */

man_status
moss_get_oid_len(
                 oid ,
                 length
                )

/*
 *    Function Description:
 *
 *       This function returns the 'length', that is the number of elements in,
 *       a variable of type object_id.
 *
 *    Arguments:
 *
 *       oid       A pointer to a variable of type object_id.
 *       length    The address of the integer to return the length
 *
 *
 *    Return Value:
 *
 *      MAN_C_SUCCESS         Successfully determined the length
 *      MAN_C_BAD_PARAMETER   Bad input parameter
 *      MAN_C_FAILURE 
 *
 *    Side Effects:
 *
 *      The length of the oid is returned in the length parameter.
 *
 */

object_id *oid ;
int *length ;

{
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( length == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    if ( oid == NULL )
        return( MAN_C_BAD_PARAMETER ) ;

    *length = oid->count ;
    return( MAN_C_SUCCESS ) ;
    
}  /* end of moss_get_oid_len() */

man_status
moss_parse_oid(
               oid ,
               element_number ,
               element
              )
/*
 *    Function Description:
 *
 *       This function returns the elements in the object identifier.
 *
 *    Arguments:
 *
 *       oid              A pointer to a variable of type object_id.
 *       element_number   An integer that is the element number to be returned.
 *       element          The address to return the element to
 *
 *    Return Value:
 *
 *       MAN_C_SUCCESS        Successfully returned the element
 *       MAN_C_BAD_PARAMETER  Bad input parameter
 *       MAN_C_FAILURE        Unable to return the element
 *
 *    Side Effects:
 *
 *       element_number is incremented.
 *       oid element is returned in element
 *
 */

object_id *oid ;
int *element_number ;
int *element ;
    
{
    unsigned int element_value ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( (oid == NULL) || (element_number == NULL) || (element == NULL) )
        return( MAN_C_BAD_PARAMETER ) ;

    /*
     *    If the element number being asked for is larger than the number of elements
     *    in the object identifier, then return a failure.
     */

    if ( *element_number >= oid->count ) 
        return( MAN_C_FAILURE ) ;
    
    /*
     *    Return the value of the element and increment to element number for the next call.
     */
    
    *element = ( oid->value )[ ( *element_number ) ] ;
    ( *element_number )++ ;

    /*
     *    Return success.
     */
    
    return( MAN_C_SUCCESS ) ;

}  /* end of moss_parse_oid() */

man_status
moss_compare_oid(
                 oid1 ,
                 oid2
                )
/*
 *    Function Description:
 *
 *       This function compares two object ids for equality.  To be equal, they must be
 *       the same length (contain the same number of elements) and each element must be the
 *       same.
 *
 *    Arguments:
 *
 *       oid1        A pointer to a variable of type object_id.
 *       oid2        A pointer to the second object identifier.
 *
 *    Return Value:
 *
 *       MAN_C_NOT_EQUAL   if they are not equal.
 *       MAN_C_EQUAL       if they are equal.
 *
 *    Side Effects:
 *
 *       None.
 *
 */

object_id *oid1 ;
object_id *oid2 ;
    
{
    int oid_index ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( (oid1 == NULL) && (oid2 == NULL) )
        return( MAN_C_EQUAL ) ;

    if (oid1 == NULL || oid2 == NULL)
        return( MAN_C_NOT_EQUAL ) ; /*?*/

    if ( oid1->count != oid2->count )
        return( MAN_C_NOT_EQUAL ) ;
    
    for ( oid_index =  0 ; oid_index < oid1->count ; oid_index++ )
        if ( ( oid1->value )[ oid_index ] != ( oid2->value )[ oid_index ] ) 
            return( MAN_C_NOT_EQUAL ) ;
    
    return( MAN_C_EQUAL ) ;
    
}  /* end of moss_compare_oid() */

man_status
moss_compare_partial_oid(
			 oid1 ,
			 oid2 ,
			 oid1_starting_element ,
			 oid2_starting_element ,
			 number_of_elements
			)
/*
 *    Function Description:
 *
 *       This function compares two subsets of two object ids for equality.
 *
 *    Arguments:
 *
 *       oid1                  A pointer to a variable of type object_id.
 *       oid2                  A pointer to the second object identifier.
 *       oid1_starting_element An integer identifying the starting element for oid1
 *       oid2_starting_element An integer identifying the starting element for oid2
 *       number_of_elements    An integer identifying the number of contiguous
 *                             elements to compare
 *
 *    Return Value:
 *
 *       MAN_C_NOT_EQUAL   if they are not equal.
 *       MAN_C_EQUAL       if they are equal.
 *
 *    Side Effects:
 *
 *       None.
 *
 */

object_id *oid1 ;
object_id *oid2 ;
int oid1_starting_element ;
int oid2_starting_element ;
int number_of_elements ;
    
{
    int oid_index ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( (oid1 == NULL) && (oid2 == NULL) )
        return( MAN_C_EQUAL ) ;

    if (oid1 == NULL || oid2 == NULL)
        return( MAN_C_NOT_EQUAL ) ; 

    if ( ( oid1_starting_element + number_of_elements ) > oid1->count )
        return( MAN_C_NOT_EQUAL ) ;

    if ( ( oid2_starting_element + number_of_elements ) > oid2->count )
        return( MAN_C_NOT_EQUAL ) ;
    
    for ( oid_index = 0 ; oid_index < number_of_elements ; oid_index++ )
        if ( ( oid1->value )[ oid1_starting_element + oid_index ] 
	    != 
	    ( oid2->value )[ oid2_starting_element + oid_index ] ) 
            return( MAN_C_NOT_EQUAL ) ;
    
    return( MAN_C_EQUAL ) ;
    
}  /* end of moss_compare_partial_oid() */

man_status
moss_concat_oids(
                 oid1 ,
                 oid2 ,
                 oid3
                )

/*
 *    Function Description:
 *
 *       This function takes two object identifiers and concatinates them together to
 *       produce a third.
 *
 *    Arguments:
 *
 *       oid1    A pointer to a variable of type object_id.
 *       oid2    A pointer to the secon object identifier.
 *       oid3    The address of a pointer to an object identifer.
 *
 *    Return Value:
 *
 *       MAN_C_SUCCESS        Successfully concatenated the object identifiers
 *       MAN_C_BAD_PARAMETER  Bad Input parameter
 *       MAN_C_FAILURE        Unable to concatenate the object identifiers
 *
 *    Side Effects:
 *
 *       The address of the concatenated object identifers is returned in oid3.
 *       If not able to concatenate oid is NULL is returned in oid3.
 *
 */

object_id *oid1 ;
object_id *oid2 ;
object_id **oid3 ;

{
    int total_size ;
    int temp1 = 0 ;
    int temp2 = 0 ;
    object_id *dummy_oid ;
    object_id *tmp_oid ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( oid1 == NULL && oid2 == NULL ) || ( oid3 == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    *oid3 = NULL ;

    if ( oid1 == NULL )
        total_size = oid2->count ;
    else
    if ( oid2 == NULL )
        total_size = oid1->count ;
    else
        total_size = oid1->count + oid2->count ;
    
    /*
     *  Allocate the memory for the elements.
     */

    MOSS_MALLOC( tmp_oid, object_id, sizeof( object_id ) + sizeof( int ) * total_size )
    if ( tmp_oid == NULL )
        return( MAN_C_FAILURE ) ;

    tmp_oid->count = total_size ;
    dummy_oid = tmp_oid ;
    dummy_oid++ ;
    tmp_oid->value = ( unsigned int * )( dummy_oid ) ; 

    /*
     *  Since we got the memory, let's copy the elements into the structure.
     */

    if ( oid1 != NULL )
        for ( ; temp1 < oid1->count ; temp1++  )
            ( tmp_oid->value )[ temp1 ] = ( oid1->value )[ temp1 ] ;

    if ( oid2 != NULL )
        for ( ; temp1 < total_size ; temp1++, temp2++ )
            ( tmp_oid->value )[ temp1 ] = ( oid2->value )[ temp2 ] ;
        
    *oid3 = tmp_oid ;
    return( MAN_C_SUCCESS ) ;
    
}  /* end of moss_concat_oids() */

man_status
moss_text_to_oid(
                 text ,
                 oid
                )

/*
 *
 * Function description:
 *
 *    This routine takes the latin1 sting and
 *    translate it into the object class structure.
 *
 * Arguments:
 *
 *    text     The string to be converted.
 *    oid      The address of a pointer to an object identifier
 *
 * Return value:
 *
 *    MAN_C_SUCCESS        Successfully created the object identifier
 *    MAN_C_BAD_PARAMETER  Bad Input parameter
 *    MAN_C_FAILURE        Unable to create the object identifier
 *
 * Side effects:
 *
 *    The address of the created oid is returned in the parameter oid.
 */

char *text ;
object_id **oid ;

{
    char *start_p = text ;
    char *temp = 0 ;
    int temp1 = 0 ;    
    object_id *dummy_oid ;
    object_id *tmp_oid ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( text == NULL ) || ( oid == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    *oid = NULL ;

    /*
     *  Walk through the string counting separators to get the length of the
     *  object class id.
     */

    temp = strpbrk( start_p, "0123456789" ) ;
    while ( temp != 0 )
    {
       temp1++ ;
       temp = temp + strspn( temp, "0123456789" ) ;
       if ( *temp == 0 )
           break ;
       temp = strpbrk( temp, "0123456789" ) ;
    } ;

    /* 
     *  Prepare the structure and allocate the memory for the elements. 
     */

    MOSS_MALLOC( tmp_oid, object_id, sizeof( object_id ) + sizeof( int ) * temp1 )

    /*
     *  If the memory could not be allocated, then return an error.
     */

    if ( tmp_oid == NULL )
        return( MAN_C_FAILURE ) ;
     
    tmp_oid->count = temp1 ;
    dummy_oid = tmp_oid ;
    dummy_oid++ ;
    tmp_oid->value = ( unsigned int * )( dummy_oid ) ; 

    /*
     *  Since we got the memory, let's copy the elements into the structure. 
     */

    temp = strpbrk( start_p, "0123456789" ) ;
    temp1 = 0 ;
    while ( temp != 0 )
    {
        ( tmp_oid->value )[ temp1 ] = atoi( temp ) ;
        temp = temp + strspn( temp, "0123456789" ) ;
        temp = strpbrk( temp, "0123456789" ) ;
        temp1++ ;
    }

    *oid = tmp_oid ;
    return( MAN_C_SUCCESS ) ;
        
}     /* end of moss_text_to_oid() */

man_status
moss_oid_to_text(
                 oid ,
                 sep ,
                 prefix ,
                 suffix ,
                 text
                )

/*
 *
 * Function description:
 *
 *    This routine takes an object identifier and returns a null terminated
 *    string representation of the object id.
 *    
 * Arguments:
 *
 *    oid       A pointer to an object_id variable.
 *    sep       A pointer to a NULL terminated string that is the separator for the elements.
 *    prefix    A pointer to a NULL terminated string that is the prefix of the string.
 *    suffix    A pointer to a NULL terminated string that is the suffix for the string.
 *    text      The address of the pointer to a character string
 *
 * Return value:
 *
 *    MAN_C_SUCCESS        Successfully created the textual representation
 *    MAN_C_BAD_PARAMETER  Bad Input parameter
 *    MAN_C_FAILURE        Unable to create the textual representation
 *
 * Side effects:
 *
 *    The address of the returned textual representation is returned in
 *    the parameter text.
 *
 */

object_id *oid ;
char *sep ;
char *prefix ;
char *suffix ;
char **text ;

{
    char *temp ;
    int junk ;
    char *tmp_text ;
    char *default_sep = "." ;
    int len ;
    int oid_index ;
    int num_char ;
    man_status return_status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */


    if ( ( oid == NULL ) || ( text == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    *text = NULL ;

    /* 
     * Set the default separator if none is specified
     */
                                                          
    if ( sep == NULL || strlen( sep ) == 0 )
        sep = default_sep ;

    /*
     * First let's find out how many elements are in the oid.
     */
    
    return_status = moss_get_oid_len( oid, &len ) ;

    if ( ( return_status != MAN_C_SUCCESS ) || ( len <= 0 ) )
        return( MAN_C_BAD_PARAMETER ) ;    
    /*
     *  Now lets allocate the space for the printable version using:
     *
     *    (# of elements * 10 char) + length of the prefix + length of the suffix +
     *    (# of elements * length of the separator)
     *
     *  This is likely to be too much but, so what.
     */
    
    num_char = ( sizeof( char ) * ( len * 10 ) ) +
               ( prefix == NULL ? 0 : strlen( prefix ) ) +
               ( suffix == NULL ? 0 : strlen( suffix ) ) +
               ( len * ( sep == NULL ? 0 : strlen( sep ) ) ) ;
    MOSS_MALLOC( tmp_text, char, num_char )
    memset( tmp_text, '\0', num_char ) ;
    
    /*
     *  If we didn't get the memory, then return a error.
     */
    
    if ( tmp_text == 0 ) 
        return( MAN_C_FAILURE ) ;
    
    /*
     *  Copy the prefix string into the output buffer and then find the terminating null.
     */
    
    temp = strcpy( tmp_text, ( prefix == NULL ? "" : prefix ) ) ;
    temp = tmp_text + strlen( tmp_text ) ;
    
    /*
     *  Now let's loop through the elemenst in the oid, using sprintf to format them
     *  and the separator and add them to the buffer.
     */
    
    for ( oid_index = 0 ; oid_index < ( oid->count - 1 ) ; oid_index++ )
    {
        junk = ( int )sprintf( temp, "%d%s", ( oid->value )[ oid_index ], sep ) ;
        temp = tmp_text + strlen( tmp_text ) ;
    } ;
    
    /*
     *  Now add the last element and finish with the suffix.
     */

    junk = ( int )( suffix == NULL ? 
	          ( sprintf( temp, "%d", ( oid->value )[ oid_index ] ) ) :	    
	          ( sprintf( temp, "%d%s", ( oid->value )[ oid_index ], suffix ) ) ) ;
    
    *text = tmp_text ;
    return( MAN_C_SUCCESS ) ;

}  /* end of moss_oid_to_text() */

man_status 
moss_oid_to_octet(
                  oid ,
                  octet
                 )

/*
 *
 * Function description:
 *
 *    This routine takes an object id and fills in the proper info to 
 *    an octet_string pointer (that already exists). Its primary function is
 *    to add an object id directly into an octet string representation for an
 *    avl add.
 *
 * Arguments:
 *
 *    oid       A pointer to an object_id variable.
 *    octet     A pointer to an octet string variable.
 *
 * Return value:
 *
 *    MAN_C_SUCESS
 *    MAN_C_BAD_PARAMETER
 *
 * Side effects:
 *
 *    None
 *
 */

object_id *oid ;
octet_string *octet ;

{
#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( octet == NULL ) || ( oid == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;
    
    octet->length = oid->count * sizeof( int ) ;
    octet->data_type = ASN1_C_OBJECT_ID ;
    octet->string = (char *)oid->value ;

    return( MAN_C_SUCCESS ) ;

} /* end of moss_oid_to_octet */

man_status
moss_octet_to_oid(
                  octet ,
                  oid
                 )

/*
 *
 * Function description:
 *
 *    This routine takes an octet string (of an object id) and returns a newly
 *    created object id. The storage is permanent, so it must be freed at some
 *    other time with moss_free_oid().
 *
 * Arguments:
 *
 *    octet     A pointer to an octet string variable.
 *    oid       The address of a pointer to an object identifier
 *
 * Return value:
 *
 *    MAN_C_SUCCESS        Successfully created the object id
 *    MAN_C_BAD_PARAMETER  Bad Input parameter
 *    MAN_C_FAILURE        Unable to create object id
 *
 * Side effects:
 *
 *    The address of the new object identifier is returned in the parameter oid
 *
 */

octet_string *octet ;
object_id **oid ;

{
    object_id *new_oid ;
    man_status return_status ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    if ( ( octet == NULL ) || ( oid == NULL ) )
        return( MAN_C_BAD_PARAMETER ) ;

    *oid = NULL ;

    if ( octet->length == 0 )
        return( MAN_C_BAD_PARAMETER ) ;

    return_status = moss_create_oid( octet->length / sizeof( int ) ,
                                     ( unsigned int * )octet->string ,
                                     &new_oid ) ;
    *oid = new_oid ;
    return( return_status ) ;
    
} /* end of moss_octet_to_oid */

man_status
moss_oid_append (
		 oid1 ,
		 value ,
                 oid2
		 )
object_id *oid1 ;
int value ;
object_id **oid2 ;

/*
 * Function description:
 *
 *    This routine appends a value to an object_id data type.
 *
 * Arguments:
 *
 *    oid1       the pointer to the object identifier to be extended
 *    value      the value to be appended
 *    oid2       the address of a pointer to an object identifier
 *
 * Return value:
 *
 *    MAN_C_SUCCESS        Successfully appended value to object identifier
 *    MAN_C_BAD_PARAMETER  Bad input parameter
 *    MAN_C_FAILURE        Unable to append value
 *
 * Side effects:
 *
 *    The address of the new object identifier is pass out in the parameter
 *    oid2.
 */

{
    int         n ;
    object_id  *new_oid ;
    object_id  *dummy_oid ;

#ifdef RPCV2
    MOSS_INIT_CHECK()
#endif /* RPCV2 */

    /*
     *  Check for bad input parameter first.
     */

    if ( ( oid1 == NULL ) || ( oid2 == NULL ) )
        return ( MAN_C_BAD_PARAMETER ) ;

    *oid2 = NULL ;
     
    /*
     *  Prepare the structure and allocate the memory for the elements.
     */

    MOSS_MALLOC( new_oid, object_id, sizeof ( object_id ) + sizeof ( unsigned int ) * ( oid1->count + 1 ) )

    /*
     *  If the memory could not be allocated, then return an error.  Also return
     *  an error if the oid to append is NULL.
     */

    if ( new_oid == NULL )
        return ( MAN_C_FAILURE ) ;
     
    /*
     *  Since we got the memory, let's copy the elements into the structure.
     *  But first set the pointer of the value to the space just following the oid...
     */

    new_oid->count = oid1->count + 1 ;
    dummy_oid = new_oid ;
    dummy_oid++ ;
    new_oid->value = ( unsigned int * ) ( dummy_oid ) ; 
    for ( n = 0; n != oid1->count; n++ )
    {
	( new_oid->value ) [ n ] = ( oid1->value ) [ n ] ;
    }
    ( new_oid->value ) [ n ] = value ;
    
    /*
     * Now lets return the new oid to the caller.
     */

    *oid2 = new_oid ;
    return ( MAN_C_SUCCESS ) ;

} /* end of moss_oid_append() */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	moss_compare_oid_prefix
**
**      This routine compares a partial oid to the prefix of a complete oid.
**	Both of these oids are compared starting at the first element.
**
**  FORMAL PARAMETERS:
**
**      oid1		A pointer to the structure object_id.
**      oid2		A pointer to the structure object_id.
**
**  RETURN VALUE:
**
**	MAN_C_EQUAL		oid2 is equal to the prefix of oid1.
**	MAN_C_NOT_EQUAL		oid2 is not equal to the prefix of oid1.
**--
*/   
man_status moss_compare_oid_prefix(oid1,
				   oid2)

object_id *oid1;
object_id *oid2;

{
    man_status	status;

    status = moss_compare_partial_oid(oid1, oid2, 0,0, oid2->count);

    return status;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	moss_oid_from_buf
**
**      This routine creates a OID from a flat representation.  This is
**	the opposite of moss_oid_to_buf.
**
**  FORMAL PARAMETERS:
**
**      user_oid	A pointer to the OID.
**	buffer		The address of the flat OID.
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		    Normal successful completion.
**	MAN_C_BAD_PARAMETER	    The OID or len argument was NULL
**	MAN_C_PROCESSING_FAILURE    Error on memory allocation
**--
*/   
man_status moss_oid_from_buf(user_oid,
			     buffer)

object_id **user_oid;
char *buffer;

{
    if ((user_oid == NULL) || (buffer == NULL))
	return MAN_C_BAD_PARAMETER;

    *user_oid = (object_id *) malloc(sizeof(object_id));
    if (*user_oid == NULL)
	return MAN_C_PROCESSING_FAILURE;

    (*user_oid)->count = * (int *)buffer;
    (*user_oid)->value = (unsigned int *) malloc(sizeof(int) * (*user_oid)->count);
    if ((*user_oid)->value == NULL)
    {
	free(*user_oid);
	return MAN_C_PROCESSING_FAILURE;
    }

    memcpy((*user_oid)->value, buffer + sizeof(int), (*user_oid)->count * sizeof(int));

    return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	moss_oid_length
**
**      This routine calculates the length of the buffer for the
**	flat representation of an AVL.
**
**  FORMAL PARAMETERS:
**
**      user_oid	A pointer to the original OID.
**	len		(output) The length of the OID buffer
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS	    Normal successful completion.
**	MAN_C_BAD_PARAMETER The OID or len argument was NULL
**
**--
*/   
man_status moss_oid_length(user_oid,
			   len)

object_id *user_oid;
int *len;

{
    if ((user_oid == NULL) || (len == NULL))
	return MAN_C_BAD_PARAMETER;

    *len = sizeof(user_oid->count) + user_oid->count * sizeof(int);    

    return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	moss_oid_to_buf
**
**      This routine creates a flat representation of an OID.
**
**  FORMAL PARAMETERS:
**
**      user_oid	A pointer to the OID.
**	buffer		The address to store a pointer to the flat OID.
**			If not NULL, then the buffer is re-used.
**	len		(Output) length of the flat OID
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		    Normal successful completion.
**	MAN_C_BAD_PARAMETER	    The OID or len argument was NULL,
**				    or the supplied buffer was not large enough
**	MAN_C_PROCESSING_FAILURE    Error on memory allocation
**--
*/   
man_status moss_oid_to_buf(user_oid,
			   buffer,
			   len)

object_id *user_oid;
char **buffer;
int *len;

{
    int	len_required;

    if ((user_oid == NULL) || (len == NULL) || (buffer == NULL))
	return MAN_C_BAD_PARAMETER;

    moss_oid_length(user_oid, &len_required);

    /*
     * Allocate buffer 
     */
    *buffer = (char *)  malloc(len_required);
    if (*buffer == NULL)
        return MAN_C_PROCESSING_FAILURE;

    *(int *)(*buffer) = user_oid->count;
    memcpy (*buffer + sizeof(int), user_oid->value, user_oid->count * sizeof(int));

    *len = len_required;

    return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	moss_oid_to_flat
**
**      This routine converts an Object Identifier (OID)
**	to a flat buffer representation.  This flat buffer
**	is itself also an OID.
**
**	This routine also determines if the OID can be
**	properly accessed in the specified access mode.
**
**      Note that this is an internal-only routine.
**
**  FORMAL PARAMETERS:
**
**      user_oid	A pointer to the original OID.
**	flat_oid	A pointer to the buffer for the flat OID
**	len		(input/output) The length of the flat OID buffer
**	probe		Boolean.  TRUE is the OID should be PROBEd
**	acmode		Access Mode to perform the PROBE in.  Used only
**			if "probe" is TRUE.  Valid values:
**			    PSL$C_KERNEL
**			    PSL$C_EXEC
**			    PSL$C_SUPER
**			    PSL$C_USER
**			On VMS these are defined in PSLDEF.H.
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		Normal successful completion.
**      MAN_C_ACCESS_VIOLATION  The OID cannot be read.
**	MAN_C_BAD_PARAMETER	The return buffer is too short.
**
**  DESIGN:
**
**--
*/   
man_status moss_oid_to_flat(user_oid,
			    flat_oid,
			    len,
			    probe,
			    acmode)

object_id *user_oid;
char *flat_oid;
int *len;
int probe;
int acmode;

{
    int		count;
    unsigned int	*value;

#ifdef VMS
    if (probe)
    {
	/*
	 * Can OID pointer be read?
	 */
	if (_PROBER(acmode, sizeof(object_id *), user_oid) == FALSE)
	    return (man_status) MAN_C_ACCESS_VIOLATION;

	/*
	 * Can OID be read?
	 */
	if (_PROBER(acmode, sizeof(object_id), &(user_oid->count)) == FALSE)
	    return (man_status) MAN_C_ACCESS_VIOLATION;
    }
#endif

    /*
     * Store "count" for safekeeping.
     */
    count = user_oid->count;

    /*
     * Is the return buffer long enough?
     */
    if (*len <  sizeof(object_id) + count * sizeof(int))
	return MAN_C_BAD_PARAMETER;

    /*
     * Store a pointer to the OID value buffer for safekeeping
     */
    value = user_oid->value;

#ifdef VMS
    if (probe)
        /*
	 * Skip probing if oid has a zero length.
	 */
	if (count != 0)
	    /*
	     * Can OID data be read?
	     */
	    if (_PROBER(acmode, sizeof(unsigned int) * count, value) == FALSE)
	        return (man_status) MAN_C_ACCESS_VIOLATION;
#endif

    /*
     * Store object_id header in flat oid.
     * Note that the "value" definition is non-portable due to the assignment
     * of pointers of different types to each other.
     */
    ((object_id *)flat_oid)->count = count;
    ((object_id *)flat_oid)->value = (unsigned int *) ((char *)(flat_oid) + sizeof(object_id));

    /*
     * Copy data from object_id, and return the length of the flat OID.
     */
    memcpy(flat_oid + sizeof(object_id), value, sizeof(unsigned int) * count);

    *len = sizeof(object_id) + sizeof(unsigned int) * count;

    return MAN_C_SUCCESS;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	moss_oid_to_mybuf
**
**      This routine creates a flat representation of an OID.
**
**  FORMAL PARAMETERS:
**
**      user_oid	A pointer to the OID.
**	buffer		The address of thge buffer in which to place
**			the flat representation of the AVL
**	len		Length of the buffer
**
**  RETURN VALUE:
**
**	MAN_C_SUCCESS		    Normal successful completion.
**	MAN_C_BAD_PARAMETER	    The OID or len argument was NULL,
**				    or the supplied buffer was not large enough
**--
*/   
man_status moss_oid_to_mybuf(user_oid,
			     buffer,
			     len)

object_id *user_oid;
char *buffer;
int len;

{
    int		len_required;
    man_status	status;

    if ((user_oid == NULL) || (buffer == NULL))
	return MAN_C_BAD_PARAMETER;

    status = moss_oid_length(user_oid, &len_required);
    if (status != MAN_C_SUCCESS)
	return status;

    if (len < len_required)
        return MAN_C_BAD_PARAMETER;

    *(int *)(buffer) = user_oid->count;
    memcpy (buffer + sizeof(int), user_oid->value, user_oid->count * sizeof(int));

    return MAN_C_SUCCESS;
}

/* end of moss_oid.c */
