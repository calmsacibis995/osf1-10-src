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
/* DEC/CMS REPLACEMENT HISTORY, Element BR_MALLOC.H*/
/* *2     3-MAR-1992 17:12:08 KARDON "UCXed"*/
/* *1    16-SEP-1991 12:48:30 PARMENTER "Memory Management"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BR_MALLOC.H*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FONT_DEFS.H*/
/* *3    25-JAN-1991 16:41:22 FITZELL "V3_EFT_24_JAN"*/
/* *2    12-DEC-1990 12:03:55 FITZELL "V3 IFT Update snapshot"*/
/* *1     8-NOV-1990 11:14:43 FITZELL "V3 IFT"*/
/* DEC/CMS REPLACEMENT HISTORY, Element BKR_FONT_DEFS.H*/

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1990  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use,	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface (bkr)
**
**  ABSTRACT:
**
**	Memory allocation and freeing macros
**
**  AUTHORS:
**
**      F. Klum
**
**  CREATION DATE:     29-Jan-1990
**
**  MODIFICATION HISTORY:
**
**--
**/

#ifndef _BKR_MALLOC_H
#define _BKR_MALLOC_H

/*
 * memory management macros
 */

#ifdef VMS

# define BKR_MALLOC( size ) lib$vm_malloc( ( size ) )

# define BKR_FREE( ptr )   \
  {  	    	    	    \
    lib$vm_free( ( ptr ) ); \
    ( ptr ) = NULL;	    \
  }

# define BKR_CALLOC( num_elem, elem_size ) \
    lib$vm_calloc((num_elem), (elem_size))

# define BKR_CFREE( ptr )    	\
  {  	    	    	    	\
    lib$vm_free( ( ptr ) ); 	\
    ( ptr ) = NULL;	    	\
  }

# define BKR_REALLOC( ptr, size )    \
    lib$vm_realloc( ( ptr ), ( size ) )

#else

# define BKR_MALLOC( size ) malloc( ( size ) )

# define BKR_FREE( ptr )    \
  {  	    	    	    \
    free( ( ptr ) ); 	    \
    ( ptr ) = NULL;	    \
  }

# define BKR_CALLOC( num_elem, elem_size ) \
    calloc((num_elem), (elem_size))

# define BKR_CFREE( ptr )   \
  {  	    	    	    \
    free( ( ptr ) ); 	    \
    ( ptr ) = NULL; 	    \
  }

# define BKR_REALLOC( ptr, size )    \
    realloc( ( ptr ), ( size ) )

#endif 	/* VMS */

#endif /* _BKR_MALLOC_H */

/* DONT ADD STUFF AFTER THIS #endif */

