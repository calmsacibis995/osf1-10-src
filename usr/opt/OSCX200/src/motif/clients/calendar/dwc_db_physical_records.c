/* dwc_db_physical_records.c */
#ifndef lint
static char rcsid[] = "$Header$";
#endif /* lint */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; database access routines
**
**  AUTHOR:
**
**	Per Hamnqvist, November 1987
**
**  ABSTRACT:
**
**	This module contains the lowest level file I/O routines, such
**	as reading and writing physical records in the file. It also contains
**	support for record allocation and deallocation.
**
**--
*/

/*
**  Include Files
*/
#include "dwc_compat.h"
#include <stdio.h>
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
#include <string.h>

#ifdef VMS
#include <file.h>
#include <unixio.h>
#else
#include <sys/file.h>
#include <sys/types.h>
#include <sys/uio.h>
#endif

#include "dwc_db_public_structures.h"
#include "dwc_db_private_include.h"
#include "dwc_db_public_include.h"

#define BSF(foo) DWC$$DB_Byte_swap_field((dwcaddr_t)&foo, (int)sizeof(foo));


int DWC$$DB_Mbz_validate
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	char *buf,
	int len)
#else	/* no prototypes */
	(Cab, buf, len)
	struct DWC$db_access_block	*Cab;
	char *buf;
	int len;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine makes sure that specified MBZ buffer is indeed
**	zero. The input buffer is byte aligned and has a byte length.
**	If the test fails, the routine signals an error.
**
**  FORMAL PARAMETERS:
**
**	Cab : Pointer to DWC$db_access_block
**	buf : Pointer to buffer to be validated
**	len : length of buffer, in bytes
**	
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	 0  -- Test was successful
**      -1  -- Test failed (and error was signalled)
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    int i;
    for (i=0; i<len; i++)
	if (*buf++ != '\0')
	    {
	    _Signal(DWC$_MBZFAIL);
	    return(-1);
	    }
    return (0);
}

VM_record *DWC$$DB_Read_physical_record
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int position,
	int Count,
	char Type)
#else	/* no prototypes */
	(Cab, position, Count, Type)
	struct DWC$db_access_block	*Cab;
	int position;
	int Count;
	char Type;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine reads database records from the position indicated
**	and stores them in specified buffer.
**
**  FORMAL PARAMETERS:
**
**	Cab	: Pointer to DWC$db_access_block
**	position: Position in the file to read from
**	Count	: Number of records to read in.
**	Type	: Type of record expected at indicated position.
**	
**  IMPLICIT INPUTS:
**
**      First char in database record is record type.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	_Failure		-- Failed to get buffer or Read failed
**					    (and error was signalled)
**	Valid buffer address	-- Read was successful.
**
**  SIDE EFFECTS:
**
**      Virtual memory allocated.
**
**--
*/
{
    struct  DWC$db_vm_record *Buffer_header;	/* Virtual buffer header    */
    int	    I, Start_bit, Free_bit, i;
    char    *Buffer;
    size_t  read_val;
    int	    seek_val;
    /*
    **  Declare operation to be DWC$_READREC.
    */
	
    _Set_cause(DWC$_READREC);
    
    /*
    **  Validate the record pointer.
    */
    if (DWC$$DB_Validate_record_ptr(Cab, position, Count) == _Failure)
    {
	/*
	** Invalid record pointer detected
	*/
	_Signal (DWC$_INVPTR);
        _Pop_cause;
	return ((VM_record *)_Failure);
    }
    /*
    **	Allocate a buffer of the required size. Position into the file as
    **	desired Read the record into the allocated buffer.
    **	Match record type against expected type. Neither header nor bitmap
    **	record is expected here. Fill in VM buffer header fields.
    */
    DWC$$DB_Get_free_buffer
	(Cab, DWC$k_db_record_length*Count, &Buffer_header);
    /*
    ** Real data address
    */
    Buffer = (char *)&(Buffer_header->DWC$t_dbvm_data[0]);

    /*
    **  All set! Position the file, verifying that works, and READ first record
    **	of set and validate type.
    */
    read_val = 0;
    seek_val = fseek (Cab->DWC$l_dcab_fd, position, SEEK_SET);
    if (seek_val == 0)
    {
	read_val = fread
	    (Buffer, DWC$k_db_file_record_length, 1, Cab->DWC$l_dcab_fd);
    }
    if (read_val != 1)
    {
	_Record_error;
        DWC$$DB_Freelist_buffer (Cab, Buffer_header);
	_Signal (DWC$_FILEIOERR);
        _Pop_cause;
	return ((VM_record *)_Failure);
    }
    if (Type != 0)
    {
	if (Buffer[0] != Type)			/* type is ALWAYS first	byte*/
	{
	    DWC$$DB_Freelist_buffer (Cab, Buffer_header);
	    _Signal (DWC$_INVRTYPE);
	    _Pop_cause;
	    return ((VM_record *)_Failure);
	}
    }
    /*
    **  If more than one record requested, read rest of records
    */
    if (Count != 1)
    {
	for (i=1; i<Count; i++)
	{
	    read_val = fread
	    (
		&Buffer[i * DWC$k_db_file_record_length],
		DWC$k_db_file_record_length,
		1,
		Cab->DWC$l_dcab_fd
	    );
	    if (read_val != 1)
	    {
		_Record_error;
		DWC$$DB_Freelist_buffer (Cab, Buffer_header);
		_Signal (DWC$_FILEIOERR);
		_Pop_cause;
		return ((VM_record *)_Failure);
	    }
	}
    }

#if BYTESWAP
    if (Type != 0)
    {
	DWC$$DB_Byte_swap_buffer (Buffer, TRUE);
    }
#endif
#if 0
#if (DWC_LONG_BIT == 64)
/* after byte swap, must create a 64 bit copy. */
    TempBuffer = DWCDB__64BitBuffer (Buffer);
#endif
#endif

    Buffer_header->DWC$l_dbvm_rec_addr = position;

/*
**	7.  Call record specific MBZ (must be zero) and validation routine.
**            Each such routine should make sure that MBZ fields are indeed zero
**            and that fields are within expected ranges.  The following
**            validation routines exist:
**
**            c. DWC$$DB_Validate_profile_blk
**            d. DWC$$DB_Validate_expr_blk
**            e. DWC$$DB_Validate_expr_item_blk
**            f. DWC$$DB_Validate_rangemap_blk
**            g. DWC$$DB_Validate_subrangemap_blk
**            h. DWC$$DB_Validate_day_data_blk
**            i. DWC$$DB_Validate_day_item_blk
*/
	
    /*
    **  Done, back to caller returning the Buffer Header Pointer.
    */
    _Pop_cause;
    return(Buffer_header);
}

int DWC$$DB_Write_physical_record
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int position,
	int Count,
	char *Buffer)
#else	/* no prototypes */
	(Cab, position, Count, Buffer)
	struct DWC$db_access_block	*Cab;
	int position;
	int Count;
	char *Buffer;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**        This routine writes out records into the database.
**	  
**  FORMAL PARAMETERS:
**
**	Cab	: Pointer to DWC$db_access_block
**	position: Position in the file to write to 
**	Count	: Number of records to write.
**	Buffer	: Address of the buffer to write out. NOT the header address.
**	
**  IMPLICIT INPUTS:
**
**      record address field in the CAB
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	_Failure    -- Failed to WRITE 
**					    (and error was signalled)
**	_Succes	    -- Write was successful.
**
**  SIDE EFFECTS:
**
**	none
**--
*/
{
    size_t  writ_val;
    int	    seek_val;
    /*
    **  Declare current operation to be DWC$_WRITEREC
    */
	
    _Set_cause(DWC$_WRITEREC);
    
    /*
    **  Validate the record pointer, first a general sanity check on the
    **	pointer and assuring the file spaces is marked as allocated.
    **	We can NOT check desired position against buffer header information
    **	because we will not (always) be called from a VM buffer.
    */
    if (DWC$$DB_Validate_record_ptr(Cab, position, Count) == _Failure)
    {
	_Signal(DWC$_INVPTR );			/* Invalid record pointer   */
        _Pop_cause;
	return(_Failure);			/*   detected		    */
    }
    /*
    ** Verify whether we got write access or not.
    */
    if (!(Cab->DWC$l_dcab_flags & DWC$m_dcab_write))
    {
	_Signal(DWC$_NOWRITE);			/* signal no write access  */
        _Pop_cause;
	return(_Failure);
    }

    /*
    ** OK, byte swap the buffer before writing.
    */
#if 0
#if (DWC_LONG_BIT == 64)
/* before byte swap, must create a 32 bit copy. */
    DWCDB__32BitBuffer (Buffer);
#endif
#endif
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (Buffer, FALSE);
#endif
    /*
    **  All set! Position the file, verifying that works, and WRITE.
    **
    */
    writ_val = 0;
    seek_val = fseek (Cab->DWC$l_dcab_fd, position, SEEK_SET);
    if (seek_val == 0)
    {
	writ_val = fwrite
	    (Buffer, DWC$k_db_file_record_length, Count, Cab->DWC$l_dcab_fd);
    }
    if (writ_val != Count)
    {
	_Record_error;
	_Signal (DWC$_FILEIOERR);
        _Pop_cause;
	return (_Failure);
    }

    /*
    ** Now, byte swap the buffer back again.
    */
#if BYTESWAP
    DWC$$DB_Byte_swap_buffer (Buffer, TRUE);
#endif
#if 0
#if (DWC_LONG_BIT == 64)
/* after swap, must create change back to a 64 bit copy. */
    DWCDB__64BitBuffer (Buffer);
#endif
#endif
    /*
    **  Done, back to caller
    */
    Cab->DWC$l_write++;
    _Pop_cause;
    return (_Success);
}

int DWC$$DB_Validate_record_ptr
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Rec_addr,
	int Count)
#else	/* no prototypes */
	(Cab, Rec_addr, Count)
	struct DWC$db_access_block	*Cab;
	int Rec_addr;
	int Count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  Validate an record pointer. Make sure it is not beyond the current
**  end of file, and the it is truly allocated.
**
**  FORMAL PARAMETERS:
**
**	Cab	: Pointer to DWC$db_access_block
**	Rec_addr: Pointer to position in the file to verify from
**	Count	: Number of records to read in.
**	
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	_Failure    -- Pointer was not valid.
**	_succes	    -- Pointer was valid.
**
**  SIDE EFFECTS:
**
**        *  DWC$_INVPTR - Invalid record pointer detected
**        *  DWC$_PTRBEOF - Record pointer points beyond end of file
**
**--
*/
    {
    struct  DWC$db_vm_record *Bitmap_VM;	/* Virtual bitmap header    */
    struct  DWCDB_bitmap    *Bitmap;
    int	    I, Bitmap_addr, Rec_num, Start_bit, Free_bit;
    int status;					/* Temp work status	    */
    
    /*
    **  Make sure the desired position is could posibly reflect a valid
    **	record postion in the file, whether truly allocated or not.
    **	It must greater than zero, record alligned and the last byte
    **	requested should not be beyond the current end of file.
    */
    Rec_num = Rec_addr / DWC$k_db_record_length;
    if ((Rec_addr < 0) ||
	(Rec_addr + (Count*DWC$k_db_record_length) > Cab->DWC$l_dcab_eof) ||
	(Rec_addr != Rec_num * DWC$k_db_record_length) ||
	(Count > DWC$k_db_max_allocation_chunk))
	{
	return(_Failure);
	}

    /*
    **	Find the corresponding bitmap and bit number. Make sure the
    **	entire range does not extend beyond then end of the bitmap
    **	and that all of it is allocated.
    */
    Bitmap_VM	= (VM_record *)Cab->DWC$a_dcab_bitmap_flink;
    Bitmap_addr	= (Rec_num / DWC$k_db_bitmap_entries)
		    * DWC$k_db_bitmap_entries * DWC$k_db_record_length;
    Start_bit	=  Rec_num % DWC$k_db_bitmap_entries;
    if (Start_bit + Count <= DWC$k_db_bitmap_entries)
	{
        while (Bitmap_VM != (VM_record *)&(Cab->DWC$a_dcab_bitmap_flink))
	    {
	    if (Bitmap_VM->DWC$l_dbvm_rec_addr == Bitmap_addr)
		{
		/*
		** Looking at the corresponding bit map now.
		** Find bit position in this bitmap corresponding with the
		** record(s) position. Make sure that no free record is
		** indicated following the start position and within the
		** size needed for total requested length.
		*/
		Bitmap = _Bind(Bitmap_VM, DWCDB_bitmap);
		if (DWC$$DB_Find_clear_bits(
				Bitmap->DWC$t_dbbi_bitmap,
				Count,
				Count,
				&I) == _Failure)
		    {
		    return (_Success);		/* ALL tests succefull	    */
		    }
		}
	    Bitmap_VM = Bitmap_VM->DWC$a_dbvm_vm_flink;
	    } /* Is this the right bitmap? */
	}/* Within range of single bitmap? */
    /*
    **	At this point, a test failed. We don't care too much which one.
    **	Simply return indicating failure
    */
    return(_Failure);	
    }

int DWC$$DB_Alloc_records
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Count)
#else	/* no prototypes */
	(Cab, Count)
	struct DWC$db_access_block	*Cab;
	int Count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine allocates physical records. The bitmap is scanned,
**	looking for a contiguous sequence of free bits. The file is
**	extended if needed.
**
**  FORMAL PARAMETERS:
**
**	Cab	: Pointer to DWC$db_access_block
**	Count	: Number of records to allocate.
**	
**  IMPLICIT INPUTS:
**
**      All the current bitmap are in memory linked of the CAB.
**
**  IMPLICIT OUTPUTS:
**
**	Bitmap updated. File may be extended and EOF pointers
**	updated accordingly.
**
**  COMPLETION CODES:
**
**	 Null    -- Allocation failed
**       Pointer -- If succesfull, address of free record in file.
**
**  SIDE EFFECTS:
**
**      File may be extended and EOF pointers updated accordingly.
**
**--
*/
    {
    struct  DWC$db_vm_record *Bitmap_VM;	/*  header for current bitmap */
    struct  DWCDB_bitmap    *Bitmap;		/*  current bitmap searched */
    int	    I, Free_bit;
    int	    Free_rec;
    int	    Allocated;				/* Number of records alloc  */
    int	    status;
    
    /*
    **  Before we waste any energy on this, make sure that the requested
    **	count of records does not exceed what we can handle. If it does,
    **	simply return NULL to indicate that the requested number of blocks could
    **	not be obtained.
    */
    if (Count > DWC$k_db_max_allocation_chunk) return (0);

    /*
    **	First Declare operation to be DWC$_ALLOCREC. Then try twice to find
    **	free space. If the first attempt fails then we shall extend the file.
    **	Return with error status if the extend fails.
    */
	
    _Set_cause(DWC$_ALLOCREC);
    
    Allocated = Count;
    while (Allocated>0)
	{
	/*
	**  Start scanning the first bitmap linked of the cab.
	**  Exit from loop if free space found or end of list.
	*/
	Bitmap_VM = (VM_record *)Cab->DWC$a_dcab_bitmap_flink;
	while (Bitmap_VM != (VM_record *)&(Cab->DWC$a_dcab_bitmap_flink))
	    {
	    Bitmap = _Bind(Bitmap_VM, DWCDB_bitmap);
	    if (_Success==DWC$$DB_Find_clear_bits(Bitmap->DWC$t_dbbi_bitmap,
			Bitmap->DWC$w_dbbi_eob + 1, Count, &Free_bit))
	        {
		/*
		**  Good, found enough free space in this bitmap. Mark them!
		**  Calculate the corresponding file position and return that.
		**  Also make this bitmap the first in the list.
		*/
		DWC$$DB_Remque
		(
		    (struct DWC$db_queue_head *) Bitmap_VM->DWC$a_dbvm_vm_blink,
		    (struct DWC$db_queue_head **) 0
		);
		DWC$$DB_Insque
		(
		    (struct DWC$db_queue_head *)&Bitmap_VM->DWC$a_dbvm_vm_flink,
		    (struct DWC$db_queue_head *)&Cab->DWC$a_dcab_bitmap_flink
		);
		DWC$$DB_Set_bits(Bitmap->DWC$t_dbbi_bitmap, Free_bit, Count);
		status = DWC$$DB_Write_physical_record
		(
		    Cab, 
		    Bitmap_VM->DWC$l_dbvm_rec_addr,
		    1,
		    (char *)Bitmap
		);
		if (_Failure == status)
		{
		    _Signal(DWC$_FILEIOERR);
		    _Pop_cause;
		    return (0);
		}
		Free_rec = Bitmap_VM->DWC$l_dbvm_rec_addr +
		    (Free_bit * DWC$k_db_record_length);
		Cab->DWC$l_allocate ++;
		_Pop_cause;
		return (Free_rec);
		}
	    Bitmap_VM = Bitmap_VM->DWC$a_dbvm_vm_flink;    /* Follow */
	    } 
	Allocated = DWC$$DB_Extend_file(Cab,Count);	    /* Extend and try again   */
	}

    /*
    **  If we fall through to here then there was no room in the current
    **	bitmaps and the extend failed. Return NULL pointer to indicate failure
    */
    _Pop_cause;
    return (0);  
}

int DWC$$DB_Deallocate_records
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Count,
	int Record_address)
#else	/* no prototypes */
	(Cab, Count, Record_address)
	struct DWC$db_access_block	*Cab;
	int Count;
	int Record_address;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine de-allocates physical records. The correct bitmap
**	found, the corresponding bits cleared, and bitmap written out.
**	[We could choose NOT to write out. Litle risk, reasonable gain.]
**
**  FORMAL PARAMETERS:
**
**	Cab	: Pointer to DWC$db_access_block
**	
**  IMPLICIT INPUTS:
**
**      All the current bitmap are in memory linked of the CAB.
**
**  IMPLICIT OUTPUTS:
**
**	Bitmap updated. 
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**
**--
*/
{
    struct  DWC$db_vm_record *Bitmap_VM;	/*  header for current bitmap */
    struct  DWCDB_bitmap    *Bitmap;		/*  current bitmap searched */
    struct  DWCDB_header    *Header;		/*  DB header in memory	    */
    int Bitmap_address;
    int Bit_number;
    int Record_number;
    int	status;
    /*
    **  Start scanning the first bitmap linked of the cab.
    **  Exit from loop if free space found or end of list.
    */
    Record_number = Record_address / DWC$k_db_record_length;
    Bit_number	  = Record_number % DWC$k_db_bitmap_entries;
    Bitmap_address= ( Record_number / DWC$k_db_bitmap_entries) *
		       DWC$k_db_bitmap_entries * DWC$k_db_record_length;
    Bitmap_VM = (VM_record *)Cab->DWC$a_dcab_bitmap_flink;
    while (Bitmap_VM != (VM_record *)&(Cab->DWC$a_dcab_bitmap_flink))
    {
	if ( Bitmap_VM->DWC$l_dbvm_rec_addr == Bitmap_address )
	{
	    Bitmap = _Bind(Bitmap_VM, DWCDB_bitmap);
	    DWC$$DB_Clear_bits(Bitmap->DWC$t_dbbi_bitmap, Bit_number, Count);
	    Cab->DWC$l_free ++;
	    status = DWC$$DB_Write_physical_record
	    (
		Cab,
		Bitmap_VM->DWC$l_dbvm_rec_addr,
		1,
		(char *) Bitmap
	    );
	    if (_Failure == status)
	    {
		_Signal(DWC$_FILEIOERR);
		return (FALSE);
	    }
	    else
	    {
		return (TRUE);
	    }
	}
	Bitmap_VM = Bitmap_VM->DWC$a_dbvm_vm_flink;    /* Follow */
    }

    printf ("DWC$DB : Could not find bitmap to DEALLOCATE from.\n");
    return (FALSE);
}

int DWC$$DB_Extend_file
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab,
	int Count)
#else	/* no prototypes */
	(Cab, Count)
	struct DWC$db_access_block	*Cab;
	int Count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine extends the file. Updating all relevant datastructures.
**	We try an extension equal to the default extend quantity.
**	If that fails, only a minimal extension should be done.
**	This fall back is not implemented now. 
**
**  FORMAL PARAMETERS:
**
**	Count	: Minimum Number of records to allocate.
**	
**  IMPLICIT INPUTS:
**
**      All the current bitmap are in memory linked of the CAB.
**
**  IMPLICIT OUTPUTS:
**
**	File extended. EOF pointers updated accordingly.
**	Updated (new) bitmap write out is delayed until after the
**	allocation for which this extend was called has taken place.
**
**  COMPLETION CODES:
**
**	 Null    -- Allocation failed. non-fatal error signalled.
**       Count   -- If succesfull, copy of input parameter.
**
**  SIDE EFFECTS:
**
**      File may be extended and EOF pointers updated accordingly.
**
**--
*/
    {
    VM_record		    *Temp_bitmap_VM;	/*  header for temp bitmap  */
    VM_record		    *Bitmap_VM;		/*  header for bitmap	    */
    VM_record		    *Header_VM;		/*  file header record	    */
    struct  DWCDB_bitmap   *Bitmap;		/*  last bitmap in file	    */
    struct  DWCDB_header   *Header;		/*  DB header in memory	    */
    char    Empty_record[DWC$k_db_record_length];
    int	    I, Eof, Bitmap_address, Bits_left, Extend_size;
    struct DWCDB_bitmap Empty_map;		/* New empty bitmap	    */
    int Allocated;				/* Number of records alloc  */
    size_t  writ_val;
    int	    seek_val;

    memset(Empty_record, 0, DWC$k_db_record_length);
    Empty_record[0]=DWC$k_db_uninitialized;
    Bitmap_address = 0;			/* address of first bitmap in file */
    Temp_bitmap_VM = (VM_record *)Cab->DWC$a_dcab_bitmap_flink;
    /*
    **  Find last bitmap in the file. This is the one to extend.
    **	Test for equal to catch first bitmap.
    */
    while (Temp_bitmap_VM != (VM_record *)&(Cab->DWC$a_dcab_bitmap_flink))
	{
	if (Temp_bitmap_VM->DWC$l_dbvm_rec_addr >= Bitmap_address)
	    {
	    Bitmap_address = Temp_bitmap_VM->DWC$l_dbvm_rec_addr;
	    Bitmap_VM = Temp_bitmap_VM;
	    }
	Temp_bitmap_VM = Temp_bitmap_VM->DWC$a_dbvm_vm_flink;
	}	    

    Bitmap = _Bind(Bitmap_VM, DWCDB_bitmap);
    Header_VM = Cab->DWC$a_dcab_header;	
    Header = _Bind(Header_VM, DWCDB_header);
    Extend_size = Header->DWC$l_dbhd_deq;
    
    Eof = ((Cab->DWC$l_dcab_eof - Bitmap_address)/DWC$k_db_record_length) - 1;
    if (Bitmap->DWC$w_dbbi_eob != Eof)
	{
	printf ("\nDB EXTEND: Eof - Bitmap size mismatch, Using EOF.\n");
	Bitmap->DWC$w_dbbi_eob = Eof;
	}	
    Bits_left = (DWC$k_db_bitmap_entries - Bitmap->DWC$w_dbbi_eob) - 1 ;

    if ((Count > Extend_size) || (Extend_size > Bits_left))
	{
	Extend_size = Count;
	}

    /*
    **  Position at end of file, for extend
    */
    seek_val = fseek (Cab->DWC$l_dcab_fd, Cab->DWC$l_dcab_eof, SEEK_SET);
    if (seek_val == EOF)
    {
	_Record_error;
	_Signal(DWC$_FILEIOERR);
	return (0);
    }
    
    /*
    **  Figure out if the extend requires us to fill out to the end of
    **	the current before we can continue to extend (or not).
    */
    Allocated = 0;
    if (Extend_size > Bits_left)
    {

	/*
	**  First, try and extend the file towards the end of this bitmap. If
	**  this fails, update the bitmap to indicate how much we actually
	**  managed to extend the file and return count back to caller.
	*/
	for (I=0; I < Bits_left; I++)
	{
	    writ_val = fwrite
		(Empty_record, 	DWC$k_db_file_record_length, 1, Cab->DWC$l_dcab_fd);
	    if (writ_val != 1)
	    {	
		_Record_error;
		Bitmap->DWC$w_dbbi_eob += I;
		DWC$$DB_Write_physical_record
		    (Cab, Bitmap_address, 1, (char *) Bitmap);
		Cab->DWC$l_dcab_eof += (I * DWC$k_db_record_length);
		return (I);
	    }
	}
	    
	/*
	**  Good. We managed to extend at least to the end of the current
	**  bitmap. Update the bitmap and the end of file pointer in the Cab.
	*/
	Bitmap->DWC$w_dbbi_eob += Bits_left;
	DWC$$DB_Write_physical_record
	    (Cab, Bitmap_address, 1, (char *) Bitmap);
	Cab->DWC$l_dcab_eof += (Bits_left * DWC$k_db_record_length);
	
	/*
	**  Try and extend file to get a new bitmap in place.
	*/
	seek_val = fseek (Cab->DWC$l_dcab_fd, Cab->DWC$l_dcab_eof, SEEK_SET);
	if (seek_val == EOF)
	{
	    _Record_error;
	    _Signal (DWC$_FILEIOERR);
	    return (0);
	}
	memset(&Empty_map, 0, DWC$k_db_record_length);
	Empty_map.DWC$b_dbbi_blocktype  = DWC$k_db_bitmap;
	Empty_map.DWC$w_dbbi_eob = 0;
	Empty_map.DWC$t_dbbi_bitmap[0] = 1;
	writ_val = fwrite
	    (&Empty_map, DWC$k_db_file_record_length, 1, Cab->DWC$l_dcab_fd);
	if (writ_val != 1)
	{
	    _Record_error;
	    return (Bits_left);
	}
	Cab->DWC$l_dcab_eof += DWC$k_db_record_length;
	
	/*
	**  There was enough room for this bitmap. Allocate a working
	**  buffer and copy in our temp map into it.
	*/
	DWC$$DB_Get_free_buffer (Cab, DWC$k_db_record_length, &Bitmap_VM);
	Bitmap = _Bind(Bitmap_VM, DWCDB_bitmap);
	memcpy (Bitmap, &Empty_map, DWC$k_db_record_length);
	
	/*
	**  Update memory structures to take the new bitmap into account.
	**  We can hook it up in the begin or end as we please becuase the
	**  allocation routine will put the bitmap that last fullfilled an
	**  allocation in the front. Suppose we extended to get a chunk bigger
	**  then currently free. The new bitmap will become first.
	**  A subsequent allocation of one block will require a new extend
	**  in the new bitmap but will possibly succceed in an old bitmap
	**  thus making that the first. ad infinitum.
	*/
	Bitmap_address += (DWC$k_db_bitmap_entries * DWC$k_db_record_length);
	Bitmap_VM->DWC$l_dbvm_rec_addr = Bitmap_address;
	DWC$$DB_Insque
	(
	    (struct DWC$db_queue_head *) &Bitmap_VM->DWC$a_dbvm_vm_flink,
	    (struct DWC$db_queue_head *) &Cab->DWC$a_dcab_bitmap_flink
	);

	/*
	**  Save count of records we have extended so far in case the next
	**  extend will fail.
	*/
	Allocated = Bits_left;
	
    }

    /*
    **  Extend file the requested number of records. Deal with case where
    **	the extend fails too.
    */
    for (I=0; I < Extend_size; I++)
    {
	writ_val = fwrite
	    (Empty_record, DWC$k_db_file_record_length, 1, Cab->DWC$l_dcab_fd);
	if (writ_val != 1)
	{	
	    _Record_error;
	    Bitmap->DWC$w_dbbi_eob += I;
	    DWC$$DB_Write_physical_record
		(Cab, Bitmap_address, 1, (char *) Bitmap);
	    Cab->DWC$l_dcab_eof += (I * DWC$k_db_record_length);
	    if (I > Allocated) Allocated = I;
	    return (Allocated);
	}
    }

    /*
    **  Update end of bitmap and end of file pointer.
    */
    Bitmap->DWC$w_dbbi_eob += Extend_size;
    DWC$$DB_Write_physical_record
	(Cab, Bitmap_address, 1, (char *) Bitmap);
    Cab->DWC$l_dcab_eof += (Extend_size * DWC$k_db_record_length);

    /*
    **  Done, successful extend. Tell caller
    */
    return (Extend_size);
}

int DWC$$DB_Find_clear_bits
#ifdef	_DWC_PROTO_
	(
	char *Bitmap,
	int Bitmap_size,
	int Target,
	int *Free_bit)
#else	/* no prototypes */
	(Bitmap, Bitmap_size, Target, Free_bit)
	char *Bitmap;
	int Bitmap_size;
	int Target;
	int *Free_bit;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Scan a bitmap for the for a contiguous sequence of free bits.
**	
**	char *Bitmap;		- Pointer to start byte of bitmap
**	int Bitmap_size;	- Total size of bitmap
**	int Target;		- Number of bits to look for
**	int *Free_bit;		- First bit of sequence if found
**	
**  COMPLETION CODES:
**
**	 _Success -- Sequence of requested size was found.
**       _Failure -- Sequence of requested size was NOT found.
**--
*/
    {
    int	    I, This_bit, Bit, Byte, Count;

    for ( I = 0; I <= (Bitmap_size)/8 ; I++)	/* Byte-Loop through bitmap */
	{
	if (~Bitmap[I])				/* Any bit free in byte?    */
	    {
	    Byte = Bitmap[I];			/* Copy (sign extended)	    */
	    for (Bit = 0; Bit < 8; Bit ++)	/* Loop individual bits	    */
		{
		Count = 0;			/* Initialize		    */
		while ( !(Byte&1) )		/* Loop while free bits	    */
		    {
		    if ( ++Count == Target)	/* Count free bits. Done?   */
			{
			This_bit = (I * 8) + Bit;  /* Perhaps...	    */
			if (This_bit < Bitmap_size) /* within limits?	    */
			    {
			    *Free_bit = This_bit - Count + 1;
			    return(_Success);	/* SUCCES		    */
			    }
			 }
		    Byte = Byte >> 1;		/* shift a bit		    */
		    if ( ++Bit >= 8)		/* Count bit, Need new byte?*/
			{
			if ( ++I * 8 < Bitmap_size) /* Count bytes, Any left? */
			    {
			    Bit = 0;		/* Init for new byte	    */
			    Byte = Bitmap[I];   /* Copy (sign extended)	    */
			    }
			else
			    {
			    return(_Failure);   /* On the edge of bitmap    */
			    }
			}/*If new byte needed				    */
		    }/* While Free bits					    */
		Byte = Byte >> 1;		/* shift a bit		    */
		
		}/* For all bits in byte				    */
	    }/* If byte had at least one bit free			    */
	}/* For all bytes in bitmap					    */
        return (_Failure);	/* Looped through all the bytes in vain	    */
    }

int DWC$$DB_Set_bits
#ifdef	_DWC_PROTO_
	(
	char *Bitmap,
	int Start_bit,
	int Count)
#else	/* no prototypes */
	(Bitmap, Start_bit, Count)
	char *Bitmap;
	int Start_bit;
	int Count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This SETs a number of bits in a bitmap at indicated offset.
**	Previous state is not tested. (Might return WAS_SET/WAS_CLEAR in futur).
**
**	char *Bitmap;		- Pointer to start byte of bitmap
**	int Start_bit;		- Number of bit to start
**      int Count;		- Number of bits to set
**
**--
*/
    {
    char    Bitmask[9];
    int	    I, Bit, Byte, Bits_in_startbyte ;

    /*
    **	Start by initializing the mask vector
    */
    Bitmask[0]=0x00;
    Bitmask[1]=0x01;
    Bitmask[2]=0x03;
    Bitmask[3]=0x07;
    Bitmask[4]=0x0F;
    Bitmask[5]=0x1F;
    Bitmask[6]=0x3F;
    Bitmask[7]=0x7F;
    Bitmask[8]=0xFF;

    Byte = Start_bit / 8;
    Bit  = Start_bit % 8;
    Bits_in_startbyte = 8 - Bit;
    if (Count < Bits_in_startbyte) Bits_in_startbyte = Count;
    Count -= Bits_in_startbyte;
    Bitmap[Byte] |= (Bitmask[Bits_in_startbyte] << Bit);
    while (Count > 0)
	{
        Byte++;
	if (Count > 7)
	    Bitmap[Byte] |= 0xFF;
	else
	    Bitmap[Byte] |= Bitmask[Count];
	Count -= 8;				/* may go under zero	    */
	}	    
    return;
    }

int DWC$$DB_Clear_bits
#ifdef	_DWC_PROTO_
	(
	char *Bitmap,
	int Start_bit,
	int Count)
#else	/* no prototypes */
	(Bitmap, Start_bit, Count)
	char *Bitmap;
	int Start_bit;
	int Count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This function clears a number of bits in a bitmap at indicated offset.
**	Future suggestions: 	- return WAS_SET, WAS_CLEAR, WAS_FUNNY.
**				- Use bitmask from SET function.
**
**	char *Bitmap;		- Pointer to start byte of bitmap
**	int Start_bit;		- Number of bit to start
**	int Count;		- Number of bits to clear
**--
*/
    {
    char    Bitmask[9];
    int	    I, Bit, Byte, Bits_in_startbyte ;

    /*
    **	Start by initializing the mask vector
    */
    Bitmask[0]=0x00;
    Bitmask[1]=0x01;
    Bitmask[2]=0x03;
    Bitmask[3]=0x07;
    Bitmask[4]=0x0F;
    Bitmask[5]=0x1F;
    Bitmask[6]=0x3F;
    Bitmask[7]=0x7F;
    Bitmask[8]=0xFF;

    Byte = Start_bit / 8;
    Bit  = Start_bit % 8;
    Bits_in_startbyte = 8 - Bit;
    if (Count < Bits_in_startbyte) Bits_in_startbyte = Count;
    Count -= Bits_in_startbyte;
    Bitmap[Byte] &= ~(Bitmask[Bits_in_startbyte] << Bit);
    while (Count > 0)
	{
        Byte++;
	if (Count > 7)
	    Bitmap[Byte] = 0x0;
	else
	    Bitmap[Byte] &= ~Bitmask[Count];
	Count -= 8;				/* may go under zero	    */
	}	    
    return;
    }

int DWC$$DB_Toggle_bits
#ifdef	_DWC_PROTO_
	(
	char *Bitmap,
	int Start_bit,
	int Count)
#else	/* no prototypes */
	(Bitmap, Start_bit, Count)
	char *Bitmap;
	int Start_bit;
	int Count;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This toggles a number of bits in a bitmap at indicated offset.
**	Can be used to SET or CLEAR bits. return value. Function can not fail!
**
**	char *Bitmap;		- Pointer to start byte of bitmap
**	int Start_bit;		- Number of bit to start
**	int Count;		- Number of bits to toggle
**
**--
*/
    {
    char    Bitmask[9];
    int	    I, Bit, Byte, Bits_in_startbyte ;

    /*
    **	Start by initializing the mask vector
    */
    Bitmask[0]=0x00;
    Bitmask[1]=0x01;
    Bitmask[2]=0x03;
    Bitmask[3]=0x07;
    Bitmask[4]=0x0F;
    Bitmask[5]=0x1F;
    Bitmask[6]=0x3F;
    Bitmask[7]=0x7F;
    Bitmask[8]=0xFF;

    Byte = Start_bit / 8;
    Bit  = Start_bit % 8;
    Bits_in_startbyte = 8 - Bit;
    if (Count < Bits_in_startbyte) Bits_in_startbyte = Count;
    Count -= Bits_in_startbyte;
    Bitmap[Byte] ^= (Bitmask[Bits_in_startbyte] << Bit);
    while (Count > 0)
	{
        Byte++;
	if (Count > 7)
	    Bitmap[Byte] ^= 0xFF;
	else
	    Bitmap[Byte] ^= Bitmask[Count];
	Count -= 8;				/* may go under zero	    */
	}	    
    return;
    }

void DWC$$DB_Byte_swap_buffer
#ifdef	_DWC_PROTO_
	(
	dwcaddr_t   Buffer,
	int	    preswap
	)
#else
	(Buffer, preswap)
	dwcaddr_t   Buffer;
	int	    preswap;
#endif
{
    int i;
    char				*buffer = (char *)Buffer;
#if 0
    struct DWCDB_header			*Header;
    struct DWCDB_bitmap			*Bitmap;
    struct DWCDB_rangemap		*Rangemap;
    struct DWCDB_subrangemap		*Subrangemap;
    struct DWCDB_day			*Day;
    struct DWCDB_extension		*Extension;
    struct DWCDB_repeat_vector		*RepeatVector;
    struct DWCDB_repeat_block		*RepeatBlock;
    struct DWCDB_repeat_exceptions	*RepeatExceptions;
    struct DWCDB_repeat_expr		*RepeatExpr;
    struct DWCDB_item			*Item;
    struct DWCDB_entry			*Entry;
#else
    struct DWCDB_header			*Header;
    struct DWCDB_bitmap			*Bitmap;
    struct DWCDB_rangemap		*Rangemap;
    struct DWCDB_subrangemap		*Subrangemap;
    struct DWCDB_day			*Day;
    struct DWCDB_extension		*Extension;
    struct DWCDB_repeat_vector		*RepeatVector;
    struct DWCDB_repeat_block		*RepeatBlock;
    struct DWCDB_repeat_exceptions	*RepeatExceptions;
    struct DWCDB_repeat_expr		*RepeatExpr;
    struct DWCDB_item			*Item;
    struct DWCDB_entry			*Entry;
#endif
    int					Max_data;

    switch (buffer[0])
    {
    case DWC$k_db_header:
	Header = (struct DWCDB_header *) Buffer;
	BSF(Header->DWC$l_dbhd_current_range);
	BSF(Header->DWC$l_dbhd_first_range);
	BSF(Header->DWC$l_dbhd_repeat_head);
	BSF(Header->DWC$l_dbhd_profile);
	BSF(Header->DWC$l_dbhd_deq);
	BSF(Header->DWC$l_dbhd_current_base);
	BSF(Header->DWC$l_write);
	BSF(Header->DWC$l_miss);
	BSF(Header->DWC$l_hit);
	BSF(Header->DWC$l_allocate);
	BSF(Header->DWC$l_free);
	break;
    case DWC$k_db_bitmap:
	Bitmap = (struct DWCDB_bitmap *) Buffer;
	BSF(Bitmap->DWC$w_dbbi_eob);
	break;
    case DWC$k_db_profile:
	break;
    case DWC$k_db_rangemap:
	Rangemap = (struct DWCDB_rangemap *) Buffer;
	BSF(Rangemap->DWC$l_dbra_baseday);
	BSF(Rangemap->DWC$l_dbra_flink);
	BSF(Rangemap->DWC$l_dbra_mbz);
	for (i=0; i<DWC$k_db_rangemap_entries; i++)
	    BSF(Rangemap->DWC$l_dbra_subvec[i]);
	break;
    case DWC$k_db_subrangemap:
	Subrangemap = (struct DWCDB_subrangemap *) Buffer;
	BSF(Subrangemap->DWC$w_dbsu_baseday);
	for (i=0; i<DWC$k_db_submap_entries; i++)
	    BSF(Subrangemap->DWC$l_dbsu_dayvec[i]);
	break;
    case DWC$k_db_day_data:
	Day = (struct DWCDB_day *) Buffer;
	BSF(Day->DWC$w_dbda_baseday);
	BSF(Day->DWC$l_dbda_prev_day);
	BSF(Day->DWC$w_dbda_prev_idx);
	BSF(Day->DWC$w_dbda_item_idx);
	BSF(Day->DWC$l_dbda_flink);
	BSF(Day->DWC$w_dbda_ext_count);

	Item = (struct DWCDB_item *)&(Day->DWC$t_dbda_data[0]);

	Max_data = DWC$k_db_max_day_data;

	while (Item->DWC$b_dbit_blocktype != DWC$k_dbit_not_used)
	{
	    int	    Item_size;

	    char    *place;

	    BSF(Item->DWC$w_dbit_id);
	    if (preswap)
	    {
		BSF(Item->DWC$w_dbit_size);
		Item_size = Item->DWC$w_dbit_size + (Item->DWC$w_dbit_size & 1);
	    }
	    else
	    {
		Item_size = Item->DWC$w_dbit_size + (Item->DWC$w_dbit_size & 1);
		BSF(Item->DWC$w_dbit_size);
	    }
	    Item_size = MIN(Item_size, Max_data);

	    switch (Item->DWC$b_dbit_blocktype)
	    {
	    case DWC$k_dbit_entry:
		Entry = (struct DWCDB_entry *) Item;
		BSF(Entry->DWC$w_dben_start_minute);
		BSF(Entry->DWC$l_dben_delta_days);
		BSF(Entry->DWC$w_dben_delta_minutes);
		BSF(Entry->DWC$w_dben_text_class);
		for (i = 0; i<Entry->DWC$t_dben_data[0]; i++)
		{
		    BSF(Entry->DWC$t_dben_data[i*2+1]);
		}
		break;
	    case DWC$k_dbit_extension:
		break;
	    default:
		break;
	    }
	    place = (char *) Item;
	    place += Item_size;
	    Item = (struct DWCDB_item *) place;
	    Max_data -= Item_size;
	}

	break;
    case DWC$k_db_day_extension:
    case DWC$k_db_repeat_extension:
	Extension = (struct DWCDB_extension *) Buffer;
	BSF(Extension->DWC$w_dbex_ext_count);
	break;
    case DWC$k_db_repeat_expr_vec:
	RepeatVector = (struct DWCDB_repeat_vector *) Buffer;
	BSF(RepeatVector->DWC$w_dbrv_size);
	BSF(RepeatVector->DWC$l_dbrv_flink);
	RepeatExpr =
	    (struct DWCDB_repeat_expr *) &RepeatVector->DWC$t_dbrv_data[0];
	for (i=0; i<DWC$k_db_repeats_per_vector; i++, RepeatExpr++)
	{
	    if (!(RepeatExpr->DWC$b_dbre_flags & DWC$m_dbre_used)) continue;
	    BSF(RepeatExpr->DWC$l_dbre_baseday);
	    BSF(RepeatExpr->DWC$l_dbre_endday);
	    BSF(RepeatExpr->DWC$w_dbre_basemin);
	    BSF(RepeatExpr->DWC$w_dbre_endmin);
	    BSF(RepeatExpr->DWC$l_dbre_repeat_interval);
	    BSF(RepeatExpr->DWC$l_dbre_repeat_interval2);
	    BSF(RepeatExpr->DWC$l_dbre_exceptions);
	    BSF(RepeatExpr->DWC$w_dbre_duration);
	}
	break;
    case DWC$k_db_repeat_expr_block:
	RepeatBlock = (struct DWCDB_repeat_block *) Buffer;
	BSF(RepeatBlock->DWC$w_dbrb_ext_count);
	BSF(RepeatBlock->DWC$l_dbrb_flink);
	BSF(RepeatBlock->DWC$w_dbrb_size);
	BSF(RepeatBlock->DWC$w_dbrb_text_class);
	for (i = 0; i<RepeatBlock->DWC$t_dbrb_data[1]; i++)
	{
	    BSF(RepeatBlock->DWC$t_dbrb_data[i*2+2]);
	}
	break;
    case DWC$k_db_repeat_exception_vec:
	RepeatExceptions = (struct DWCDB_repeat_exceptions *) Buffer;
	BSF(RepeatExceptions->DWC$l_dbrx_flink);
	for (i=0; i<DWC$k_db_max_exceptions; i++)
	{
	    BSF(RepeatExceptions->DWC$l_dbrx_exvec[i]);
	}
	break;
    }
}

void DWC$$DB_Byte_swap_field
#if defined(_DWC_PROTO_)
	(
	dwcaddr_t   Field,
	int	    size
	)
#else
	(Field, size)
	dwcaddr_t   Field;
	int	    size;
#endif
{
    char    *field = (char *)Field;
    char    temp[8];

    switch (size)
    {
    case 8:
	temp[0]=field[7];
	temp[1]=field[6];
	temp[2]=field[5];
	temp[3]=field[4];
	temp[4]=field[3];
	temp[5]=field[2];
	temp[6]=field[1];
	temp[7]=field[0];
	field[0]=temp[0];
	field[1]=temp[1];
	field[2]=temp[2];
	field[3]=temp[3];
	field[4]=temp[4];
	field[5]=temp[5];
	field[6]=temp[6];
	field[7]=temp[7];
	break;
    case 4:
	temp[0]=field[3];
	temp[1]=field[2];
	temp[2]=field[1];
	temp[3]=field[0];
	field[0]=temp[0];
	field[1]=temp[1];
	field[2]=temp[2];
	field[3]=temp[3];
	break;
    case 3:
	temp[0]=field[2];
	temp[1]=field[1];
	temp[2]=field[0];
	field[0]=temp[0];
	field[1]=temp[1];
	field[2]=temp[2];
	break;
    case 2:
	temp[0]=field[1];
	temp[1]=field[0];
	field[0]=temp[0];
	field[1]=temp[1];
	break;
    }
}

#if defined(DWC$NOFSYNC)
int DWC$$DB_fsync
#ifdef	_DWC_PROTO_
	(
	struct DWC$db_access_block	*Cab)
#else	/* no prototypes */
	(Cab)
	struct DWC$db_access_block	*Cab;
#endif	/* prototypes */

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine flushes the file I/O buffers. This routine is needed
**	on VMS only, since the VAXCRTL does not have the "fsync" function.
**	The flush is done through very simple means --> Open/Close. The
**	routione makes sure that we have proper access the calendar file
**	after it has been reopened.
**
**  FORMAL PARAMETERS:
**
**	Cab : Calendar access block, by ref
**	
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  COMPLETION CODES:
**
**	TRUE	    - Flush Ok
**	FALSE	    - Failed to flush
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/
{
    /*
    **  Setup error context, in case we fail
    */
    _Set_cause(DWC$_FLUSHF);

    /*
    **  Close the calendar file (to force a flush)
    */
    if (fclose(Cab->DWC$l_dcab_fd) != 0)
    {
	_Record_error;
	_Signal(DWC$_BADCLOSE);
	return (FALSE);
    }

    /*
    **  Try and open the file the same way as it was opened before. Use
    **	different file attributes for read and write. If this cannot
    **	be granted, signal an error.
    */
    if (Cab->DWC$l_dcab_flags & DWC$m_dcab_write)
    {
#if defined(VMS)
	Cab->DWC$l_dcab_fd = fopen
	    (Cab->DWC$a_dcab_filename, "r+b", DWC$t_db_file_attributes);
#else
	Cab->DWC$l_dcab_fd = fopen (Cab->DWC$a_dcab_filename, "r+b");
#endif
	if (Cab->DWC$l_dcab_fd == NULL)
	{
	    _Record_error;
	    _Signal(DWC$_OPENF);
	    return (FALSE);
	}
    }
    else
    {
#if defined(VMS)
	Cab->DWC$l_dcab_fd = fopen
	    (Cab->DWC$a_dcab_filename, "rb", DWC$t_db_file_attributes_read);
#else
	Cab->DWC$l_dcab_fd = fopen (Cab->DWC$a_dcab_filename, "rb");
#endif
	if (Cab->DWC$l_dcab_fd == NULL)
	{
	    _Record_error;
	    _Signal(DWC$_OPENF);
	    return (FALSE);
	}
    }	
    
    /*
    **  The flush was successful. Tell caller.
    */
    _Pop_cause;
    return (TRUE);
}
#endif

/*
**  End of Module
*/
