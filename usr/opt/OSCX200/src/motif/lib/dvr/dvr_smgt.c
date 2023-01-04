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
#define Module DVR_SMGT
#define Ident  "V02-013"

/*
**++
**   COPYRIGHT (c) 1989, 1992 BY
**   DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**   ALL RIGHTS RESERVED.
**
**   THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**   ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**   INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**   COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**   OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**   TRANSFERRED.
**
**   THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**   AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**   CORPORATION.
**
**    DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**    SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**
** MODULE NAME:
**	dvr_struct_mgmt.c (vms)
**	dvr_struct_mgmt.c (ultrix)
**	dvr_smgt.c	  (os/2)
**
** FACILITY:
**
**	Compound Document Architecture (CDA)
**	Compound Document Viewers
**	DIGITAL Document Interchange Format (DDIF)
**
** ABSTRACT:
**	Structure and memory management routines.
**
** ENVIORNMENT:
**	vms, ultrix, os/2
**
** AUTHORS:
**	Marc A. Carignan,  30-Nov-1988
**
**
** MODIFIED BY:
**
**	V02-001		PBD0001		Peter B. Derr		1-Mar-1989
**		Use new flattened include files
**	V02-002		DAM0002		Dennis McEvoy		05-mar-1990
**		changes for os/2 port
**	V02-003		DAM0003		Dennis McEvoy		05-apr-1990
**		use cda_malloc/cda_free on all platforms
**	V02-004		SJM0000		Stephen Munyan		28-Jun-1990
**		Conversion to Motif
**	V02-005		RTG0001		Dick Gumbel		05-Mar-1991
**		Cleanup #include's
**	V02-006		DAM0001		Dennis McEvoy		03-apr-1991
**		Cleanup typedefs
**	V02-007		RAM0001		Ralph A. Mack		24-Apr-1991
**		Add #ifdef's for MS-Windows
**      V02-008	        DAM0001		Dennis McEvoy		06-may-1991
**		move dvr_int include to cleanup ultrix build
**      V02-009	        DAM0001		Dennis McEvoy		05-aug-1991
**		renamed headers, removed dollar signs
**      V02-010	        DAM0001		Dennis McEvoy		18-oct-1991
**		more cleanups to match protos
**      V02-011	        ECR0001		Elizabeth C. Rust	30-Mar-1992
**		merge in audio code
**      V02-012	        KLM0001		Kevin McBride		12-Jun-1992
**		Alpha OSF/1 port
**
**	V02-013		RDH003		Don Haney		15-Jul-1992
**		Specify same-directory includes with "", not <>
**--
*/

/*
 *  Include files
 */
#include <cdatrans.h>

#ifdef __vms__

#include <dvrint.h>				/* DVR internal definitions */

#pragma nostandard				/* turn off /stand=port for
						   "unclean" X include files */

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#pragma standard				/* turn /stand=port back on */

#endif

#ifdef OS2

#define INCL_PM 				/* tell OS/2 to include Presentation Manager defs */
#define PMMLE_INCLUDED                          /* do not include multi-line editing defs */
#include <os2.h>                                /* OS/2 defs */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef MSWINDOWS
#define NOKERNEL				/* Omit unused definitions */
#define NOUSER					/* from parse of windows.h */
#define NOMETAFILE				/* to save memory on compile. */
#include <windows.h>				/* MS-Windows definitions. */

#include <dvrint.h>				/* DVR internal definitions */
#endif

#ifdef __unix__

#include <Xm/Xm.h>				/* Motif definitions */
#include <Mrm/MrmAppl.h>			/* Motif Resource Manager defintions */

#include <dvrint.h>				/* DVR internal definitions */

#endif

#include "dvrwdef.h"				/* Public Defns w/dvr_include */
#include "dvrwint.h"				/* Viewer Constants and structures  */
#include "dvrwptp.h"				/* dvr windowing prototypes */

/* local routine prototypes */

PROTO(CDAstatus dvr_dealloc_struct,
		(DvrViewerWidget,
		 void * *) );

PROTO( CDAstatus dvr_dealloc_path_structs,
		(DvrViewerWidget) );



/*
 *++
 *
 *  FUNCTION NAME:
 *
 *      dvr_alloc_memory
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Allocates dynamic memory of specified byte size and returns
 *	memory address; memory is cleared before return.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	alloc_size	- Number of bytes to allocate
 *	memory_addr	- Return address of allocated memory
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_MEMALLOFAIL Memory allocation failure
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_alloc_memory (
    vw,
    alloc_size,
    memory_addr )

DvrViewerWidget	vw;		/* Viewer widget context pointer */
CDAsize		alloc_size;	/* Number of bytes to allocate */
CDAaddress	*memory_addr;	/* Return address of allocated memory */

{
    /* Local variables */
    CDAstatus status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( memory_addr == NULL ))
	return DVR_BADPARAM;

    if ( alloc_size <= 0 ) {
	/* Memory allocation failure */
	return DVR_MEMALLOFAIL;
    }

    /*
     *  Allocate memory.
     */

    /* Alternate memory allocation calls could be entered here */

    /* Use default memory routine */
    status = DVR_ALLOCATE_MEMORY (
	alloc_size,
	memory_addr);

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Clear memory */
    (void) memset ( *memory_addr, 0, (unsigned int) alloc_size );

    /* Successful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_alloc_struct
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Allocates memory for specified structure.  Return parameter,
 *	struct_ref, is returned as an integer pointer, which should be cast
 *	to the appropriate pointer type, and expected by 'reference' (and
 *	therefore declared here as **struct_ref); data passed in should be
 *	the address of a pointer object, which can be casted as needed upon
 *	return by the caller.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	struct_type	- Structure type to allocate
 *	struct_ref	- Return structure reference
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *  	DVR_BADPARAM	Bad parameter
 *	DVR_UNKSTRTYPE Unknown structure type
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_alloc_struct (
    vw,
    struct_type,
    struct_ref )

DvrViewerWidget	vw;		/* Viewer widget context pointer */
enum str_type	struct_type;	/* Struct type */
CDAaddress	*struct_ref;	/* Return structure reference */

{
    /* Local variables */
    DvrStruct	*dvr_struct;		/* DVR struct pointer */
    CDAaddress	memory_addr;		/* Allocated memory address */
    CDAsize	struct_size;		/* Structure size */
    int		store_size = TRUE;	/* Store size in structure case as 'str' */
    CDAstatus	status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( struct_ref == NULL ))
	return DVR_BADPARAM;

    /*
     *	Allocate structure indicated by structure type parameter, and
     *	add structure to allocated structure list for proper memory
     *	management.
     */

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */

    /* Determine size of structure to allocate */
    switch ( struct_type ) {

    case pag_private :
	/* Per page private structure */
	struct_size = sizeof( struct pag_private );
	break;

    case obj_private :
     	/* Per object private structure */
	struct_size = sizeof( struct obj_private );
	break;

    case frm_private :
     	/* Frame private structure */
	struct_size = sizeof( struct frm_private );
	break;

    case trn_private :
	/* Transform private structure */
	struct_size = sizeof( struct trn_private );
	break;

    case gly_private :
     	/* Galley private structure */
	struct_size = sizeof( struct gly_private );
	break;

    case txt_private :
	/* Text line private structure */
	struct_size = sizeof( struct txt_private );
	break;

    case frg_private :
	/* Text fragment private structure */
	struct_size = sizeof( struct frg_private );
	break;

    case lin_private :
     	/* Polyline private structure */
	struct_size = sizeof( struct lin_private );
	break;

    case crv_private :
	/* Bezier curve private structure */
    	struct_size = sizeof( struct crv_private );
	break;

    case arc_private :
	/* Arc private structure */
	struct_size = sizeof( struct arc_private );
	break;

    case fil_private :
     	/* Fill private structure */
	struct_size = sizeof( struct fil_private );
	break;

    case img_private :
	/* Image private structure */
	struct_size = sizeof( struct img_private );
	break;

    case img_list_struct :
	/* Image list structure */
	struct_size = sizeof( struct img_list_struct );
	break;

/*BEGIN AUDIO STUFF*/
#ifdef CDA_AUDIO_SUPPORT
    case aud_private :
	/* Audio private structure */
	struct_size = sizeof( struct aud_private );
	break;
#endif
/*END AUDIO STUFF*/
    default:
	/* Unknown structure type */
	return DVR_UNKSTRTYPE;

    }

    /* Allocate memory, which is cleared during call */
    status = dvr_alloc_memory (
	vw,
	struct_size,
	&memory_addr );

    if ( DVR_FAILURE ( status )) {
    	/* Unexpected error */
	return status;
    }

    /* Store allocation size in structure if flag is on */
    if ( store_size ) {
	/* Store allocation size in structure case as 'str' */
    	((STR) memory_addr)->str_length = struct_size;

	/* Link structure into private allocation list */
	QUE_INSERT ( dvr_struct->private_structs, memory_addr )
    }

    /* Successful allocation; return memory address param */
    *struct_ref = memory_addr;

    /* Succesful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_dealloc_memory
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Deallocates dynamic memory (of specified byte size).
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	dealloc_size	- Number of bytes to deallocate (allocated)
 *	memory_addr	- Address of allocated memory (pass pointer by value)
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *	DVR_MEMDEALLOFAIL Memory deallocation failure
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_dealloc_memory (
    vw,
    dealloc_size,
    memory_addr )

DvrViewerWidget	vw;		/* Viewer widget context pointer */
CDAsize		dealloc_size;	/* Number of bytes to deallocate */
CDAaddress	*memory_addr;	/* Address of allocated memory */

{
    /* Local variables */
    CDAstatus status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( memory_addr == NULL ))
	return DVR_BADPARAM;

    if ( dealloc_size <= 0 ) {
	/* Memory deallocation failure */
	return DVR_MEMDEALLOFAIL;
    }

    /*
     *  Deallocate memory.
     */

    /* Alternate memory deallocation calls could be entered here */

    /* Use default memory routine */
    status = DVR_DEALLOCATE_MEMORY (
	dealloc_size,
	memory_addr);

    if ( DVR_FAILURE ( status )) {
	/* Unexpected error */
	return status;
    }

    /* Successful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_dealloc_struct
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Deallocates structure memory; routine casts structure as an 'str'
 *	and uses the structure length stored here for deallocation.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *	struct_ref	- Structure reference (pointer by value)
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_dealloc_struct (
    vw,
    struct_ref )

DvrViewerWidget	vw;		/* Viewer widget context pointer */
CDAaddress	*struct_ref;	/* Structure reference */

{
    /* Local variables */
    CDAaddress	struct_ptr;	/* Structure pointer */
    CDAsize	struct_size;	/* Structure size */
    CDAstatus	status;


    /* Check parameter validity */
    if (( vw == NULL ) || ( struct_ref == NULL ))
	return DVR_BADPARAM;

    /*
     *	Deallocate structure using allocation length stored in structure
     *  in standard 'str' format.
     */

    /* Dereference structure reference pointer */
    struct_ptr = (void *) *struct_ref;

    /* Get structure size */
    struct_size = ((STR) struct_ptr)->str_length;

    /* Check length validity */
    if ( struct_size <= 0 ) return DVR_MEMDEALLOFAIL;

    /* Deallocate memory */
    status = dvr_dealloc_memory (
	vw,
	struct_size,
	struct_ref );

    if ( DVR_FAILURE ( status )) {
    	/* Unexpected error */
	return status;
    }

    /* Succesful completion */
    return DVR_NORMAL;
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *	dvr_free_file_memory
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Frees (deallocates) all memory in use for the current file, including
 *	the layout engine page structures, the private page structures, the
 *	private object structures, any private specific object structures,
 *	and other allocated memory such as special path elements.  Routine
 *	should be called as part of the close file operation.
 *
 *  FORMAL PARAMETERS:
 *
 *	vw		- Viewer widget context structure
 *
 *  FUNCTION VALUE:
 *
 *    	DVR_NORMAL	Normal successful completion
 *	DVR_BADPARAM	Bad parameter
 *      Error status as returned by other called procedures.
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */


CDAstatus dvr_free_file_memory (
    vw )

DvrViewerWidget vw;		/* Viewer widget context pointer */

{
    /* Local variables */
    DvrStruct	*dvr_struct;		/* DVR struct pointer */
    OBJ		object;			/* Object structure */
    ENG		engine_context;		/* Layout engine context */
    CDAstatus	last_bad_status;	/* Last bad status, if any */
    CDAstatus	status;


    /* Check parameter validity */
    if ( vw == NULL ) return DVR_BADPARAM;

    dvr_struct = (DvrStruct *) &( vw->dvr_viewer.Dvr );	/* Init */
    engine_context = dvr_struct->engine_context;
    last_bad_status = DVR_NORMAL;


    /*
     *	Traverse through private structure list deallocating these structures
     *	one by one, if any.
     */

    FOREACH_POPLIST ( dvr_struct->private_structs, object, OBJ ) {

	status = dvr_dealloc_struct ( vw, (void **) &object );

	if ( DVR_FAILURE (status)) {
	    /* Save last bad status */
	    last_bad_status = status;
        }
    }


    /*
     *	Traverse page list freeing layout engine page structures, if any.
     */

    FOREACH_POPLIST ( dvr_struct->page_list, object, OBJ ) {

	status = dvs_delete_page ( &engine_context, (PAG *) &object );

	if ( DVR_FAILURE (status)) {
	    /* Save last bad status */
	    last_bad_status = status;
        }
    }


    /*
     *	Deallocate specially allocated path elements.
     */

    status = dvr_dealloc_path_structs ( vw );

    if ( DVR_FAILURE (status)) {
	/* Save last bad status */
	last_bad_status = status;
    }


    /*
     *	Final error processing before return.
     */

    if ( DVR_FAILURE (last_bad_status)) {
        /* Report recoverable error */
        (void) dvr_error_callback ( vw, 0L, DVR_MEMDEALLOFAIL, 0, 0L );
    }

    /* Successful completion; no need returning error; handled here */
    return DVR_NORMAL;
}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Deallocate pth structures created when paths were created from frame
**	bounding boxes.  PBD
**
**  FORMAL PARAMETERS:
**
**      vw : viewer widget (context structure pointer)
**
**  COMPLETION CODES:
**
**      DVR_NORMAL or memory deallocation failure status codes
**
**  SIDE EFFECTS:
**
**      pth structures referenced in the array
**	vw->dvr_viewer.Dvr.allocated_pth_structs are deallocated.
**
**--
**/

CDAstatus	dvr_dealloc_path_structs(vw)
DvrViewerWidget	vw;
{
    CDAcount	counter;
    CDAstatus	status = DVR_NORMAL, new_status;
    PTH		*list_place;
    CDAsize	dealloc_size;

    if ((vw->dvr_viewer.Dvr.allocated_pth_structs == NULL)
	|| (vw->dvr_viewer.Dvr.alloc_pth_struct_array_size == 0))
	return(DVR_NORMAL);	/* no pth structs to deallocate */

    list_place = vw->dvr_viewer.Dvr.allocated_pth_structs;
    for (counter = 0;
	 counter < vw->dvr_viewer.Dvr.alloc_pth_struct_count;
	 counter++)
	{
	    /*
	    **	NOTE: list_place is NOT passed by reference to
	    **	DVR_DEALLOCATE_MEMORY on purpose because list_place is a pointer
	    **	into the array of PTH pointers to the pth structure that is to
	    **	be deallocated.  The DVR_DEALLOCATE_MEMORY will correctly
	    **	de-reference it, or pass the PTH to lib$free_vm by reference.
	    */
	    new_status =
			DVR_DEALLOCATE_MEMORY((*list_place)->pth_str.str_length,
					       list_place);
	    list_place++;

	    if (DVRSuccess(status)) /* preserve non-success status codes */
		status = new_status;
	}
    dealloc_size =
		 vw->dvr_viewer.Dvr.alloc_pth_struct_array_size * sizeof(PTH *);
    new_status = DVR_DEALLOCATE_MEMORY(dealloc_size,
				      &vw->dvr_viewer.Dvr.allocated_pth_structs
				      );

    vw->dvr_viewer.Dvr.alloc_pth_struct_array_size = 0;
    vw->dvr_viewer.Dvr.allocated_pth_structs = NULL;
    vw->dvr_viewer.Dvr.alloc_pth_struct_count = 0;

    if (DVRSuccess(status)) /* preserve non-success status codes */
	status = new_status;
    return(status);
}

/*
 *++
 *
 *  FUNCTION NAME:
 *
 *      dvr_realloc_memory
 *
 *  FUNCTIONAL DESCRIPTION:
 *
 *      Reallocates dynamic memory by increasing the size of the specified
 *	memory region by a specified increment, and copying the contents of
 *	the original memory structure into that of the newly allocated
 *	structure.  The new structure is set to null before
 *	the data is copied.  Both the base address and byte size are passed
 *	by reference, thus allowing this routine to replace the old values
 *	with the newly created values; increment is passed by value.
 *
 *  FORMAL PARAMETERS:
 *
 *      base_addr		- Memory region base address (generic pointer)
 *      byte_size		- Size of memory region in bytes
 *	increment		- Increment value in bytes
 *
 *  IMPLICIT INPUTS:
 *      None.
 *
 *  IMPLICIT OUTPUTS:
 *      None.
 *
 *  FUNCTION VALUE:
 *
 *      DVR_NORMAL		Normal successful completion
 *	DVR_MEMALLOCFAIL	Memory allocation failure
 *	DVR_MEMDEALLOCFAIL	Memory deallocation failure
 *
 *  SIDE EFFECTS:
 *      None.
 *
 *--
 */

CDAstatus realloc_memory(base_addr,byte_size,increment )

    CDAaddress	*base_addr;	/* Input/Return param (Pointer by ref)*/
    CDAsize	*byte_size;	/* Input/Return param (Ref) */
    CDAsize	increment;	/* Input param (Value) */

{

    /* Local variables */
    CDAaddress	*new_base;
    CDAsize	new_size;
    CDAstatus	status;


    /*
     *	Allocate new memory structure.
     */
    new_size = *byte_size + increment;
    status = DVR_ALLOCATE_MEMORY(new_size, &new_base);


    if ((new_base == NULL) || !DVRSuccess(status))
	return DVR_MEMALLOFAIL;	/* Memory allocation failure */

    /*
     *	Initialize new memory to NULL and copy data from current
     *	memory region to new memory region.
     */
    memset ( new_base, 0, (unsigned int) new_size );	/* initialize to all null chars */
    if ((*base_addr != 0) && (*byte_size != 0))
	{
	    CDAsize old_size = *byte_size;

	    memcpy ( new_base, *base_addr, (unsigned int) *byte_size );
	    /* Deallocate current memory structure. */
	    /* if (free(*base_addr) == -1) */
	    status = DVR_DEALLOCATE_MEMORY(old_size, base_addr);
	    if (!DVRSuccess(status))
		return DVR_MEMDEALLOFAIL;  /* Memory deallocation failure */
	}

    /*
     *	Setup new memory region return values via calling params.
     */
    *base_addr = new_base;
    *byte_size = new_size;

    return DVR_NORMAL;

} /* end realloc_memory */
