/******************************************************************************
**
**  Copyright (c) Digital Equipment Corporation, 1990-1991 All Rights Reserved. 
**  Unpublished rights reserved under the copyright laws of the United States.
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.

**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the U.S. 
**  Government is subject to restrictions as set forth in Subparagraph 
**  (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
**/

/************************************************************************
**
**  FACILITY:
**
**      Image Processing Services (IPS)
**
**  ABSTRACT:
**
**      This module contains the user level service and support routines
**	for arithmetic operations on bitmap data.
**
**  ENVIRONMENT:
**
**      VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**      Richard J. Piccolo
**
**  CREATION DATE:
**
**      15-JAN-1990
**
**
************************************************************************/

#include <ipsprot.h>
/*
**  Table of contents
*/
#ifdef NODAS_PROTO
void _IpsInitMemoryTable();
#endif

/*
**  Equated symbols
*/
# define IpsK_Alloc				0
# define IpsK_Dealloc				1
# define IpsK_Realloc				2
# define IpsK_AllocateDataPlane			3 
# define IpsK_FreeDataPlane			4 
# define IpsK_ReallocateDataPlane		5 

/* NOTE: this must agree with IpsMemoryTable.h declaration */
#define IpsM_InitMem				1  

/*
** global entry for the Ips Memory Table
*/
#ifdef NODAS_PROTO
static unsigned char *default_alloc_image();
static void default_dealloc_image();
static unsigned char *default_realloc_image();

static unsigned char *default_alloc_buffer();
static void default_dealloc_buffer();
static unsigned char *default_realloc_buffer();
#else
PROTO(static unsigned char *default_alloc_image, (unsigned long /*size_in_bytes*/, unsigned long /*init_flag*/, unsigned char /*fill_byte*/));
PROTO(static void default_dealloc_image, (unsigned char */*mem_ptr*/));
PROTO(static unsigned char *default_realloc_image, (unsigned char */*mem_ptr*/, unsigned long /*new_size_in_bytes*/));

PROTO(static unsigned char *default_alloc_buffer, (unsigned long /*size_in_bytes*/, unsigned long /*init_flag*/, unsigned char /*fill_byte*/));
PROTO(static void default_dealloc_buffer, (unsigned char */*mem_ptr*/));
PROTO(static unsigned char *default_realloc_buffer, (unsigned char */*mem_ptr*/, unsigned long /*new_size_in_bytes*/));
#endif


/* table of pointers to mem routines */

#ifdef VMS
globaldef {"$$z_ac_IPSA_MEMORYTABLE"} noshare
#endif
    unsigned char *IpsA_MemoryTable[6]
	= {(unsigned char *)default_alloc_buffer,
	   (unsigned char *)default_dealloc_buffer,
	   (unsigned char *)default_realloc_buffer, 
	   (unsigned char *)default_alloc_image,
	   (unsigned char *)default_dealloc_image,
	   (unsigned char *)default_realloc_image};

/*
**  External references
*/
#ifdef NODAS_PROTO
void	cfree();
void	*malloc();
void	*realloc();
#endif

/*****************************************************************************
**  _IpsInitMemoryTable
**
**  FUNCTIONAL DESCRIPTION:
**	
**	Initializes the Ips Memory Vector Table
**
**  FORMAL PARAMETERS:
**
**	A series of six function pointers in the following order
**
**	Allocate Data Plane - for image plane allocations
**	Deallocate Data Plane - for image data plane deallocation
**	Reallocate Data Plane - for image data plane reallocation
**
**	Allocate Buffer		- any buffer allocation other than image
**	Deallocate Buffer	- any buffer deallocation other than image
**	Reallocation Buffer	- any buffer reallocation other than image
**
**  IMPLICIT INPUTS:  none
**  IMPLICIT OUTPUTS: none
**  FUNCTION VALUE:   none
**  SIGNAL CODES:     none
**  SIDE EFFECTS:     none
**
************************************************************************/
void _IpsInitMemoryTable (allocate_image,
			  deallocate_image,
			  reallocate_image,
			  allocate_buffer,
			  deallocate_buffer,
			  reallocate_buffer)
unsigned char *allocate_image;
unsigned char *reallocate_image;
unsigned char *deallocate_image;
unsigned char *allocate_buffer;
unsigned char *deallocate_buffer;
unsigned char *reallocate_buffer;

{
    if (allocate_image != 0)
	IpsA_MemoryTable[IpsK_AllocateDataPlane] = 
	    (unsigned char *)allocate_image;
    if (deallocate_image != 0)
    	IpsA_MemoryTable[IpsK_FreeDataPlane] = 
	    (unsigned char *)deallocate_image;
    if (reallocate_image != 0)
	IpsA_MemoryTable[IpsK_ReallocateDataPlane] = 
	    (unsigned char *)reallocate_image;

    if (allocate_buffer != 0)
	IpsA_MemoryTable[IpsK_Alloc] = 
	    (unsigned char *)allocate_buffer;
    if (deallocate_buffer != 0)
	IpsA_MemoryTable[IpsK_Dealloc] = 
	    (unsigned char *)deallocate_buffer;
    if (reallocate_buffer != 0)
	IpsA_MemoryTable[IpsK_Realloc] = 
	    (unsigned char *)reallocate_buffer;

    return;
}

static unsigned char *default_alloc_image (
    unsigned long   size_in_bytes,
    unsigned long   init_flag,
    unsigned char   fill_byte
    )
{
    unsigned char * mem_ptr;
    unsigned char * ptr;
    unsigned long   i;

    mem_ptr = (unsigned char *) malloc (size_in_bytes);
    ptr = (unsigned char *) mem_ptr;
    if (init_flag == IpsM_InitMem)
	if (mem_ptr != 0)
	    for (i=0; i < size_in_bytes; i++) *ptr++ = fill_byte;

    return (unsigned char*) mem_ptr;
}

static void default_dealloc_image (mem_ptr)
unsigned char  * mem_ptr;
{
    cfree (mem_ptr);
}

static unsigned char *default_realloc_image(mem_ptr, new_size_in_bytes)
unsigned char *		mem_ptr;
unsigned long	new_size_in_bytes;
{
    unsigned char * new_mem_ptr;
    new_mem_ptr = (unsigned char *)realloc (mem_ptr, new_size_in_bytes);
    return (unsigned char*) new_mem_ptr;
}

static unsigned char *default_alloc_buffer (
    unsigned long   size_in_bytes,
    unsigned long   init_flag,
    unsigned char   fill_byte
    )
{
    unsigned char *	    mem_ptr;
    unsigned char *	    ptr;
    unsigned long   i;

    mem_ptr = (unsigned char *) malloc (size_in_bytes);
    ptr = (unsigned char *) mem_ptr;
    if (init_flag == IpsM_InitMem)
	if (mem_ptr != 0)
	    for (i=0; i < size_in_bytes; i++) *ptr++ = fill_byte;

    return (unsigned char *) mem_ptr;
}

static void default_dealloc_buffer (mem_ptr)
unsigned char *		mem_ptr;
{
    cfree (mem_ptr);
}

static unsigned char *default_realloc_buffer (mem_ptr, new_size_in_bytes)
unsigned char *		mem_ptr;
unsigned long	new_size_in_bytes;
{
    unsigned char * new_mem_ptr;
    new_mem_ptr = (unsigned char *)realloc (mem_ptr, new_size_in_bytes);
    return (unsigned char*) new_mem_ptr;
}

