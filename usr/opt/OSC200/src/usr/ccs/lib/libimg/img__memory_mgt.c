
/***************************************************************************** 
**
**  Copyright (c) Digital Equipment Corporation, 1990 All Rights Reserved.
**  Unpublished rights reserved under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary to and embodies the 
**  confidential technology of Digital Equipment Corporation. Possession, use,
**  duplication or dissemination of the software and media is authorized only 
**  pursuant to a valid written license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the 
**  U.S. Government is subject to restrictions as set forth in 
**  Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**
*****************************************************************************/

/************************************************************************
**  IMG__MEMORY_MGT.C
**
**  FACILITY:
**
**	Image Services Library
**
**  ABSTRACT:
**
**	Memory management services to replace C malloc(), calloc(),
**	free(), and cfree(), and to replace STR$xxx_DX() functions
**	that are useful.
**
**  ENVIRONMENT:
**
**	VAX/VMS, VAX/ULTRIX, RISC/ULTRIX
**
**  AUTHOR(S):
**
**	Mark W. Sornson
**
**  CREATION DATE:
**
**	22-JUN-1987
**
**  MODIFICATION HISTORY:
**
************************************************************************/

/*
**  Table of contents:
**
**	Global Routines
*/
#ifdef NODAS_PROTO
void		 ImgResetMemoryMgt();
void		 ImgSetMemoryMgtToCda();

char		*_ImgAlloc();
char		*_ImgCalloc();
void		 _ImgCfree();
void		 _ImgCopyDx();
void		 _ImgDealloc();
void		 _ImgFree();
int		 _ImgFree_VM();
void		 _ImgFree1_DX();
int		 _ImgGetVM();
void		 _ImgGet1_DX();
void		 _ImgInitMemoryMgt();
char		*_ImgMalloc();
char		*_ImgRealloc();
void		 _ImgRestoreMemoryMgt();
void		 _ImgSetMemoryMgt();
#endif

/*
**	Module local routines underneath the ISL veneer, used
**	when ISL is to perform memory mgt, rather than the
**	system RTLs.
*/
#ifdef NODAS_PROTO
CDAaddress	IMG__CALLOC();
CDAstatus	IMG__CFREE();
CDAstatus	IMG__FREE();
CDAstatus	IMG__FREE_VM();
CDAstatus	IMG__GET_VM();
CDAaddress	IMG__MALLOC();
CDAaddress	IMG__REALLOC();
#endif

/*
**	Module local routines available only under VMS
*/
static long Select_memmgt();

#if defined(__VMS) || defined(VMS)
static long _ImgGetAutoTuneSize();
static void _ImgInitializeAutoTune();

void	     ImgShowAutoTuneHistogram();
#endif



/*
**  Include files:       
**
**	Standard include files from SYS$LIBRARY
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include        <img/ChfDef.h>
#include	<img/ImgDef.h>
#include	<ImgMacros.h>
#ifndef NODAS_PROTO
#include <imgprot.h>	    /* Img prototypes */
#endif

#if defined(__VMS) || defined(VMS)
#include	<lnmdef.h>	    /* for VMS only: no Ultrix support	*/
#include	<ddif$def.h>
#include        <cda$ptp.h>
#else
#if defined(NEW_CDA_SYMBOLS)
#include	<ddifdef.h>
#else
#include	<ddif_def.h>
#endif
#if defined(NEW_CDA_CALLS)
#include	<cdaptp.h>
#else
#include	<cda_ptp.h>
#endif
#endif

#include	<stdio.h>

/*                                            
**  MACRO definitions:
**
**	NOTE: all macros are defined in this module to make it as
**	independant from the rest of the ISL library as possible.
*/

#define BLOCK_SIZE_( blk )	*((int *)blk)

#define FIRST_FLB_BY_SIZE_()	flist_by_size.LhdA_Flink

#define FIRST_RLB_BY_SIZE_()	rlist_by_size.LhdA_Flink

#define	NEXT_FLIST_BLK_BY_ADR( cur_blk ) \
	    ((int)(cur_blk) != (int) flist_by_adr.LhdA_Blink)? \
	    ((struct FLB*)(cur_blk))->FLB_A_FLINK_BY_ADR: \
	    0

#define	NEXT_FLIST_BLK_BY_SIZE_( cur_blk ) \
	    ((int)(cur_blk) != (int) flist_by_size.LhdA_Blink)? \
	    ((struct FLB *)(cur_blk))->FLB_A_FLINK_BY_SIZE: \
	    0
/* this macros is not currently use 3-sep-88 */
#define	NEXT_RLIST_BLK_BY_ADR_( cur_blk ) \
	    ((int)(cur_blk ) != (int) rlist_by_adr.LhdA_Blink)? \
	    ((struct RLB*)(cur_blk))->RLB_A_FLINK_BY_ADR:  \
	    0

#define	NEXT_RLIST_BLK_BY_SIZE_( cur_blk ) \
	    ((int)(cur_blk ) != (int) rlist_by_size.LhdA_Blink)? \
	    ((struct RLB*)(cur_blk))->RLB_A_FLINK_BY_SIZE:  \
	    0

#define	NEXT_ULIST_BLK_BY_ADR_( cur_blk ) \
	    ((int)(cur_blk) != (int) ulist_by_adr.LhdA_Blink)? \
	    ((struct FLB*)(cur_blk))->FLB_A_FLINK_BY_ADR: \
	    0

#define	NEXT_ULIST_BLK_BY_SIZE_( cur_blk ) \
	    ((int)(cur_blk) != (int) ulist_by_size.LhdA_Blink)? \
	    ((struct FLB*)(cur_blk))->FLB_A_FLINK_BY_SIZE: \
	    0

#define PAGE_BOUNDARY_TEST_( memadr ) ( ((int)(memadr)) & 511)==0? TRUE: FALSE

#define ROUNDUP_PAGESIZE_( size ) ((((size) + 511) >> 9) << 9)

#define LOWBIT_TEST_AND_SIGNAL_( status )        if ( (status&1) != 1 ) \
    ChfStop( 1,  status )

#define IF_NOT_SET_( flag ) if ( (flag) == FALSE )

#define I_PRINT_( memadr, user_size, rtn_name )			\
		    if ( ImgL_MemTraceOutputFile )		\
			{					\
			i_print( (char *)memadr, (long)user_size, (char *)rtn_name );	\
			}
/*
**  Equated Symbols:
**
**  FALSE and TRUE are conditionally defined under Ultrix, since the ULTRIX
**  version of STDIO.H does not define them.
*/
#if !defined(VMS) && !defined(__VMS)
#define	FALSE	0
#define	TRUE	1
#endif

/*
** from ssdef.h
*/
#define SS_WASSET 9

/*
** Literals used under VMS to determine which memory mgt to use.
*/
#define	IMG_K_MEMMGT_ISL_DEFAULTED  1
#define	IMG_K_MEMMGT_ISL	    2
#define	IMG_K_MEMMGT_CDA1	    3
#define	IMG_K_MEMMGT_CDA2	    4
#define	IMG_K_MEMMGT_LIB	    5

/*
** This constant is the length of scratch space, in bytes,
** that is added to the end of every request for dynamic memory.
** The scratch space is needed to keep ISL applications (that 
** read or write longwords in the process of doing variable length
** bitfield access) from accessing non-existant memory past the
** end of a buffer of image data.
*/
#define	IMG_K_SCRATCH_SPACE_EXTENDER 4

/*
**	Structure Definitions and constants from descrip.h
*/                                           
struct	dsc_descriptor
        {
        unsigned short	dsc_w_length;	/* specific to descriptor class;  typically a 16-bit (unsigned) length */
        unsigned char	dsc_b_dtype;	/* data type code */
        unsigned char	dsc_b_class;	/* descriptor class code */
        char		*dsc_a_pointer;	/* address of first byte of data element */
        };

#define DSC_K_DTYPE_T	14		/* character-coded text;  a single character or a string */
#define DSC_K_CLASS_D	2		/* dynamic string descriptor */
#define DSC_K_CLASS_S	1		/* scalar or string descriptor */

/*
** Free-list Block (FLB) [free-list block header]
**
**	Note that the last field, the user-space field,
**	has no fixed size.
*/
struct	FLB {                  
	int	    		 FLB_L_LENGTH;
	struct {                
		unsigned	 FLB_V_RLIST_MEMBER:	1;
		unsigned	 FLB_V_FLIST_MEMBER:	1;
		unsigned	 FLB_V_ULIST_MEMBER:	1;
		unsigned	 FLB_V_PAGE_BEGIN:	1;
		unsigned	 FLB_V_PAGE_END:	1;
		unsigned	 FLB_V_DZERO_CLRD:	1;
		unsigned	 FLB_V_FIRST_LSTBLK:	1;
		unsigned	 FLB_V_LAST_LSTBLK:	1;
		unsigned	 FLB_V_NODELETE:	1;
		unsigned	 :	    		7;
	    	}		 FLB_W_FLAGS;
	char			 FLB_B_TYPE;
	char			 FLB_B_PADCNT;
	struct	FLB		*FLB_A_FLINK_BY_SIZE;
  	struct	FLB		*FLB_A_BLINK_BY_SIZE;
	struct	FLB		*FLB_A_FLINK_BY_ADR;
	struct	FLB		*FLB_A_BLINK_BY_ADR;
	};

/*
** List head structure (LHD)
**
*struct	LHD {
*	struct	LHD	*LhdA_Flink;
*	struct	LHD	*LhdA_Blink;
*	short		 LhdW_ListCnt;
*	unsigned	 : 16;
*	};
*/

/*
** Memory mgt context (MCT)
*/
struct	MEMCTX {
	long		MEMCTX_A_ALLOC_RTN;
	long		MEMCTX_A_DEALLOC_RTN;
	long		MEMCTX_L_PARAM;
	};

/*
** Returned-list Block (RLB)
**
**	Note that the block header has the same fields
**	as the FLB, but that an address range field is
**	included.
*/
struct	RLB {
	int			 RLB_L_LENGTH;
	struct {                
		unsigned	 RLB_V_RLIST_MEMBER:	1;
		unsigned	 RLB_V_FLIST_MEMBER:	1;
		unsigned	 RLB_V_ULIST_MEMBER:	1;
		unsigned	 RLB_V_PAGE_BEGIN:	1;
		unsigned	 RLB_V_PAGE_END:	1;
		unsigned	 :	    	    	11;
		}		 RLB_W_FLAGS;
	char			 RLB_B_TYPE;
	char			 RLB_B_PADCNT;
	struct	RLB		*RLB_A_FLINK_BY_SIZE;
  	struct	RLB		*RLB_A_BLINK_BY_SIZE;
	struct	RLB		*RLB_A_FLINK_BY_ADR;
	struct	RLB		*RLB_A_BLINK_BY_ADR;     
	int		 	 RLB_A_FIRST_BYTE;
	int		 	 RLB_A_LAST_BYTE;
	int			 RLB_L_PAGESET_SIZE;
	unsigned		 :			32;	/* padding */
	};             

/*
** Module wide state flags
*/
struct	STF {
	unsigned	STF_V_INITIALIZED:	1;
	unsigned	STF_V_CDA_MEMMGT_INIT:	1;
	unsigned	STF_V_CDA_MEMMGT_ISL:	1;
	unsigned	:			29;
	};

/*
** Typedefs
*/
typedef struct LHD	LISTHEAD;  	/* free-list and reserved-list */
typedef struct STF	STATEFLAGS;

/*
** Constant Definitions
*/
#define DSC_K_CLASS_Z		0

#define	IMG_K_MAX_FLB_SIZE	1024		/* 2 blocks, max free-list  */
						/* block size		    */
#define	FLB_K_SIZE		sizeof( struct FLB )
#define	RLB_K_SIZE		sizeof( struct RLB )

#define	HIGHEST_P0_ADDRESS	0x3FFFFFFF
#define	ZERO_FILL_CHAR		0

#define	IMG_K_DISABLE_AST	0		/* passed to SYS$SETAST()   */
#define	IMG_K_ENABLE_AST	1
						/* block-type codes	    */
#define	IMG_K_TYPE_FLB		1		/* -free-list block	    */
#define	IMG_K_TYPE_RLB		2		/* -returned-list block	    */
#define	IMG_K_TYPE_USB		3		/* -user block		    */

#define SIZEOF_LONGWORD		sizeof( long )

/*
**  External References:
**
**	From CHF$OBJLIB
*/
#ifdef NODAS_PROTO
void	ChfSignal();
void	ChfStop();
#endif

/*
**	From VMS System Services
*/
#if defined(__VMS) || defined(VMS)
long	sys$cretva();			/* create virtual address space	    */
long	sys$deltva();			/* delete virtual address space	    */
long	sys$expreg();			/* extend program region (get vm)   */
long	sys$setast();			/* enable and disable ASTs	    */
long	sys$trnlnm();			/* translate logical name	    */

char	*LIB$VM_CALLOC();
void	 LIB$VM_CFREE();
void	 LIB$VM_FREE();
char	*LIB$VM_MALLOC();
char	*LIB$VM_REALLOC();

#endif

/*
**	Most CDA Toolkit routines are defined in <cdaptp.h>
*/
unsigned long CDA_EXCHANGE_ALLOCATION_();

/*
**	From Image Core Services
*/
#ifdef NODAS_PROTO
long	_IpsInitMemoryTable();

/*
**	From ISL
*/
char	*ImgAllocateDataPlane();
char	*ImgAllocDataPlane();
void	 ImgFreeDataPlane();
char	*ImgReallocateDataPlane();
#endif


/*
**	Module local routines
*/
#ifdef NODAS_PROTO
void _ImgCopyDx();
void _ImgFree1_DX();
void _ImgGet1_DX();

static void		 Add_blk_to_flist();		/* Add block to free-list */
static void		 Add_blk_to_rlist();
static void		 Add_blk_to_ulist();
static struct 	FLB	*Allocate_memory();
static struct	RLB	*Allocate_rlb();
static struct	RLB	*Extract_rlist_blk();
static struct	FLB	*Extract_user_blk();
static struct	FLB   	*Find_flist_best_fit();
static struct	RLB	*Find_rlist_best_fit();
static void   		 Free_memory();
static void   		 Free_pageset();
static struct	FLB	*Getblk_from_expreg();		/* Get blk from expanded program region */
static struct	FLB	*Getblk_from_flist();		/* Get blk from free-list */
static struct	FLB	*Getblk_from_p0reg();		/* Get blk from P0 region */
static struct	FLB	*Getblk_from_rlist();		/* Get blk from returned-list */
static struct	FLB	*Initialize_flb();
static void   		 Initialize_imm(); 
static struct	FLB	*Initialize_p0_flb();
static int		 Largest_flb_size();
static int		 Largest_rlb_size();
static void		 Link_flist_blk_by_adr();
static void		 Link_flist_blk_by_size();
static void		 Link_rlist_blk_by_adr();
static void		 Link_rlist_blk_by_size();
static void		 Link_ulist_blk_by_adr();
static void		 Link_ulist_blk_by_size();
static struct	FLB	*Merge_flist_block();
static struct	FLB   	*Merge_flist_blocks();
static int		 Merge_flist_test();
static struct	RLB	*Merge_rlist_block();
static struct	RLB   	*Merge_rlist_blocks();
static int		 Merge_rlist_test();
static int		 Pageset_test();
static struct	FLB	*Realloc_down();
static struct	FLB	*Reclaim_p0_flb();
static int    		 Roundup_blocksize();
static struct 	FLB	*Unlink_flist_blk();
static struct 	FLB	*Unlink_flist_blk_by_adr();
static struct 	FLB	*Unlink_flist_blk_by_size();
static struct 	RLB	*Unlink_rlist_blk();
static struct 	RLB	*Unlink_rlist_blk_by_adr();
static struct 	RLB	*Unlink_rlist_blk_by_size();
static struct	FLB	*Unlink_ulist_blk();
static struct	FLB	*Unlink_ulist_blk_by_adr();
static struct	FLB	*Unlink_ulist_blk_by_size();
static void   		 Verify_block_type();
static void		 Verify_flb();
static void		 Verify_flist();
static void		 Verify_rlist();
static void		 Verify_ulist();
static void		 i_print ();

static CDAaddress IMG__CALLOC ();
static CDAstatus IMG__CFREE ();
static CDAstatus IMG__FREE ();
static CDAstatus IMG__FREE_VM ();
static CDAstatus IMG__GET_VM ();
static CDAaddress IMG__MALLOC ();
static CDAaddress IMG__REALLOC ();
#else
PROTO(void _ImgCopyDx, (struct dsc_descriptor */*dststr*/, struct dsc_descriptor */*srcstr*/));
PROTO(void _ImgFree1_DX, (struct dsc_descriptor */*memdsc*/));
PROTO(void _ImgGet1_DX, (short */*len*/, struct dsc_descriptor */*memdsc*/));

PROTO(static void Add_blk_to_flist, (struct FLB */*new_blk*/));
PROTO(static void Add_blk_to_rlist, (struct RLB */*rlb*/));
PROTO(static void Add_blk_to_ulist, (struct FLB */*user_blk*/));
PROTO(static struct FLB *Allocate_memory, (int /*size*/));
PROTO(static struct RLB *Allocate_rlb, (void));
PROTO(static struct RLB *Extract_rlist_blk, (struct RLB */*rlb*/, int /*size*/));
PROTO(static struct FLB *Extract_user_blk, (struct FLB */*src_blk*/, int /*size*/));
PROTO(static struct FLB *Find_flist_best_fit, (int /*size*/));
PROTO(static struct RLB *Find_rlist_best_fit, (int /*size*/));
PROTO(static void Free_memory, (struct FLB */*flb*/));
PROTO(static void Free_pageset, (struct FLB */*pageset_blk*/));
PROTO(static struct FLB *Getblk_from_expreg, (int /*size*/));
PROTO(static struct FLB *Getblk_from_flist, (int /*size*/));
PROTO(static struct FLB *Getblk_from_p0reg, (int /*size*/));
PROTO(static struct FLB *Getblk_from_rlist, (int /*size*/));
PROTO(static struct FLB *Initialize_flb, (struct FLB */*flb*/, int /*size*/));
PROTO(static void Initialize_imm, (void));
PROTO(static struct FLB *Initialize_p0_flb, (struct FLB */*pageset*/, int /*size*/));
PROTO(static int Largest_flb_size, (void));
PROTO(static int Largest_rlb_size, (void));
PROTO(static void Link_flist_blk_by_adr, (struct FLB */*new_blk*/));
PROTO(static void Link_flist_blk_by_size, (struct FLB */*new_blk*/));
PROTO(static void Link_rlist_blk_by_adr, (struct RLB */*new_blk*/));
PROTO(static void Link_rlist_blk_by_size, (struct RLB */*new_blk*/));
PROTO(static void Link_ulist_blk_by_adr, (struct FLB */*new_blk*/));
PROTO(static void Link_ulist_blk_by_size, (struct FLB */*new_blk*/));
PROTO(static struct FLB *Merge_flist_block, (struct FLB */*new_blk*/));
PROTO(static struct FLB *Merge_flist_blocks, (struct FLB */*first_blk*/, struct FLB */*second_blk*/));
PROTO(static int Merge_flist_test, (struct FLB */*first_blk*/, struct FLB */*second_blk*/));
PROTO(static struct RLB *Merge_rlist_block, (struct RLB */*new_blk*/));
PROTO(static struct RLB *Merge_rlist_blocks, (struct RLB */*first_blk*/, struct RLB */*second_blk*/));
PROTO(static int Merge_rlist_test, (struct RLB */*first_blk*/, struct RLB */*second_blk*/));
PROTO(static int Pageset_test, (struct FLB */*memblk*/));
PROTO(static struct FLB *Realloc_down, (char */*userbuf_adr*/, int /*resize*/));
PROTO(static struct FLB *Reclaim_p0_flb, (struct RLB */*rlb*/));
PROTO(static int Roundup_blocksize, (int /*size*/));
PROTO(static struct FLB *Unlink_flist_blk, (struct FLB */*flist_blk*/));
PROTO(static struct FLB *Unlink_flist_blk_by_adr, (struct FLB */*flist_blk*/));
PROTO(static struct FLB *Unlink_flist_blk_by_size, (struct FLB */*flist_blk*/));
PROTO(static struct RLB *Unlink_rlist_blk, (struct RLB */*rlist_blk*/));
PROTO(static struct RLB *Unlink_rlist_blk_by_adr, (struct RLB */*rlist_blk*/));
PROTO(static struct RLB *Unlink_rlist_blk_by_size, (struct RLB */*rlist_blk*/));
PROTO(static struct FLB *Unlink_ulist_blk, (struct FLB */*ulist_blk*/));
PROTO(static struct FLB *Unlink_ulist_blk_by_adr, (struct FLB */*ulist_blk*/));
PROTO(static struct FLB *Unlink_ulist_blk_by_size, (struct FLB */*ulist_blk*/));
PROTO(static void Verify_block_type, (struct FLB */*block*/, int /*type*/));
PROTO(static void Verify_flb, (struct FLB */*flb*/));
PROTO(static void Verify_flist, (void));
PROTO(static void Verify_rlist, (void));
PROTO(static void Verify_ulist, (void));
PROTO(static long Select_memmgt, (void));

PROTO(static CDAaddress IMG__CALLOC, (CDAsize /*number*/, CDAsize /*size*/));
PROTO(static CDAstatus IMG__CFREE, (CDAaddress /*memptr*/));
PROTO(static CDAstatus IMG__FREE, (CDAaddress /*memptr*/));
PROTO(static CDAstatus IMG__FREE_VM, (long */*num_bytes*/, char **/*base_adr*/));
PROTO(static CDAstatus IMG__GET_VM, (long */*num_bytes*/, char **/*base_adr*/, long /*param*/));
PROTO(static CDAaddress IMG__MALLOC, (CDAsize /*size*/));
PROTO(static CDAaddress IMG__REALLOC, (CDAaddress /*oldbuf*/, CDAsize /*size*/));
PROTO(static void i_print, (char * /* memadr */ ,long /* user_size */,char * /* rtn_name */ ));
#endif


/*
**  Message status codes (globally defined in BIN$:IMG$STATUS_CODES.OBJ)
**  [Also see SRC$:IMG$STATUS_CODES.MSG and LIB$:IMG$STATUS_CODES.H.]
*/
#include	<img/ImgStatusCodes.h>

/*
**  Local Storage:
**
**	Module wide static storage.  Note that these are all
**	non-sharable.  Therefore, if ISL is built as a sharable
**	library, all images will receive their own copy of 
**	these data areas.  
*/
#if defined(__VAXC) || defined(VAXC)
#define Static_data globaldef { "IMM$DATA" } noshare
#else
#define Static_data  static 
#endif

/*
** Pointers to designate the memory mgt scheme to be used by the
** DDIF toolkit (which requires routines to conform to the LIB$xxx_VM
** call interface).  These pointers are filled in either the first time 
** ISL memory mgt services are called, or when an explicit call is made
** to the exchange mem mgt service.
*/
#if defined(__VMS) || defined(VMS)
Static_data CDAstatus	(*IMG_A_FREE_VM)();
Static_data CDAstatus	(*IMG_A_GET_VM)();	
Static_data CDAuserparam  IMG_L_MEMMGT_PARAM;

Static_data CDAaddress	(*IMG_A_CALLOC)();
Static_data CDAstatus	(*IMG_A_CFREE)();
Static_data CDAstatus	(*IMG_A_FREE)();
Static_data CDAaddress	(*IMG_A_MALLOC)();
Static_data CDAaddress	(*IMG_A_REALLOC)();
#else
CDAuserparam	  IMG_L_MEMMGT_PARAM;
#ifdef NODASPROTO
CDAstatus	(*IMG_A_FREE_VM)();
CDAstatus	(*IMG_A_GET_VM)();	
CDAaddress	(*IMG_A_CALLOC)();
CDAstatus	(*IMG_A_CFREE)();
CDAstatus	(*IMG_A_FREE)();
CDAaddress	(*IMG_A_MALLOC)();
CDAaddress	(*IMG_A_REALLOC)();
#else
CDAstatus	(*IMG_A_FREE_VM)(long */*num_bytes*/, char **/*base_adr*/);
CDAstatus	(*IMG_A_GET_VM)(long */*num_bytes*/, char **/*base_adr*/, long /*param*/);
CDAaddress	(*IMG_A_CALLOC)(CDAsize /*number*/, CDAsize /*size*/);
CDAstatus	(*IMG_A_CFREE)(CDAaddress /*memptr*/);
CDAstatus	(*IMG_A_FREE)(CDAaddress /*memptr*/);
CDAaddress	(*IMG_A_MALLOC)(CDAsize /*size*/);
CDAaddress	(*IMG_A_REALLOC)(CDAaddress /*oldbuf*/, CDAsize /*size*/);
#endif
#endif

Static_data struct LHD	flist_by_size;		/* free-list list heads */
Static_data LISTHEAD	flist_by_adr;
Static_data LISTHEAD	rlist_by_size;		/* returned-list list heads */
Static_data LISTHEAD	rlist_by_adr;
Static_data LISTHEAD	ulist_by_size;		/* user-list list heads */
Static_data LISTHEAD	ulist_by_adr;

Static_data STATEFLAGS state_flags;		/* module-wide state flags */

Static_data char	*lowest_page_allocd;
Static_data char	*highest_page_allocd;
Static_data char	*highest_byte_allocd;

Static_data int	 max_flist_blksiz;
Static_data int	 max_rlist_blksiz;
Static_data int	 min_flist_blksiz;
Static_data int	 min_rlist_blksiz;

Static_data struct RLB highest_rlb_by_adr;	/* used in merge functions */

Static_data int	 watch_block;
Static_data int	 watch_block_found;

#if defined(__VMS) || defined(VMS)
/*
**  More Equated Symbols:
**
**  ABSOLUTE_LOWEST_MEM_INDEX
**
**  The value for the absolute lowest index is arbitrary, and is the
**  cut-off free-list index below which memory is simply passed right
**  through to the RTL.
**
**  ABSOLUTE_MAX_MEM_INDEX
**
**  The index value 255 corresponds to the highest size, in bytes, of
**  virtual memory that there is a free-list for.  This value is
**  actually 2**31 - 1 bytes (or 4194304 blocks).
**
*/
#define	ABSOLUTE_LOWEST_MEM_INDEX   111
#define	ABSOLUTE_MAX_MEM_INDEX	    255

/*
**  DEFAULT_LO_MEM_INDEX
**
**  The index value 136 corresponds to the size, in blocks, that is
**  one block larger than the maximum heap sized used by LIB$MALLOC.
**  (This means that requests that are larger than the max heap
**  size are gotten directly from the operating system.)  This is
**  (2 ** 16) + 1
**
**  DEFAULT_HI_MEM_INDEX
**
**  The index value 248 is an arbitrary value that is the last number
**  that is exactly (8 * n) units away from the DEFAULT_LOW_MEM_INDEX
**  value of 136.  (The next index number, 256, is too high to be
**  actually used.)
**
**  DEFAULT_MEM_TUNING_FACTOR
**
**  The value 14 was chosen to divide the range between the default
**  low and high memory indexes so that each quantization step falls
**  on a power of 2 (+1 block) boundary.
*/
#define DEFAULT_LO_MEM_INDEX	    136
#define DEFAULT_HI_MEM_INDEX	    248
#define	DEFAULT_MEM_TUNING_FACTOR    14

/*
** OVERHEAD
**
**  This is the size in bytes of the header that the VMS LIB$VM_MALLOC
**  type routines prepend to allocated memory
*/
#define OVERHEAD		     16

/*
**  Local Storage:
**
**	VMS specific variables and tables for tuning memory management.
*/
Static_data char	 ImgA_AutoTuneHistOutputFile[512];
Static_data char	*ImgA_AutoTuneHistTriggerFile	    = 0;
Static_data long	 ImgL_AutoTune			    = 0;
Static_data long	 ImgL_AutoTuneDebug		    = 0;
Static_data long	 ImgL_AutoTuneDebugShowTable	    = 0;
Static_data long	 ImgL_AutoTuneDebugShowHistogram    = 0;
Static_data long	 ImgL_AutoTuneDebugShowTrace	    = 0;
Static_data long	 ImgL_AutoTuneInitialized	    = 0;
Static_data long	 ImgL_HiMemIndex;
Static_data long	 ImgL_HiMemSize;
Static_data long	 ImgL_LoMemIndex;
Static_data long	 ImgL_LoMemSize;
Static_data long	 ImgL_LookForHistFile		    = 0;
Static_data long	*ImgR_MemRoundupTable		    = 0;
Static_data long	 ImgL_TuningFactor;
Static_data long	 ImgL_TuningPointIndex;

Static_data double	 ImgD_TuningBias;

Static_data FILE	*ImgL_AutoTuneHistOutputFile;
Static_data FILE	*ImgL_AutoTuneOutputFile;

/*
** Histogram arrays to keep track of free-list indexes that
** would be used if no auto-tuning was performed, and which
** indexes are used.
*/
static long	ImgA_ReqSizes[ 256 ];
static long	ImgA_WrkSizes[ 256 ];

/*
** Macros:
**
**  CVT_SIZE_TO_INDEX_ converts a longword integer size value
**  into an index value by converting the long into a real number,
**  and then extracting the relevant bits from the real field.
**
**  This trick is used by the LIB$VM_MALLOC (VMS RTL) code to round up
**  requested memory sizes to match pre-selected sizes which then
**  fit nicely into free-lists when they are returned.
**
**  NOTE that its dependency on the hardware format for real numbers
**  makes this particular trick VAX only.
*/
#define CVT_SIZE_TO_INDEX_( SIZE, INDEX ) \
             { \
                 union u_cvt \
                 { \
                     double toFloat; \
                     struct \
                     { \
                         unsigned int a:4; \
                         unsigned int index:10; \
                         unsigned int b:18; \
                     } toBits; \
                } cvt; \
              \
                 cvt.toFloat = (SIZE) - 1 + OVERHEAD; \
                 INDEX       = cvt.toBits.index; \
             }
#endif

/*
** Platform independent variable for memory-leak tracing.
*/
Static_data FILE	*ImgL_MemTraceOutputFile	    = 0;
Static_data long	 ImgL_MemTraceInitialized	    = 0;
Static_data long	 ImgL_MemTraceId		    = 0;


/******************************************************************************
**  ImgResetMemoryMgt()
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function will undo the effects of a call to 
**	ImgSetCdaMemoryMgt, restoring the internal ISL memory mgt 
**	functions as those to be used in conjunction DDIF/CDA function
**	calls.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void ImgResetMemoryMgt()
{
long	scheme;

scheme = Select_memmgt();
switch ( scheme )
    {
    case IMG_K_MEMMGT_CDA1:
	ImgSetMemoryMgtToCda();
	IMG_A_CALLOC	    = CdaCalloc;
	IMG_A_CFREE	    = CdaFree;
	IMG_A_FREE	    = CdaFree;
	IMG_A_MALLOC	    = CdaMalloc;
	IMG_A_REALLOC	    = CdaRealloc;
	break;

    case IMG_K_MEMMGT_CDA2:
	ImgSetMemoryMgtToCda();
	IMG_A_CALLOC	    = IMG__CALLOC;
	IMG_A_CFREE	    = IMG__CFREE;
	IMG_A_FREE	    = IMG__FREE;
	IMG_A_MALLOC	    = IMG__MALLOC;
	IMG_A_REALLOC	    = IMG__REALLOC;
	break;

    case IMG_K_MEMMGT_LIB:
    case IMG_K_MEMMGT_ISL_DEFAULTED:
	IMG_A_GET_VM	    = IMG__GET_VM;
	IMG_A_FREE_VM	    = IMG__FREE_VM;
	IMG_L_MEMMGT_PARAM  = 0;
	IMG_A_CALLOC	    = CdaCalloc;
	IMG_A_CFREE	    = CdaFree;
	IMG_A_FREE	    = CdaFree;
	IMG_A_MALLOC	    = CdaMalloc;
	IMG_A_REALLOC	    = CdaRealloc;
	break;

    case IMG_K_MEMMGT_ISL:
	IMG_A_GET_VM	    = IMG__GET_VM;
	IMG_A_FREE_VM	    = IMG__FREE_VM;
	IMG_L_MEMMGT_PARAM  = 0;
	IMG_A_CALLOC	    = IMG__CALLOC;
	IMG_A_CFREE	    = IMG__CFREE;
	IMG_A_FREE	    = IMG__FREE;
	IMG_A_MALLOC	    = IMG__MALLOC;
	IMG_A_REALLOC	    = IMG__REALLOC;
	break;

    default:
	break;
    }

state_flags.STF_V_CDA_MEMMGT_INIT = TRUE;
state_flags.STF_V_CDA_MEMMGT_ISL = TRUE;

return;
} /* end of ImgResetMemoryMgt */


/******************************************************************************
**  ImgSetMemoryMgtToCda()
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function will associate a user-supplied set of memory mgt
**	functions with ISL that will be used to allocate and deallocate
**	memory for use as DDIF/CDA aggregate memory.  This will allow 
**	applications to control the memory mgt functions used to allocate 
**	memory for ISL frames, and for ISL/DDIF file IO.
**
**	This function, when invoked, will establish the user routines
**	as the defaults passed into CDA function calls that ISL makes
**	until it is called again by the user to establish some other
**	memory mgt functions.
**
**	If never called, ISL will establish _ImgGetVM and _ImgFree_VM as
**	the defaults.
**
**  FORMAL PARAMETERS:
**
**	get_vm_rtn	Entry point (address) of allocation routine.
**	free_vm_rtn	Entry point (address) of deallocation routine.
**	user_param	Longword value to be passed to alloc-dealloc
**			functions.  Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void ImgSetMemoryMgtToCda()
{
CDAaggtype aggr_type;
CDAstatus (*default_free_vm_rtn)();
CDAstatus (*default_get_vm_rtn)();
CDAuserparam  	  default_param;
long	  status;
CDArootagghandle temp_root_aggr;

/*
** Create a temporary root aggregate and retrieve the memory mgt pointers
** that are associated with it by default.
*/
#if defined(NEW_CDA_SYMBOLS)
aggr_type = DDIF_DDF;
#else
aggr_type = DDIF$_DDF;
#endif

/*
** Create a temporary root aggregate and retrieve the memory mgt pointers
** that are associated with it by default.
*/
status = CDA_CREATE_ROOT_AGGREGATE_ (
		    0, 0, 0,		/* use default toolkit memmgt	*/
		    0,			/* no options			*/
		    &aggr_type,
		    &temp_root_aggr
		    );
LOWBIT_TEST_AND_SIGNAL_( status );

/*
** Exchange to get memory managemement
*/
status = CDA_EXCHANGE_ALLOCATION_(
		&temp_root_aggr,
		IMG_A_GET_VM,
		IMG_A_FREE_VM,
		IMG_L_MEMMGT_PARAM,
		&default_get_vm_rtn,	/* default values returned here	*/
		&default_free_vm_rtn,
		&default_param
		);
LOWBIT_TEST_AND_SIGNAL_( status );

/*
** Exchange them back
*/
status = CDA_EXCHANGE_ALLOCATION_(
		&temp_root_aggr,
		default_get_vm_rtn,	/* default values returned here	*/
		default_free_vm_rtn,
		default_param,
		&IMG_A_GET_VM,
		&IMG_A_FREE_VM,
		&IMG_L_MEMMGT_PARAM
		);
LOWBIT_TEST_AND_SIGNAL_( status );

/*
** Now set the ISL memory mgt pointers
*/
IMG_A_GET_VM	    = default_get_vm_rtn;
IMG_A_FREE_VM	    = default_free_vm_rtn;
IMG_L_MEMMGT_PARAM  = default_param;

/*
** Delete the temporary root aggregate
*/
status = CDA_DELETE_ROOT_AGGREGATE_ ( &temp_root_aggr );
LOWBIT_TEST_AND_SIGNAL_( status );

/*
** Set the appropriate state flags.
*/
state_flags.STF_V_CDA_MEMMGT_INIT = TRUE;
state_flags.STF_V_CDA_MEMMGT_ISL = FALSE;

return;
} /* end of ImgSetMemoryMgtToCda */


/******************************************************************************
**  _ImgAlloc
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate Dynamic memory.
**
**  FORMAL PARAMETERS:
**
**	size	    unsigned longword, by value
**		    Size in bytes to allocate.
**
**	flags	    unsigned longword, by value.
**		    ImgM_InitMem    Initialize each byte of allocated 
**				    memory with fill mask.
**
**		    default	    No initialization.
**
**	fill_mask   unsigned byte, by value
**		    Value to initialize each byte of allocated memory with.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	[@description_or_none@]
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
char *_ImgAlloc( size, flags, fill_mask )
unsigned long	size;
unsigned long	flags;
int		fill_mask;
{
char	*memptr;

if ( (flags&ImgM_InitMem) == 0)
    memptr = _ImgMalloc( size );
else
    if ( fill_mask == 0 )
	memptr = _ImgCalloc( size, 1 );
    else
	{
	memptr = _ImgMalloc( size );
	memset( memptr, fill_mask, size );
	}

return memptr;
} /* end of _ImgAlloc */


/******************************************************************************
**  _ImgCalloc
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate a block of memory and clear (or zero-fill) it.  This
**	function emulates the C function calloc().  This function is
**	actually a veneer on top of a call to a run-time specifiable
**	calloc function.
**
**  FORMAL PARAMETERS:
**
**	number		Number of items of a specified size (next parameter)
**			to be allocated.
**			Passed by value.
**
**      user_size	Size of memory in bytes to be allocated.
**			Passed by value.
**
**	FILL_MASK	A 8-bit pattern to fill each byte with before
**			returning.  The default pattern is 00000000, which
**			returns a cleared buffer back to the caller.
**			This parameter is optional.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      user_mem	Pointer to dynamic memory allocated.  Note that the 
**			pointer returned is the address of the first byte of 
**			user space.  The address of the block head is never 
**			returned to the caller.
**			Passed by reference.
**              
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
char *_ImgCalloc( number, user_size)
int		number;
int		user_size;
{
char	*retmem;
long	 index;
long	 working_size	= user_size * number;
long	 working_size2	= user_size * number;

_ImgInitMemoryMgt();

/*
** This #ifdef'd code rounds up requested allocation sizes to
** the values set by the AutoTuneMemory function.
*/
#ifdef VMS
if ( ! ImgL_AutoTuneInitialized )
    _ImgInitializeAutoTune();

if ( ImgL_AutoTune )
    working_size = _ImgGetAutoTuneSize( working_size );
#endif

retmem = (char *)(*IMG_A_CALLOC)( 1, working_size );
if ( retmem == 0 )
    ChfStop( 1, ImgX_MEMALLOC );

I_PRINT_( retmem, working_size2, "_ImgCalloc" );

return retmem; 
} /* end of _ImgCalloc */


/******************************************************************************
**  _ImgCfree
**
**  FUNCTIONAL DESCRIPTION:
**
**      Frees dynamic memory allocated by _ImgCalloc.  This function
**	emulates the C function cfree().
**
**	This function is a veneer on top of a run-time specifiable
**	calloc function.
**
**  FORMAL PARAMETERS:
**
**      memptr		Pointer to memory allocated by a call to _ImgCalloc.
**			Passed by value.  (Memory itself passed by ref.)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgCfree( memptr )
void	*memptr;
{

_ImgInitMemoryMgt();

I_PRINT_( memptr , 0, "_ImgCfree" );

(*IMG_A_FREE)( memptr );

return;
} /* end of _ImgCfree */


/******************************************************************************
**  _ImgCopyDx
**
**  FUNCTIONAL DESCRIPTION:
**
**      Copy the contents of a buffer pointed to by descriptor into another
**	buffer pointed to by descriptor.  This function emulates the
**	STR$COPY_DX() function in the VMS RTL.
**
**	If the target buffer doesn't exist and the class is dynamic, allocate 
**	a dynamic buffer.
**
**	If the target buffer does exist and the class is static, but is too 
**	short, truncate the data and return a warning.
**
**  FORMAL PARAMETERS:
**
**      dststr		Destination string buffer descriptor.  
**			Passed by reference.  (Alt. string is passed by
**			descriptor.)
**	srcstr		Source string buffer descriptor.
**			Passed by reference.  (Alt. string is passed by
**			descriptor.)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**	none (void)
**
**  SIGNAL CODES:
**
**      ImgX_DXCPYTRUN		Descriptor copy destination string truncated
**	ImgX_INVDSCCLA		Invalid descriptor class field value
**	ImgX_INVDSCLEN	       	Invalid descriptor length value
**	ImgX_INVDYNSRC		Invalid dynamic source descriptor
**      
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgCopyDx( dststr, srcstr )
struct	dsc_descriptor	*dststr;
struct	dsc_descriptor	*srcstr;
{
int	length;

/*
** Validate srcstr descriptor
*/
if ( srcstr->dsc_w_length == 0 || srcstr->dsc_a_pointer == NULL )
    ChfStop( 1,  ImgX_INVSRCDSC );

/*
** Select action based on destination class field
*/
switch ( dststr->dsc_b_class )
    {         
    /*
    ** Dynamic string descriptor.
    */
    case DSC_K_CLASS_D:
    	/*
    	** Deallocate existing destination memory if it is shorter in 
    	** length than the source string length.
    	*/    
    	if ( dststr->dsc_w_length != 0 &&
             dststr->dsc_a_pointer != NULL &&
    	     dststr->dsc_w_length < srcstr->dsc_w_length )
            {
            _ImgFree( dststr->dsc_a_pointer );
            dststr->dsc_a_pointer = 0;
            dststr->dsc_w_length = 0;
            }
    	/*
    	** Allocate new memory for destination and modify the
    	** destination descriptor length.  The new memory is 
    	** equal in length to the old.
    	*/
    	if ( dststr->dsc_w_length == 0 && dststr->dsc_a_pointer == NULL )
            {
            dststr->dsc_a_pointer = _ImgMalloc( srcstr->dsc_w_length );
            dststr->dsc_w_length = srcstr->dsc_w_length;
            }
    	break;

    /*
    ** Scalar or string descriptor.  (Note that zero is accepted in
    ** the class field.  It will be assumed that a valid length value
    ** will have been supplied in the length field.  This allowance
    ** has been made for programmers too lazy to set up the class and
    ** type fields of the descriptor.)
    */
    case DSC_K_CLASS_S:
    case DSC_K_CLASS_Z:
    	if ( dststr->dsc_w_length == 0 )
            ChfStop( 1,  ImgX_INVDSCLEN );		/* Invalid desc length value */
    	if ( dststr->dsc_w_length < srcstr->dsc_w_length )
            ChfSignal( 1,  ImgX_DXCPYTRUN );	/* Desc copy truncation */
    	break;

    /*
    ** All other classes are invalid.
    */
    default:
    	ChfStop( 1,  ImgX_INVDSCCLA );		/* Invalid desc class value */
    } /* end of switch */
              
/*
** Copy the data from the source to the destination.
*/
memcpy( dststr->dsc_a_pointer, srcstr->dsc_a_pointer, dststr->dsc_w_length );

return;
} /* end of _ImgCopyDx */
             

/******************************************************************************
**  _ImgDealloc
**
**  FUNCTIONAL DESCRIPTION:
**
**	Deallocate dynamic memory allocated by _ImgAlloc.
**
**  FORMAL PARAMETERS:
**
**	bufptr	    unsigned character pointer, passed by value
**		    Address of the dynamic memory buffer returned
**		    by _ImgAlloc.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgDealloc( bufptr )
unsigned char	*bufptr;
{

_ImgFree( bufptr );

return;
} /* end _ImgDealloc */


/******************************************************************************
**  _ImgFree
**
**  FUNCTIONAL DESCRIPTION:
**
**      Free dynamic memory allocated by a call to _ImgMalloc().  This
**	function emulates the C function free().
**
**	This function is a veneer on top of a run-time specifiable
**	free function.
**
**  FORMAL PARAMETERS:
**
**      memptr		Pointer to dynamic memory to be deallocated.
**			Passed by value (memory passed by ref.)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgFree( memptr )
void	*memptr;
{

_ImgInitMemoryMgt();

I_PRINT_( memptr, 0, "_ImgFree" );

(*IMG_A_FREE)( memptr );

return;
} /* end of _ImgFree */


/******************************************************************************
**  _ImgFree_VM
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free virtual memory that was allocated by a call to _ImgGetVM.
**	This routine is similar to the VMS RTL function LIB$FREE_VM, with 
**	an identical call interface.
**
**	This function is a veneer on top of a run-time specifiable
**	free_vm function.
**
**  FORMAL PARAMETERS:
**
**	num_bytes	Number of contiguous bytes to be deallocated.
**			This parameter is actually a dummy argument, since
**			the base_adr argument is really all that is needed.
**			This parameter is passed by reference.
**
**	base_adr	Address of the dynamic memory to be deallocated.
**			Note that the memory passed in MUST be the address
**			of a memory-block that was returned by a call
**			to _ImgGetVM.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgFree_VM( num_bytes, base_adr, param )
long	 *num_bytes;
char	**base_adr;
long	  param;
{
long	status;

_ImgInitMemoryMgt();

status = (*IMG_A_FREE_VM)( num_bytes, base_adr );

return status;				    /* always return success */
} /* end of _ImgFree_VM */


/******************************************************************************
**  _ImgFree1_DX
**
**  FUNCTIONAL DESCRIPTION:
**
**      Free a dynamic buffer pointed to by descriptor.  This function
**	emulates the VMS RTL function STR$FREE1_DX.
**
**  FORMAL PARAMETERS:
**
**      memdsc		Memory buffer descriptor.  
**			Passed by reference.  (Alt. memory is passed by
**			descriptor.)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgFree1_DX( memdsc )
struct	dsc_descriptor	*memdsc;
{

/*
** Free the memory and modify the descriptor contents
** to indicate that the descriptor memory has been taken away.
*/
_ImgFree( memdsc->dsc_a_pointer );
memdsc->dsc_w_length = 0;
memdsc->dsc_a_pointer = 0;

return;
} /* end of _ImgFree1_DX */


/******************************************************************************
**  _ImgGetVM
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a cleared block of dynamic (virtual) memory, using the
**	same call interface as the VMS RTL function LIB$GET_VM.
**
**	This function is a veneer on top of a run-time specifiable
**	get_vm function.
**
**  FORMAL PARAMETERS:
**
**	num_bytes	Number of contiguous bytes to be allocated.
**			This parameter is passed by reference.
**
**	base_adr	Address of a longword to receive the address of
**			the first byte of allocated memory.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
long _ImgGetVM( num_bytes, base_adr, param )
long	*num_bytes;
char	**base_adr;
long	param;
{
long	buflen;
long	status;

_ImgInitMemoryMgt();

/*
** IMPORTANT NOTE:
**
**	The constant IMG_K_SCRATCH_SPACE_EXTENDER is used because certain 
**	ISL algorithms actually access memory beyond the end of the data
**	plane when getting or putting data that actually only falls
**	within the data plane.  This is mostly the case with access
**	mechanisms that work on bitfields of arbitrary length.
**
**	It is ONLY a workaround, and will be removed once a general
**	(and fast) access mechanism is figured out to solve the problems
**	that arise when using bitfields of arbitrary length that fall
**	on arbitrary bit boundaries.
*/
buflen = *num_bytes + IMG_K_SCRATCH_SPACE_EXTENDER;
status = (*IMG_A_GET_VM)( &buflen, base_adr, param );

return status;
} /* end of _ImgGetVM */


/******************************************************************************
**  _ImgGet1_DX
**
**  FUNCTIONAL DESCRIPTION:
**
**      Get dynamic memory by descriptor.  This function emulates the VMS
**	RTL function STR$GET1_DX().
**
**  FORMAL PARAMETERS:
**
**	len		Length of dynamic memory to allocate.
**			Pass by reference.  This is a word value.
**
**      memdsc		Dynamic memory (string) descriptor.
**			Passed by reference.  [Alt. null string passed
**			by descriptor.]
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      ImgX_DSCNOTDYN		Descriptor not of class DYNAMIC.
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
void _ImgGet1_DX( len, memdsc )
short			*len;
struct	dsc_descriptor	*memdsc;
{

/*
** Verify that the descriptor class is dynamic
*/
if ( memdsc->dsc_b_class != DSC_K_CLASS_D )
    ChfStop( 1,  ImgX_DSCNOTDYN );			/* desc not dynamic */

/*
** Deallocate Existing memory
*/
if ( memdsc->dsc_w_length != 0 && memdsc->dsc_a_pointer != NULL )
    _ImgFree( memdsc->dsc_a_pointer );

/*
** Allocate memory and modify the descriptor.
*/
memdsc->dsc_a_pointer = _ImgCalloc( 1, *len );
memdsc->dsc_w_length = *len;

return;
} /* end of _ImgGet1_DX */


/******************************************************************************
**  _ImgInitMemoryMgt
**
**  FUNCTIONAL DESCRIPTION:
**
**	Global entry point to be called from other ISL modules (such as
**	Img__DATA_PLANE_UTILS) to ensure that ISL memory mgt is initialized
**	before it is used.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgInitMemoryMgt()
{

if ( !(state_flags.STF_V_INITIALIZED) )
    Initialize_imm();

return;
} /* end of _ImgInitMemoryMgt */


/******************************************************************************
**  _ImgMalloc
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate dynamic memory.  This function emulates the C function
**	malloc().
**
**	This function is a veneer on top of a run-time specifiable
**	malloc function.
**
**  FORMAL PARAMETERS:
**
**      user_size	Size, in bytes, of memory block to be allocated.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none              
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      user_mem		Pointer to dynamic memory.
**				Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**               
******************************************************************************/
char *_ImgMalloc( user_size )
int	user_size;
{
char	*retmem;
long	 index;
long	 working_size	= user_size;

_ImgInitMemoryMgt();

/*
** This #ifdef'd code rounds up requested allocation sizes to
** the values set by the AutoTuneMemory function.
*/
#ifdef VMS
if ( ! ImgL_AutoTuneInitialized )
    _ImgInitializeAutoTune();

if ( ImgL_AutoTune )
    working_size = _ImgGetAutoTuneSize( working_size );
#endif

retmem = (char *)(*IMG_A_MALLOC)( working_size );
if ( retmem == 0 )
    ChfStop( 1, ImgX_MEMALLOC );

I_PRINT_( retmem, user_size, "_ImgMalloc" );

return retmem; 
} /* end of _ImgMalloc */


/******************************************************************************
**  _ImgRealloc
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reallocate a buffer, changing its size, and copy the contents
**	of the input buffer to the new output buffer.  Delete the input
**	buffer when the copy is complete.
**
**	This function is a veneer on top of a run-time specifiable
**	realloc function.
**
**  FORMAL PARAMETERS:
**
**	oldbuf		Buffer to reallocate.
**			Passed by reference.
**
**	user_size	Size in bytes of the new buffer.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	newbuf		Reallocated buffer.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
char *_ImgRealloc( oldbuf, user_size )     
char	*oldbuf;
int	 user_size;
{
char	*retmem;
long	 index;
long	 working_size	= user_size;

_ImgInitMemoryMgt();

/*
** This #ifdef'd code rounds up requested allocation sizes to
** the values set by the AutoTuneMemory function.
*/
#ifdef VMS
if ( ! ImgL_AutoTuneInitialized )
    _ImgInitializeAutoTune();

if ( ImgL_AutoTune )
    working_size = _ImgGetAutoTuneSize( working_size );
#endif

retmem = (char *)(*IMG_A_REALLOC)( oldbuf, working_size );
if ( retmem == 0 )
    ChfStop( 1, ImgX_MEMALLOC );

return retmem; 
} /* end of _ImgRealloc */


/******************************************************************************
**  _ImgRestoreMemoryMgt
**
**  FUNCTIONAL DESCRIPTION:
**
**	Restore the memory_mgt routines that were saved in by a call
**	to _ImgSetMemoryMgt.  This function operates on the root-
**	aggregate handle being used for a DDIF IO import operation,
**	and is called just before the import operation returns control
**	to the user.
**
**  FORMAL PARAMETERS:
**
**	root_aggr	    Root aggregate handle, by value.
**	memctx		    Memory mgt context block, by reference.
**
**  IMPLICIT INPUTS:
**
**	none.
**
**  IMPLICIT OUTPUTS:
**
**	memctx		    NULL pointer returned.
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgRestoreMemoryMgt( user_root_aggr, memctx )
CDArootagghandle user_root_aggr;
void **memctx;
{
int		 dummy;
int		 status;
struct	MEMCTX	*local_memctx	= (struct MEMCTX *) *memctx;

/*
** Swap use of ISL memory mgt back for original if the memory context
** block is non-NULL.
*/
if ( local_memctx != NULL )
    {
    /*
    ** Restore saved memory mgt context.
    */
    status = CDA_EXCHANGE_ALLOCATION_(
	    &user_root_aggr,
	    local_memctx->MEMCTX_A_ALLOC_RTN,
	    local_memctx->MEMCTX_A_DEALLOC_RTN,
	    local_memctx->MEMCTX_L_PARAM,
	    &dummy,
	    &dummy,
	    &dummy
	    );
    LOWBIT_TEST_AND_SIGNAL_( status );

    /*
    ** Deallocate memory mgt context block
    */
    _ImgCfree( local_memctx );
    *memctx = 0;
    }

return;
} /* end of _ImgRestoreMemoryMgt */


/******************************************************************************
**  _ImgSetMemoryMgt
**
**  FUNCTIONAL DESCRIPTION:
**
**	Save the memory mgt routines used by the root aggregate to import
**	data, and replace them with ISL memory mgt routines so that ISL
**	can read in the image info aggregates and own the dynamic memory.
**
**	Memory mgt will be restored by a call to _ImgRestoreMemoryMgt.
**
**  FORMAL PARAMETERS:
**
**	root_aggr	    Root aggregate handle, by value.
**	memctx		    Memory mgt context block, by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	Memory mgt routines to be saved (associated with root_aggr).
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void _ImgSetMemoryMgt( user_root_aggr, memctx )
CDArootagghandle user_root_aggr;
void **memctx;
{
int		 status;
struct	MEMCTX	*local_memctx	= 0;

/*
** Swap existing memory mgt for ISL memory mgt if ISL memory mgt has not
** been disabled.
*/
IF_NOT_SET_( state_flags.STF_V_CDA_MEMMGT_ISL )
    {
    /*
    ** Allocate a memory mgt context block.
    */
    local_memctx = (struct MEMCTX *) _ImgCalloc( 1, sizeof( struct MEMCTX ) );

    /*
    ** Exchange the previous memory mgt routines for ISL private routines.
    */
    status = CDA_EXCHANGE_ALLOCATION_(
	    &user_root_aggr,
	    IMG_A_GET_VM,
	    IMG_A_FREE_VM,
	    IMG_L_MEMMGT_PARAM,
	    &(local_memctx->MEMCTX_A_ALLOC_RTN),
	    &(local_memctx->MEMCTX_A_DEALLOC_RTN),
	    &(local_memctx->MEMCTX_L_PARAM)
	    );
    LOWBIT_TEST_AND_SIGNAL_( status );
    }

*memctx = (char *)local_memctx;
return;
} /* end of _ImgSetMemoryMgt */


/******************************************************************************
**  IMG__CALLOC
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate a block of memory and clear (or zero-fill) it.  
**
**	This function is ISL's implementation of calloc functionality.
**
**  FORMAL PARAMETERS:
**
**	number		Number of items of a specified size (next parameter)
**			to be allocated.
**			Passed by value.
**
**      size		Size of memory in bytes to be allocated.
**			Passed by value.
**
**	FILL_MASK	A 8-bit pattern to fill each byte with before
**			returning.  The default pattern is 00000000, which
**			returns a cleared buffer back to the caller.
**			This parameter is optional.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      user_mem	Pointer to dynamic memory allocated.  Note that the 
**			pointer returned is the address of the first byte of 
**			user space.  The address of the block head is never 
**			returned to the caller.
**			Passed by reference.
**              
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
static CDAaddress IMG__CALLOC( number, size)
CDAsize    	number;
CDAsize		size;
{
unsigned char	 fill_mask = ZERO_FILL_CHAR;
int		 argcnt;
int		 length;
int		 working_size;
struct	FLB	*user_flb;
char		*user_mem;

/*
** Allocate a block of memory.
*/
working_size = number * size;
user_flb = Allocate_memory( working_size );
user_mem = (char *) &user_flb[1];

/*
** See if it has to be cleared 
*/
if ( !(user_flb->FLB_W_FLAGS.FLB_V_DZERO_CLRD))
    {
    /*
    ** Clear the user space.
    */
    length = user_flb->FLB_L_LENGTH - FLB_K_SIZE;
    
    memset(	user_mem,		/* mem address	*/
    		fill_mask,		/* fill char	*/
    		length			/* mem length	*/
    		);
    }

/*
** Mark this as a user-block, and set the demand-zero cleared flag to
** false since the memory will almost certainly not be zero upon return.
** Also, add this block to the in-use list for user blocks.
*/
user_flb->FLB_B_TYPE = IMG_K_TYPE_USB;
user_flb->FLB_W_FLAGS.FLB_V_DZERO_CLRD = FALSE;
Add_blk_to_ulist( user_flb );

/*                
** Return the address of the user space portion of the block
*/
return (char *) user_mem;
} /* end of IMG__CALLOC */


/******************************************************************************
**  IMG__CFREE
**
**  FUNCTIONAL DESCRIPTION:
**
**      Frees dynamic memory allocated by IMG__CALLOC.  
**
**	This function is ISL's implementation of cfree functionality.
**
**	Note that on the VAX, the procedure to deallocate memory allocated
**	by IMG__CALLOC() is identical to the procedure to deallocate memory
**	allocated by IMG__MALLOC().  Therefore, this procedure simply calls
**	IMG__FREE().
**
**	This procedure may have to be modified when ported to ULTRIX
**	or DOS.
**
**  FORMAL PARAMETERS:
**
**      memptr		Pointer to memory allocated by a call to _ImgCalloc.
**			Passed by value.  (Memory itself passed by ref.)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      unsigned long status
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
static CDAstatus IMG__CFREE( memptr )
CDAaddress    memptr;
{

IMG__FREE( memptr );

return 1; /* return success */
} /* end of IMG__CFREE */


/******************************************************************************
**  IMG__FREE
**
**  FUNCTIONAL DESCRIPTION:
**
**      Free dynamic memory allocated by a call to IMG__MALLOC().  This
**	function emulates the C function free().
**
**	This function is ISL's implementation of free functionality.
**
**  FORMAL PARAMETERS:
**
**      memptr		Pointer to dynamic memory to be deallocated.
**			Passed by value (memory passed by ref.)
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      unsigned long success
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
******************************************************************************/
static CDAstatus IMG__FREE( memptr )
CDAaddress    memptr;
{
struct	FLB	*flb;	/* NOTE: FLB immediately precedes user-accessible mem */

/*
** Calculate the free-list block head address
** by subtracting the block header size from the
** user space address.
*/
flb = (struct FLB *) (((char *)memptr) - FLB_K_SIZE);

/*
** Free the memory from usage.  
*/
Free_memory( flb );

return 1; /* return success */
} /* end of IMG__FREE */


/******************************************************************************
**  IMG__FREE_VM
**
**  FUNCTIONAL DESCRIPTION:
**
**	Free virtual memory that was allocated by a call to _ImgGetVM.
**	This routine is similar to the VMS RTL function LIB$FREE_VM, with 
**	a simliar call interface, with the exception of there being no 
**	zone-id.
**
**	This function actually only repackages the call to _ImgFree in
**	order to be used by routines that take the address of memory-mgt
**	routines with the same call parameters as the VMS RTL functions.
**
**	NOTE: this function CANNOT deallocate memory in the middle of a
**	block that was allocated by _ImgGetVM.  The address passed in
**	MUST be the starting address of the block returned by _ImgGetVM.
**
**  FORMAL PARAMETERS:
**
**	num_bytes	Number of contiguous bytes to be deallocated.
**			This parameter is actually a dummy argument, since
**			the base_adr argument is really all that is needed.
**			This parameter is passed by reference.
**
**	base_adr	Address of the dynamic memory to be deallocated.
**			Note that the memory passed in MUST be the address
**			of a memory-block that was returned by a call
**			to _ImgGetVM.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static CDAstatus IMG__FREE_VM( num_bytes, base_adr )
long	 *num_bytes;
char	**base_adr;
{

_ImgCfree( *base_adr );

*base_adr = 0;

return (CDAstatus)1;
} /* end of IMG__FREE_VM */


/******************************************************************************
**  IMG__GET_VM
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a block of dynamic (virtual) memory.
**	This routine is similar to the VMS RTL function LIB$GET_VM, with 
**	a simliar call interface, with the exception of there being no 
**	zone-id.
**
**	This function actually only repackages the call to _ImgCalloc in
**	order to be used by routines that take the address of memory-mgt
**	routines with the same call parameters as the VMS RTL functions.
**
**	Memory allocated by this routine should (for the sake of
**	consistency) by deallocated by a call to _ImgFree_VM.
**
**  FORMAL PARAMETERS:
**
**	num_bytes	Number of contiguous bytes to be allocated.
**			This parameter is passed by reference.
**
**	base_adr	Address of a longword to receive the address of
**			the first byte of allocated memory.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static CDAstatus IMG__GET_VM( num_bytes, base_adr, param )
long	 *num_bytes;
char	**base_adr;
long	  param;					/* not used */
{

*base_adr = (char *)_ImgCalloc( 1, *num_bytes );

return (CDAstatus)1;				    /* always return success */
} /* end of IMG__GET_VM */


/******************************************************************************
**  IMG__MALLOC
**
**  FUNCTIONAL DESCRIPTION:
**
**      Allocate dynamic memory.  This function is ISL's implementation
**	of malloc functionality.
**
**	This function does not clear the allocated memory before passing it 
**	back to the caller.
**
**  FORMAL PARAMETERS:
**
**      size		Size, in bytes, of memory block to be allocated.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none              
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      user_mem		Pointer to dynamic memory.
**				Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**               
******************************************************************************/
static CDAaddress IMG__MALLOC( size )
CDAsize    size;
{
struct	FLB	*user_flb;
char		*user_mem;

/*
** Allocate a block of memory
*/
user_flb = Allocate_memory( size );
user_mem = (char *) &user_flb[1];

/*
** Mark this as a user-block, and set the demand-zero cleared flag to
** False since the memory will almost certainly not be zero upon return.
** Also add the block to the in-use list of user blocks.
*/
user_flb->FLB_B_TYPE = IMG_K_TYPE_USB;
user_flb->FLB_W_FLAGS.FLB_V_DZERO_CLRD = FALSE;
Add_blk_to_ulist( user_flb );

/*
** Return the address of the user space portion of the block
*/
return (char *) user_mem;
} /* end of IMG__MALLOC */


/******************************************************************************
**  IMG__REALLOC
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reallocate a buffer, changing its size, and copy the contents
**	of the input buffer to the new output buffer.  Delete the input
**	buffer when the copy is complete.
**
**	This function is ISL's implementation of realloc functionality.
**
**	Note that if the reallocation size is less than the current
**	size, the original buffer (of its original size) will be returned.
**	If the buffer is reallocated, it will be cleared before the
**	old buffer is copied into it.
**
**	It is expected that this function will only be called to increase
**	the size of a buffer, not decrease it.
**
**  FORMAL PARAMETERS:
**
**	oldbuf		Buffer to reallocate.
**			Passed by reference.
**
**	size		Size in bytes of the new buffer.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	newbuf		Reallocated buffer.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static CDAaddress IMG__REALLOC( oldbuf, size )     
CDAaddress	oldbuf;
CDAsize		size;
{
char		     	*newbuf;
int			 old_userblk_size;
struct	FLB		*newblk;
struct	FLB		*oldblk;
char			*oldmem;

oldblk = (struct FLB *)(((char *)oldbuf) - FLB_K_SIZE);    /* find start of block header */
old_userblk_size = oldblk->FLB_L_LENGTH - FLB_K_SIZE;

if ( old_userblk_size >= size )
    /*
    ** The original buffer is actually greater than or equal to the input 
    ** buffer size.  Therefore no reallocation (to make the buffer larger)
    ** is really necessary.  Instead, an attempt will be made to shorten
    ** the old buffer, returning the old buffer address as newbuf.
    ** (Shortening will only happen if the extra memory is greater
    ** than 2 blocks, and can be rounded up to the nearest page
    ** boundary.)
    */
    newbuf = (char *) Realloc_down( oldbuf, size );
else
    /*
    ** The old buffer is less than the input size.  Therefore allocate
    ** a buffer of the input size and copy the old buffer into the
    ** new buffer.  Afterwards deallocate the old buffer.
    */
    {
    newbuf = IMG__CALLOC( 1, size );
    oldmem = (char *) &oldblk[1];
    memcpy( newbuf, oldmem, old_userblk_size );

    IMG__CFREE( oldbuf );
    }

return newbuf;
} /* end of IMG__REALLOC */

       
/******************************************************************************
**  Add_blk_to_flist()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Add a dynamic memory block to the free list.
**
**	This operation may or may not result in a block being added to the
**	free-list.  If the block to be added gets merged with another, and
**	the merged block is greater than IMG_K_MAX_FLB_SIZE (and meets all
**	the boundary conditions), the returnable memory will be returned
**	to P0 space (deallocated) and an entry will be added to the
**	returned-list.
**
**	Basic steps:
**
**		1)	Merge the new block into any adjacent blocks
**			in the free-list.  This step will unlink all
**			blocks that are merged.
**
**		2)	If the returned merge block exceeds the maximum
**			free-list block size, return it to P0 space
**			(if the block actually begins and ends on page
**			boundaries).
**
**		3)	If the block is not returned to P0 space, add
**			it to the free list, first by address and then
**			by size.
**
**  FORMAL PARAMETERS:
**
**      new_blk		Block to add to the free list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**           
**  SIDE EFFECTS:                     
**
**      none
**
**  CALLED BY:
**
**	Extract_user_blk()
**	Free_memory()
**	Merge_rlist_blocks()
**	Realloc_down()
**	Reclaim_p0_blk()
******************************************************************************/
static void Add_blk_to_flist( new_blk )
struct	FLB	*new_blk;
{
struct 	FLB	*merged_blk;
struct	FLB	*pageset_blk;

/*
** Mark block as a free-list block.
*/
new_blk->FLB_B_TYPE = IMG_K_TYPE_FLB;
new_blk->FLB_W_FLAGS.FLB_V_FLIST_MEMBER = TRUE;
new_blk->FLB_W_FLAGS.FLB_V_RLIST_MEMBER = FALSE;
new_blk->FLB_W_FLAGS.FLB_V_ULIST_MEMBER = FALSE;

/*
** If the block is a pageset already, and is big enough, don't
** even bother trying to merge it.
*/
if ( new_blk->FLB_L_LENGTH >= IMG_K_MAX_FLB_SIZE &&
       Pageset_test( new_blk ))
    merged_blk = new_blk;			/* pretend it's been merged */
else
    /*
    ** Merge the new block in with any adjacent blocks already
    ** in the free-list.
    */
    merged_blk = Merge_flist_block( new_blk );

/*
** Determine if the merged block should be returned to P0 space
** or added to the free-list.  Unless it's ultrix, in which case, 
** it must go on the free-list
*/
#if defined(__VMS) || defined(VMS)
if ( merged_blk->FLB_L_LENGTH >= IMG_K_MAX_FLB_SIZE &&
     Pageset_test( merged_blk ))
    Free_pageset( merged_blk );
else
#endif
    {
    /*
    ** Verify the block as an flb.
    */
    Verify_flb( merged_blk );

    /*
    ** Add block to free-list by address.  
    */
    Link_flist_blk_by_adr( merged_blk );

    /*
    ** Add block to free-list by size.  
    */
    Link_flist_blk_by_size( merged_blk );
    
    }

/*
** Verify consistancy between lists.
*/
Verify_flist();                             

return;
} /* end of Add_blk_to_flist */
    

/******************************************************************************
**  Add_blk_to_rlist()
**                                         
**  FUNCTIONAL DESCRIPTION:
**
**	Add a returned-list block to the returned-list.
**      
**  FORMAL PARAMETERS:
**
**      rlb		Block to add to the returned list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**	ImgX_NOTUSRMEM	    Ultrix runtime problem -- this should never
**			    be signalled, since this routine should never
**			    be called by Ultrix.
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Extract_rlist_blk()
**	Free_pageset()
******************************************************************************/
static void Add_blk_to_rlist( rlb )
struct	RLB	*rlb;
{
struct	RLB	*merged_block;

#if !defined(VMS) && !defined(__VMS)
/*
**  We should not be here under Ultrix
*/
ChfStop( 1, ImgX_NOTUSRMEM);
#endif

/*
** Merge rlist block.
*/
merged_block = Merge_rlist_block( rlb );

/*
** Add to rlist by adr
*/
Link_rlist_blk_by_adr( merged_block );

/*
** Add to rlist by size
*/
Link_rlist_blk_by_size( merged_block );

/*
** Verify rlist.
*/
Verify_rlist();

return;
} /* end of Add_blk_to_rlist */


/******************************************************************************
**  Add_blk_to_ulist()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Add a user-block to the user-block in-use list.
**	This list simply keeps track of the blocks that have been
**	given out to callers of _ImgCalloc and _ImgMalloc.
**
**  FORMAL PARAMETERS:
**
**	user-blk	User block to add to the user-list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	
******************************************************************************/
static void Add_blk_to_ulist( user_blk )
struct	FLB *user_blk;
{
Verify_ulist();

Link_ulist_blk_by_adr( user_blk );
Link_ulist_blk_by_size( user_blk );

Verify_ulist();

return;
} /* end of Add_blk_to_ulist */


/******************************************************************************
**  Allocate_memory
**
**  FUNCTIONAL DESCRIPTION:
**
**	General and outermost function for allocating a block of dynamic
**	memory.  This function may return memory either from the free-list,
**	the returned-list, or from previously unused P0 space at the end
**	of the P0 region.
**
**	This routine actually returns the starting address of free-list
**	blocks and returned-list blocks.  Blocks returned by this function
**	may be used internally by IMM.
**
**  FORMAL PARAMETERS:
**
**      size		Size in bytes needed by the IMM user.
**			Passed by value.  This may be a full longword integer.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      Address of a block-header of a free-list or returned-list block.
**	The block returned is treated as a free-list block, but it could
**	be used as a returned-list block with no side effects.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**  
**	_ImgCalloc()
**	_ImgMalloc()
**	Free_pageset()
******************************************************************************/
static struct FLB *Allocate_memory( size )
int	size;
{
int	     ast_status;
int	     status;
int	     working_size;
struct	FLB *user_memptr;

#if defined(__VMS) || defined(VMS)
/*
** Disable ASTs
*/
ast_status = sys$setast( IMG_K_DISABLE_AST );
LOWBIT_TEST_AND_SIGNAL_( ast_status );
#endif
/*
** Verify that the IMM environment has been initialized
*/
if ( !(state_flags.STF_V_INITIALIZED) )
    Initialize_imm();

/*
** Round up the requested size (in bytes) to include a
** block header and quadword alignment padding.
*/
working_size = Roundup_blocksize( size );

/*
** Get memory.  Go to free-list if blocks are available, else
** get VM from P0 space.
*/
if ( working_size <= Largest_flb_size() )
    user_memptr = Getblk_from_flist( working_size );
else
    user_memptr = Getblk_from_p0reg( working_size );

/*
** Calculate the amount of padding at the end of the block (i.e., unused
** space at the end of the user block).
*/
user_memptr->FLB_B_PADCNT = working_size - size - FLB_K_SIZE;

/*
** Does the block being returned contain an address that is being watched for?
**
** This is here to help with debugging with the VAX/VMS debugger.
*/
if ( (long)user_memptr <= watch_block && 
     watch_block <= (long)user_memptr + *((long *)user_memptr) )
     watch_block_found = 1;

/*
** Enable ASTs
*/
#if defined(__VMS) || defined(VMS)
if ( ast_status == SS_WASSET )
    ast_status = sys$setast( IMG_K_ENABLE_AST );
LOWBIT_TEST_AND_SIGNAL_( ast_status );
#endif

return user_memptr;
} /* end of Allocate_memory */


/******************************************************************************
**  Allocate_rlb()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Allocate a returned list block to be used to keep track of
**	a virtual page set that is being returned to the system.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	rlb	    Address of rlb.	Passed by value.
**
**  SIGNAL CODES:
**
**	none.
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static struct RLB *Allocate_rlb()
{
struct RLB  *rlb;

rlb = (struct RLB *)Allocate_memory( RLB_K_SIZE - FLB_K_SIZE);

((struct FLB*)rlb)->FLB_W_FLAGS.FLB_V_RLIST_MEMBER = TRUE;
((struct FLB*)rlb)->FLB_W_FLAGS.FLB_V_FLIST_MEMBER = FALSE;
((struct FLB*)rlb)->FLB_W_FLAGS.FLB_V_ULIST_MEMBER = FALSE;
((struct FLB*)rlb)->FLB_W_FLAGS.FLB_V_DZERO_CLRD = FALSE;

return rlb;
} /* end of Allocate_rlb */

/******************************************************************************
**  Extract_rlist_blk
**
**  FUNCTIONAL DESCRIPTION:
**
**	Extract a region of (DELTVA'ed) virtual memory from an RLB in the
**	rlist, returning the extracted region to the caller, and returning
**	any left-over back to the rlist.
**
**	If there is no remainder of the region, the input RLB is deleted.
**
**  FORMAL PARAMETERS:
**
**	rlb, by reference	Rlist block to extract region from.
**
**	size, by value		Size in bytes of region to be extracted.
**				NOTE that this size is rounded up to the
**				nearest full pageset size.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	The remainder region will be returned to the rlist; a new
**	rlb will be created to describe the remainder region.
**
**	The LAST_BYTE and PAGESET_SIZE fields of the returned rlb will
**	be modified.
**
**  FUNCTION VALUE:
**
**	Returns the address of the rlb that was passed in.  The returned
**	rlb will point to the extracted region.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Find_rlist_best_fit()
**
******************************************************************************/
static struct RLB *Extract_rlist_blk( rlb, size )
struct	RLB *rlb;
int	     size;
{
int	     working_size;
struct	RLB *remainder_rlb;

/*
** Round up the requested size to the nearest full page size.
*/
working_size = ROUNDUP_PAGESIZE_( size );

/*
** If the difference between the working size and the actual region
** size is less than IMG_K_MAX_FLB_SIZE, return the entire region.
** Otherwise extract a region that is working_size bytes and return 
** the remainder to the returned list (after adjusting the input rlb
** to reflect the extraction of the working region).
*/
if ( rlb->RLB_L_PAGESET_SIZE - working_size >= IMG_K_MAX_FLB_SIZE )
    /*
    ** Extract a region from the beginning of the region pointed to 
    ** by the rlb.
    */
    {
    /*
    ** Allocate and initialize the working rlist_blk.
    */
    remainder_rlb = Allocate_rlb();
    remainder_rlb->RLB_A_FIRST_BYTE = rlb->RLB_A_FIRST_BYTE + working_size;
    remainder_rlb->RLB_A_LAST_BYTE = rlb->RLB_A_LAST_BYTE;
    remainder_rlb->RLB_L_PAGESET_SIZE = rlb->RLB_L_PAGESET_SIZE - working_size;

    /*
    ** Modify the input rlb for return.
    */
    rlb->RLB_A_LAST_BYTE = rlb->RLB_A_FIRST_BYTE + working_size;
    rlb->RLB_L_PAGESET_SIZE = working_size;

    /*
    ** Return the (new) remainder rlb to the rlist.
    */
    Add_blk_to_rlist( remainder_rlb );
    }

return rlb;
} /* end of Extract_rlist_blk */


/******************************************************************************
**  Extract_user_blk()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Extract a user-block from a (possibly larger) block of dynamic memory.
**	Store any remainder in the free-list, provided that the remainder
**	is large enough to form a free-list block.
**
**  FORMAL PARAMETERS:
**
**      src_blk		Source block from which to extract the user-block.
**			Passed by reference.
**	size		Size of the user-block to extract.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      user-blk	User-block extracted from input block.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Getblk_from_expreg()
**	Getblk_from_flist()
**	Getblk_from_rlist()
******************************************************************************/
static struct FLB *Extract_user_blk( src_blk, size )
struct	FLB *src_blk;
int	     size;                             
{
int	     remainder_blk_size;
int	     src_blk_size;
int	     user_blk_size;
struct	FLB *remainder_blk;
struct	FLB *user_blk;

src_blk_size = BLOCK_SIZE_( src_blk );
remainder_blk_size = src_blk_size - size;

/*
** Determine whether to save the remainder in the free-list or to
** simply leave it as part of the user-block.  All remainders greater
** than or equal to the size of an FLB block header will be saved in
** the free-list.  
**
** This will save blocks with no user-space.  Blocks of this nature
** are saved in case they can be combined, at some future time, with
** other blocks in the free list (as part of a concatenate operation).
*/
if ( remainder_blk_size < FLB_K_SIZE )
    /*
    ** The remainder is too small, so absorb it into the user-block.
    */
    {
    user_blk_size = src_blk_size;
    user_blk = src_blk;
    }
else
    /*
    ** The remainder is big enough to store in the free-list.
    ** Therefore, break off the remainder and store it.  Note that
    ** both blocks (user and remainder) must be resized.
    **
    ** NOTE that src_blk is type cast as an int so that the user_blk_size
    ** will add properly
    */
    {
    user_blk_size = size;
    user_blk = src_blk;
    remainder_blk = (struct FLB *) ((int)src_blk + user_blk_size);
    
    /*
    ** Initialize the remainder block header.
    ** (Copy appropriate characteristics from original user block.)
    */
    Initialize_flb( remainder_blk, remainder_blk_size );
    if ( user_blk->FLB_W_FLAGS.FLB_V_DZERO_CLRD)
    	remainder_blk->FLB_W_FLAGS.FLB_V_DZERO_CLRD = TRUE;
    if ( user_blk->FLB_W_FLAGS.FLB_V_PAGE_END)
    	remainder_blk->FLB_W_FLAGS.FLB_V_PAGE_END = TRUE;
    
    	/*
    	** resize and reset the (shortened) user block
    	*/
    user_blk->FLB_L_LENGTH = user_blk_size;
    user_blk->FLB_W_FLAGS.FLB_V_PAGE_END = FALSE;	/* not at the end any more */

    Add_blk_to_flist( remainder_blk );
    }

return user_blk;
} /* end of Extract_user_blk */


/******************************************************************************
**  Find_flist_best_fit()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Find the "best fit" block in the free list that will satisfy a
**	request of a block for a particular size.  The "best fit" block
**	is the smallest block in the free-list that is greater than or
**	equal to the requested size.
**
**	This routine removes the "best fit" block from the free-list.
**
**	Note that this routine assumes that a "best fit" block will be
**	found in the list.  The test to see if there is a best "fit block"
**	was done by the routine that called this one.
**
**  FORMAL PARAMETERS:
**
**      size		Size of user block, in bytes.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      best_fit_blk		Best fit block.
**				Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Getblk_from_flist()
******************************************************************************/
static struct FLB *Find_flist_best_fit( size )
int	size;
{
struct	FLB	*best_fit_blk;
struct	FLB	*cur_blk;

/*
** Get the first (and smallest) block in the free-list
*/
cur_blk = (struct FLB *) (FIRST_FLB_BY_SIZE_());

/*
** Loop: find the best fit by comparing the input size with the
** size of the current block.
*/
while ( BLOCK_SIZE_( cur_blk ) < size )
    {
    cur_blk = (struct FLB *) (NEXT_FLIST_BLK_BY_SIZE_( cur_blk ));
    }
              
/*
** Unlink current block from free-list, returning it as the
** best-fit block.
*/
best_fit_blk = Unlink_flist_blk( cur_blk );

return best_fit_blk;
} /* end of Find_flist_best_fit */


/******************************************************************************
**  Find_rlist_best_fit()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Find the "best fit" block in the returned list that will satisfy a
**	request of a block for a particular size.  The "best fit" block
**	is the smallest block in the free-list that is greater than or
**	equal to the requested size.
**
**	This routine reallocates memory that was returned to P0 space,
**	and removes the corresponding RLB from the returned-list.
**
**	The best-fit block is actually a pageset block.  
**
**  FORMAL PARAMETERS:
**
**      size		Size of user block, in bytes.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      best_fit_blk		Best fit block.
**				Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Getblk_from_rlist()
******************************************************************************/
static struct RLB *Find_rlist_best_fit( size )
int	size;
{
struct	FLB *best_fit_blk;
struct	RLB *cur_blk;
struct	RLB *working_rlb;

/*
** Get the first (and smallest) block in the returned-list.
*/
cur_blk = (struct RLB *) (FIRST_RLB_BY_SIZE_());

/*
** Loop: find the best fit by comparing the input size with the
** size of the current block.
*/
while ( cur_blk->RLB_L_PAGESET_SIZE < size )
    {
    cur_blk = (struct RLB *) (NEXT_RLIST_BLK_BY_SIZE_( cur_blk ));
    }

/*
** Unlink current block from returned-list.  The best-fit block will
** be taken from the pageset pointed to by this block.
*/
cur_blk = Unlink_rlist_blk( cur_blk );

/*
** Extract an Rlist block from the current block that satisfies the
** size requirement of the requested block.  This will return the
** cur_blk rlb to the rlist if there is any memory out of the best-fit 
** region that is unused.
*/
working_rlb = Extract_rlist_blk( cur_blk, size );

/*
** Reclaim the deallocated memory (pointed to by the current RLB)
** from P0 space.  This reclaimed memory will be the best-fit block.
** The reclaimed best-fit block is returned as an FLB.
**
** The working_rlb is deallocated by the Reclaim_p0_flb function.
*/
best_fit_blk = Reclaim_p0_flb( working_rlb );
                        
return (struct RLB *) best_fit_blk;
} /* end of Find_rlist_best_fit */
         

/******************************************************************************
**  Free_memory()
**
**  FUNCTIONAL DESCRIPTION:
**
**	High level function for freeing a block from use.  These blocks
**	may have been used either by users or by IMM as returned-list
**	blocks.
**
**  FORMAL PARAMETERS:
**
**      flb		Address of the memory block header (treated like
**			a free-list block).  The block is passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**  
**      none
**
**  CALLED BY:
**
**  	_ImgFree()
******************************************************************************/
static void Free_memory( flb )
struct	FLB	*flb;
{
int ast_status;
int status;

/*
** Disable ASTs
*/
#if defined(__VMS) || defined(VMS)
ast_status = sys$setast( IMG_K_DISABLE_AST );
LOWBIT_TEST_AND_SIGNAL_( ast_status );
#endif

/*
** Verify that the IMM environment has been initialized
*/
if ( !(state_flags.STF_V_INITIALIZED) )
    Initialize_imm();

/*
** Verify that the block being freed is a valid user-block.
*/
Verify_block_type( flb, IMG_K_TYPE_USB );

/*
** Unlink the block from the user-block in-use list	
*/
Unlink_ulist_blk( flb );
    
/*
** Add block to free-list.  If the block is actually a pageset large
** enough to be deallocated (and put on the returned-list), the
** Add_blk_to_flist function will do that automatically.
*/
Add_blk_to_flist( flb );
   
/*
** All done.  Enable ASTs
*/
#if defined(__VMS) || defined(VMS)
if ( ast_status == SS_WASSET )
    ast_status = sys$setast( IMG_K_ENABLE_AST );
LOWBIT_TEST_AND_SIGNAL_( ast_status );
#endif 

return;
} /* end of Free_memory */


/******************************************************************************
**  Free_pageset()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return a pageset to P0 space.  Deallocate the memory (via $DELTVA)
**	and add an entry for that memory to the returned-list.
**
**	NOTE: This routine is only defined for VMS.  It is empty under
**	ULTRIX.
**
**  FORMAL PARAMETERS:
**
**      pageset_blk	Pageset block to be deallocated.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**    
**  SIGNAL CODES:
**      
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
******************************************************************************/
static void Free_pageset( pageset_blk )
struct	FLB	*pageset_blk;
{
#if defined(__VMS) || defined(VMS)
int		 status;
struct	RLB	*rlb;
struct	RANGE {
    int	start;
    int	end;
    }		 inadr;			/* memory address range */
struct	RANGE	 retadr;

/*
** Allocate a returned-list block (RLB).  Request just the amount of
** non-RLB-header space needed.  (Note that FLB_K_SIZE is exactly the
** size of the block head used by both FLB and RLB blocks, whereas the
** RLB_K_SIZE value includes structure space following the block head.)
*/
rlb = Allocate_rlb();

/*
** Fill in the returned range and page count fields of the RLB
*/
rlb->RLB_A_FIRST_BYTE = (long)pageset_blk;
rlb->RLB_A_LAST_BYTE = rlb->RLB_A_FIRST_BYTE + pageset_blk->FLB_L_LENGTH - 1;
rlb->RLB_L_PAGESET_SIZE = pageset_blk->FLB_L_LENGTH;

/*
** Mark this as a returned-list block
*/
rlb->RLB_B_TYPE = IMG_K_TYPE_RLB;

/*
** Figure out the real end-range value to deallocate.  Deallocate the
** entire pageset unless the last page of the pageset is also the
** last page that was allocated by IMM services.  If this is so, then
** do not actually deallocate the last page.  (Note that the RLB range
** will look as if the last page was deallocated.)
**                                               
** In actuallity, although, in the returned-list, it may look as though
** the last page was deallocated, it doesn't matter that it wasn't, since
** the entire range can be reallocated by $CRETVA, which will deallocate
** and then reallocate the last page all at once.
**
** The purpose of not deallocating the last page in the P0 region is to
** avoid having the P0 region contract.  This ensures that all calls to
** $EXPREG will return successively higher addresses each time, and will not
** 'back-track' over previously allocated memory address ranges.  This will
** cut out having to determine if and when the program region contracts
** upon deallocation of a pageset.
*/
inadr.start = (int)pageset_blk;
inadr.end = (rlb->RLB_A_LAST_BYTE == (int)highest_byte_allocd)?
		rlb->RLB_A_LAST_BYTE - 512: rlb->RLB_A_LAST_BYTE;
/*
** Deallocate the memory.
*/
status = sys$deltva( &inadr, &retadr, 0 );
if ( !VMS_STATUS_SUCCESS_( status ) )
    ChfStop( 1,  status );

/*                
** Add the rlb to the returned-list.
*/
Add_blk_to_rlist( rlb );

#endif
return;
} /* end of Free_pageset */

                                                
/******************************************************************************
**  Getblk_from_expreg()
**
**  FUNCTIONAL DESCRIPTION:
**                                                             
**	Get a block of memory by calling $EXPREG to extend to P0 program
**	region.  The smallest number of pages needed to satisfy the
**	block request will be allocated.
**
**  FORMAL PARAMETERS:
**
**	size		Block size in bytes.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	memblk		Memory block allocated (and treated as a freelist block)
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	ImgX_NOTUSRMEM	    Ultrix runtime problem -- this should never
**			    be signalled, since this routine should never
**			    be called by Ultrix.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Getblk_from_p0reg()
******************************************************************************/
static struct FLB *Getblk_from_expreg( size )
int	size;
{
int		 bytecnt;
int		 pagecnt;
int		 status;
struct	FLB	*memblk;
struct	FLB	*pageset;
struct	{
    long	 begin_adr;
    long	 end_adr;
    }		 retadr;

/*
** Calculate page count
*/
pagecnt = (size + 511)/512;

/*
** Allocate pages.
*/
#if defined(__VMS) || defined(VMS)
status = sys$expreg( pagecnt, &retadr, 0, 0 );
if ( !VMS_STATUS_SUCCESS_( status ) )
    ChfStop( 1,  status );
#else
bytecnt = pagecnt * 512;
retadr.begin_adr = (long)sbrk(bytecnt);
if (retadr.begin_adr == -1)
    ChfStop( 1, ImgX_INSVIRMEM);
retadr.end_adr = retadr.begin_adr + bytecnt -1;
#endif

highest_page_allocd = (char *) (retadr.end_adr - 511);
highest_byte_allocd = (char *) (retadr.end_adr);
if ( lowest_page_allocd == 0 )
    lowest_page_allocd = (char *) retadr.begin_adr;

/*      
** Turn the allocated pages into a free-list block
*/
pageset = Initialize_p0_flb( (struct FLB *)retadr.begin_adr, pagecnt * 512 );

/*
** Extract a block of the requested size from the pageset.
** (This will store any saveable excess bytes from the end of
** the pageset in the freelist.)
*/
memblk = (struct FLB *)Extract_user_blk( pageset, size );

return memblk;
} /* end of Getblk_from_expreg */


/******************************************************************************
**  Getblk_from_flist()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get dynamic memory block from the free-list.
**
**  FORMAL PARAMETERS:
**
**	size		Size in bytes of block to return.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	user_blk	Pointer to first byte of the returned block
**			of memory.  Memory block passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Allocate_memory()
******************************************************************************/
static struct FLB *Getblk_from_flist( size )
int	size;
{
struct	FLB	*best_fit_blk;
struct	FLB	*user_blk;

/*
** Find and extract the "best fit" block from the free-list.  The "best-fit"
** block is the smallest undivided block currently in the free-list that
** is greater than or equal to the requested size.  This procedure
** removes the "best fit" block from the free-list.
*/
best_fit_blk = Find_flist_best_fit( size );

/*
** Extract the user-block from the "best fit" block.  If there is a remainder,
** put it back into the free-list automatically.
*/
user_blk = (struct FLB *)Extract_user_blk( best_fit_blk, size );
                         
return user_blk;
} /* end of Getblk_from_flist */


/******************************************************************************
**  Getblk_from_p0reg()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Get dynamic memory block from the P0 program region.
**
**  FORMAL PARAMETERS:
**
**	size		Size in bytes of block to return.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	memptr		Pointer to first byte of the returned block
**			of memory.  Memory block passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Allocate_memory()
******************************************************************************/
static struct FLB *Getblk_from_p0reg( size )
int	size;
{
struct	FLB	*memptr;

/*
** Try to get memory from the returned-list if it contains a big-enough
** block.  If it doesn't allocate new memory at the end of the P0
** program region.
**
** NOTE: if under ULTRIX, do not use return lists.
*/
#if defined(__VMS) || defined(VMS)
if ( size <= Largest_rlb_size() )		/* VMS only conditional	*/
    memptr = Getblk_from_rlist( size );
else         
#endif
    memptr = Getblk_from_expreg( size );	/* VMS & ULTRIX		*/

return memptr;
} /* end of Getblk_from_p0reg */


/******************************************************************************
**  Getblk_from_rlist()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Get a block of dynamic memory from the returned-list.
**
**	NOTE: This routine is only defined for VMS.
**
**  FORMAL PARAMETERS:
**
**	size		Size in bytes of block to allocate.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	memblk		Memory block that was allocated.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	ImgX_NOTUSRMEM	    Ultrix runtime problem -- this should never
**			    be signalled, since this routine should never
**			    be called by Ultrix.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Getblk_from_p0reg()
******************************************************************************/
static struct FLB *Getblk_from_rlist( size )
int	size;
{
#if defined(__VMS) || defined(VMS)
struct	RLB	*best_fit_blk;
struct	FLB	*memblk;
/*
** Get the "best fit" block from the returned list.  This is not necessarily
** an entire RLB-pageset block from the rlist, but is actually the only the
** smallest pageset needed that can be made from the smallest RLB in the rlist.
*/
best_fit_blk = Find_rlist_best_fit( size );

/*
** Extract the pageset to fulfill the requested size and return
** any remainder to the returned-list.
*/
memblk = (struct FLB *)Extract_user_blk( best_fit_blk, size );

return memblk;
#endif
} /* end of Getblk_from_rlist */
                       

/******************************************************************************
**  Initialize_flb()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Initialize the header of a free-list block.
**
**  FORMAL PARAMETERS:
**
**	flb		Free-list block to initialize.
**			Passed by reference.
**	size		Size of block in bytes.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	flb		Free-list block that was initialized.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none                            
**
**  CALLED BY:
**
**	Extract_user_blk()
**	Initialize_p0_flb()
**	Realloc_down()
******************************************************************************/
static struct FLB *Initialize_flb( flb, size )
struct	FLB	*flb;
int		 size;
{

flb->FLB_L_LENGTH = size;
/*
** Note that for this next line, since the flags field is actually a
** structure, the address of the field first has to be cast as a pointer
** to a word (short); after which the value of 0 is assigned to the
** contents of the word being pointed to.
*/
*((short*)(&(flb->FLB_W_FLAGS))) = 0;		/* clear the flags */

flb->FLB_B_TYPE = IMG_K_TYPE_FLB;
flb->FLB_W_FLAGS.FLB_V_NODELETE = FALSE;
flb->FLB_A_FLINK_BY_SIZE = flb;
flb->FLB_A_BLINK_BY_SIZE = flb;
flb->FLB_A_FLINK_BY_ADR = flb;
flb->FLB_A_BLINK_BY_ADR = flb;

return flb;
} /* end of Initialize_flb */

     
/******************************************************************************
**  Initialize_imm()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Initialize ISL Memory Management environment.
**
**	Make sure that all list-heads have been initialized.
**
**  FORMAL PARAMETERS:
**
**	none.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Allocate_memory()
**	Free_memory()
******************************************************************************/
static void Initialize_imm()
{

IF_NOT_SET_( state_flags.STF_V_INITIALIZED )
    {
    /*
    ** Initialize list heads
    */
    flist_by_adr.LhdA_Flink = (struct BHD *)&flist_by_adr;
    flist_by_adr.LhdA_Blink = (struct BHD *)&flist_by_adr;

    flist_by_size.LhdA_Flink = (struct BHD *)&flist_by_size;
    flist_by_size.LhdA_Blink = (struct BHD *)&flist_by_size;
                                                              
    /*
    ** Initialize highest returned-list block
    */
    highest_rlb_by_adr.RLB_A_FIRST_BYTE = HIGHEST_P0_ADDRESS;

    /*
    ** Initialize the memory mgt routine pointers used
    ** by core services to allocate dynamic memory.
    */
    _IpsInitMemoryTable(
	    (unsigned char *)ImgAllocDataPlane,			/* allocate image	*/
	    (unsigned char *)ImgFreeDataPlane,			/* deallocate image	*/
	    (unsigned char *)ImgReallocateDataPlane,		/* reallocate image	*/
	    (unsigned char *)_ImgAlloc,				/* allocate buffer	*/
	    (unsigned char *)_ImgDealloc,			/* deallocate buffer	*/
	    (unsigned char *)_ImgRealloc				/* reallocate buffer	*/
	    );

    /*
    ** Set module-wide state flag, initializing all done.
    */
    state_flags.STF_V_INITIALIZED = TRUE;
    }

IF_NOT_SET_( state_flags.STF_V_CDA_MEMMGT_INIT )
    {
    /*
    ** Establish ISL memory mgt as the default for use with DDIF/CDA
    ** functions.  Set up the pointers to get and free vm calls.
    ** This will ALWAYS be initialized before a CDA function has been 
    ** called.
    */
    ImgResetMemoryMgt();
    }

IF_NOT_SET_( ImgL_MemTraceInitialized )
    {
    char    *trace_fname;
    long     str_len;
    FILE    *fp;

    trace_fname = (char *)getenv( "IMG_TRACE_MEM_FILE" );
    if ( trace_fname )
	{
	fp = fopen( trace_fname, "w" );
	if ( fp )
	    ImgL_MemTraceOutputFile = fp;
	}

    ImgL_MemTraceInitialized = 1;
    }

return;
} /* end of Initialize_imm */


/******************************************************************************
**  Initialize_p0_flb()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Initialize a free-list block that has been newly allocated from
**	process P0 space.  This will always initialize pageset blocks.
**
**	This function:
**
**		- fills in the block length (in bytes)
**		- initializes all the flags
**
**  FORMAL PARAMETERS:
**
**	pageset		Pageset to be initialized as a free-list block.
**  			Passed by reference.
**	size		Size of pageset in bytes.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	pageset		Pageset block after having been initialized.
**			Passed by reference.
**                                         
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Getblk_from_expreg()
**	Reclaim_p0_blk()
******************************************************************************/
static struct FLB *Initialize_p0_flb( pageset, size )
struct	FLB	*pageset;
int		 size;
{

Initialize_flb( pageset, size );
pageset->FLB_W_FLAGS.FLB_V_DZERO_CLRD	= TRUE;
pageset->FLB_W_FLAGS.FLB_V_PAGE_BEGIN	= TRUE;
pageset->FLB_W_FLAGS.FLB_V_PAGE_END	= TRUE;
pageset->FLB_W_FLAGS.FLB_V_RLIST_MEMBER	= FALSE;
pageset->FLB_W_FLAGS.FLB_V_FLIST_MEMBER	= FALSE;
pageset->FLB_W_FLAGS.FLB_V_ULIST_MEMBER	= FALSE;
pageset->FLB_W_FLAGS.FLB_V_FIRST_LSTBLK	= FALSE;
pageset->FLB_W_FLAGS.FLB_V_LAST_LSTBLK	= FALSE;
pageset->FLB_W_FLAGS.FLB_V_NODELETE	= FALSE;

return pageset;
} /* end of Initialize_p0_flb */


/******************************************************************************
**  Largest_flb_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return the size of the largest free-list block in the free-list.
**
**  FORMAL PARAMETERS:
**
**	none.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	flb_size	Free-list block size.
**			Passed by value.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Allocate_memory()
******************************************************************************/
static int Largest_flb_size()
{
int	flb_size = 0;

if ( flist_by_size.LhdW_ListCnt != 0 )
    flb_size = ((struct FLB *)flist_by_size.LhdA_Blink)->FLB_L_LENGTH;

return flb_size;
} /* end of Largest_flb_size */


/******************************************************************************
**  Largest_rlb_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Return the size of the largest pageset in the returned-list.
**
**  FORMAL PARAMETERS:
**
**	none.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	rlb_size	The maximum pageset size in the returned-list.
**			Passed by value.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Getblk_from_p0reg()
******************************************************************************/
static int Largest_rlb_size()
{
int	rlb_size = 0;

if ( rlist_by_size.LhdW_ListCnt != 0 )
    rlb_size = ((struct RLB *)rlist_by_size.LhdA_Blink)->RLB_L_PAGESET_SIZE;

return rlb_size;
} /* end of Largest_rlb_size */


/******************************************************************************
**  Link_flist_blk_by_adr()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Link a block into the free-list, ordering it by address.
**
**  FORMAL PARAMETERS:
**
**	new_blk		New block to be added.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
******************************************************************************/
static void Link_flist_blk_by_adr( new_blk )
struct	FLB	*new_blk;
{
int		 index;
int		 test1;
int		 test2;
struct	FLB	*next_blk	= 0;
struct	FLB	*prev_blk	= 0;
        
/*
** Figure out where the new block should go.
*/
if ( flist_by_adr.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, add it to the list
    ** with no further ado.
    */
    {
    flist_by_adr.LhdA_Flink = (struct BHD *) new_blk;
    flist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
    new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &flist_by_adr;
    new_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) &flist_by_adr;
    } /* end if-then */
else if ( (int) new_blk < (int) flist_by_adr.LhdA_Flink )
    {
    /*
    ** The block should be entered before the current first
    ** block in the list-by-address.
    */
    next_blk = (struct FLB *) flist_by_adr.LhdA_Flink;
    flist_by_adr.LhdA_Flink = (struct BHD *) new_blk;
    new_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) &flist_by_adr;
    new_blk->FLB_A_FLINK_BY_ADR = next_blk;
    next_blk->FLB_A_BLINK_BY_ADR = new_blk;
    }
else if ( flist_by_adr.LhdW_ListCnt == 1 )
    {
    /*
    ** There's only one element in the list, and since the block
    ** to be inserted does not go before the current first block
    ** in the list, it must go after it.  This makes the new block
    ** the last element in the list.
    */
    prev_blk = (struct FLB *) flist_by_adr.LhdA_Flink;
    prev_blk->FLB_A_FLINK_BY_ADR = new_blk;
    new_blk->FLB_A_BLINK_BY_ADR = prev_blk;
    new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &flist_by_adr;
    flist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
    }
else
    {
    /*
    ** The block should be entered somewhere past the first
    ** block in the list.  Figure out where the new block will go.
    */
    prev_blk = (struct FLB *) flist_by_adr.LhdA_Flink;	/* get the first block */
    next_blk = prev_blk->FLB_A_FLINK_BY_ADR;
    for ( index = 0; index < flist_by_adr.LhdW_ListCnt; index++ )
    	{
    	if ( prev_blk < new_blk && new_blk < next_blk )
            /*
            ** We've found the right place:  the new block
            ** goes in between the current block and the next
            ** block;  therefore, insert it by address.
            */
            {
            prev_blk->FLB_A_FLINK_BY_ADR = new_blk;
            new_blk->FLB_A_BLINK_BY_ADR = prev_blk;
            new_blk->FLB_A_FLINK_BY_ADR = next_blk;
            next_blk->FLB_A_BLINK_BY_ADR = new_blk;
            break;
            } /* end if-then */
    	else
            /*
            ** Advance the previous and next block pointers.
            ** Detect end of list condition (when the index
            ** is two less than the list count).
            */
            {
            if ( index < (flist_by_adr.LhdW_ListCnt - 2) )
            	/*
            	** Advance the pointers and loop again.
            	*/
            	{
            	prev_blk = next_blk;
            	next_blk = next_blk->FLB_A_FLINK_BY_ADR;
            	} /* end if-then */
            else
            	/*
            	** End of list condition; the new block goes
            	** after the last block.  Insert it by address.
            	** Note that the "next_blk" pointer points to
            	** the last block in the list at this point.
            	*/
            	{
            	next_blk->FLB_A_FLINK_BY_ADR = new_blk;
            	new_blk->FLB_A_BLINK_BY_ADR = next_blk;
            	new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &flist_by_adr;
            	flist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
            	break;
            	} /* end if-else */
            } /* end if-else */                   
    	} /* end for-loop */
    } /* end outer-most if-else */

++flist_by_adr.LhdW_ListCnt;

return;
} /* end of Link_flist_blk_by_adr */


/******************************************************************************
**  Link_flist_by_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Link a block into the free-list, ordering it by size.
**
**  FORMAL PARAMETERS:
**
**	new_blk		Block to add to free-list.
**			Passed by reference.
**
**			This argument is the value returned from a call
**			to Add_flist_blk_by_size().
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	none (void)
**
**  SIGNAL CODES:          
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
******************************************************************************/
static void Link_flist_blk_by_size( new_blk )
struct	FLB	*new_blk;
{
int		 index;
int		 test1;
int		 test2;
struct	FLB	*next_blk	= 0;
struct	FLB	*prev_blk	= 0;

/*
** Figure out where the new block should go.
*/
if ( flist_by_size.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, add it to the list
    ** with no further ado.
    */
    {
    flist_by_size.LhdA_Flink = (struct BHD *) new_blk;
    flist_by_size.LhdA_Blink = (struct BHD *) new_blk;
    new_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &flist_by_size;
    new_blk->FLB_A_BLINK_BY_SIZE = (struct FLB *) &flist_by_size;
    } /* end if-then */
else if ( new_blk->FLB_L_LENGTH <=
	((struct FLB *)(flist_by_size.LhdA_Flink))->FLB_L_LENGTH )
    {
    /*
    ** The block should be entered before the current first
    ** block in the list-by-address.
    **
    **	NOTE that if the block is the same size as the first block
    **	in the free-list-by-size, it will be inserted before the
    **	current first block.
    */
    next_blk = (struct FLB *) flist_by_size.LhdA_Flink;
    flist_by_size.LhdA_Flink = (struct BHD *) new_blk;
    new_blk->FLB_A_BLINK_BY_SIZE = (struct FLB *) &flist_by_size;
    new_blk->FLB_A_FLINK_BY_SIZE = next_blk;
    next_blk->FLB_A_BLINK_BY_SIZE = new_blk;
    }
else if ( flist_by_size.LhdW_ListCnt == 1 )
    {
    /*
    ** There's only one element in the list, and since the block
    ** to be inserted does not go before the current first block
    ** in the list, it must go after it.  This makes the new block
    ** the last element in the list.
    */
    prev_blk = (struct FLB *) flist_by_size.LhdA_Flink;
    prev_blk->FLB_A_FLINK_BY_SIZE = new_blk;
    new_blk->FLB_A_BLINK_BY_SIZE = prev_blk;
    new_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &flist_by_size;
    flist_by_size.LhdA_Blink = (struct BHD *) new_blk;
    }
else
    {
    /*
    ** The block should be entered somewhere past the first
    ** block in the list.  Figure out where the new block will go.
    */                                           
    prev_blk = (struct FLB *) flist_by_size.LhdA_Flink;/* get first block */
    next_blk = prev_blk->FLB_A_FLINK_BY_SIZE;
    for ( index = 0; index < flist_by_size.LhdW_ListCnt; index++ )
    	{
    	if ( prev_blk->FLB_L_LENGTH < new_blk->FLB_L_LENGTH && 
	     new_blk->FLB_L_LENGTH <= next_blk->FLB_L_LENGTH )
            /*
            ** We've found the right place:  the new block
            ** goes in between the current block and the next
            ** block;  therefore, insert it by address.
            **
            **	NOTE that if the block is the same size as the next_blk,
            **	it will be inserted before the next_blk, just as if it
            **	were smaller than it.
            */
            {
            prev_blk->FLB_A_FLINK_BY_SIZE = new_blk;
            new_blk->FLB_A_BLINK_BY_SIZE = prev_blk;
            new_blk->FLB_A_FLINK_BY_SIZE = next_blk;
            next_blk->FLB_A_BLINK_BY_SIZE = new_blk;
            break;
            } /* end if-then */
    	else
            /*
            ** Advance the previous and next block pointers.
            ** Detect end of list condition (when the index
            ** is two less than the list count).
            */
            {
            if ( index < (flist_by_size.LhdW_ListCnt - 2) )
            	/*
            	** Advance the pointers and loop again.
            	*/
            	{
            	prev_blk = next_blk;
            	next_blk = next_blk->FLB_A_FLINK_BY_SIZE;
            	} /* end if-then */
            else
            	/*
            	** End of list condition; the new block goes
            	** after the last block.  Insert it by address.
            	** Note that the "next_blk" pointer points to
            	** the last block in the list at this point.
            	*/
            	{
            	next_blk->FLB_A_FLINK_BY_SIZE = new_blk;
            	new_blk->FLB_A_BLINK_BY_SIZE = next_blk;
            	new_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &flist_by_size;
            	flist_by_size.LhdA_Blink = (struct BHD *) new_blk;
            	break;
            	} /* end if-else */
            } /* end if-else */
    	} /* end for-loop */
    } /* end outer-most if-else */

++flist_by_size.LhdW_ListCnt;

return;                                                    
} /* end of Link_flist_blk_by_size */


/******************************************************************************
**  Link_rlist_blk_by_adr()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Link a block into the returned list by ascending address,
**	i.e., by the address in the RLB_A_FIRST_BYTE field.
**
**  FORMAL PARAMETERS:
**
**	new_blk		RLB to link into the returned-list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_rlist()
******************************************************************************/
static void Link_rlist_blk_by_adr( new_blk )
struct	RLB	*new_blk;
{
int		 index;
int		 test1;
int		 test2;
struct	RLB	*next_blk	= 0;
struct	RLB	*prev_blk	= 0;
        
/*
** Figure out where the new block should go.
*/
if ( rlist_by_adr.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, add it to the list
    ** with no further ado.
    */
    {
    rlist_by_adr.LhdA_Flink = (struct BHD *) new_blk;
    rlist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
    new_blk->RLB_A_FLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
    new_blk->RLB_A_BLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
    } /* end if-then */
else if ( new_blk->RLB_A_FIRST_BYTE < 
	((struct RLB *)(rlist_by_adr.LhdA_Flink))->RLB_A_FIRST_BYTE )
    {
    /*
    ** The block should be entered before the current first
    ** block in the list-by-address.
    */
    next_blk = (struct RLB *) rlist_by_adr.LhdA_Flink;
    rlist_by_adr.LhdA_Flink = (struct BHD *) new_blk;
    new_blk->RLB_A_BLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
    new_blk->RLB_A_FLINK_BY_ADR = next_blk;
    next_blk->RLB_A_BLINK_BY_ADR = new_blk;
    }
else if ( rlist_by_adr.LhdW_ListCnt == 1 )
    {
    /*
    ** There's only one element in the list, and since the block
    ** to be inserted does not go before the current first block
    ** in the list, it must go after it.  This makes the new block
    ** the last element in the list.
    */
    prev_blk = (struct RLB *) rlist_by_adr.LhdA_Flink;
    prev_blk->RLB_A_FLINK_BY_ADR = new_blk;
    new_blk->RLB_A_BLINK_BY_ADR = prev_blk;
    new_blk->RLB_A_FLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
    rlist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
    }
else
    {
    /*
    ** The block should be entered somewhere past the first
    ** block in the list.  Figure out where the new block will go.
    */
    prev_blk = (struct RLB *) rlist_by_adr.LhdA_Flink;   /* get  first block */
    next_blk = prev_blk->RLB_A_FLINK_BY_ADR;
    for ( index = 0; index < rlist_by_adr.LhdW_ListCnt; index++ )
    	{
    	if ( prev_blk->RLB_A_FIRST_BYTE < new_blk->RLB_A_FIRST_BYTE && 
	     new_blk->RLB_A_FIRST_BYTE < next_blk->RLB_A_FIRST_BYTE )
            /*
            ** We've found the right place:  the new block
            ** goes in between the current block and the next
            ** block;  therefore, insert it by address.
            */
            {
            prev_blk->RLB_A_FLINK_BY_ADR = new_blk;
            new_blk->RLB_A_BLINK_BY_ADR = prev_blk;
            new_blk->RLB_A_FLINK_BY_ADR = next_blk;
            next_blk->RLB_A_BLINK_BY_ADR = new_blk;
            break;
            } /* end if-then */
    	else
            /*
            ** Advance the previous and next block pointers.
            ** Detect end of list condition (when the index
            ** is two less than the list count).
            */
            {
            if ( index < (rlist_by_adr.LhdW_ListCnt - 2) )
            	/*
            	** Advance the pointers and loop again.
            	*/
            	{
            	prev_blk = next_blk;
            	next_blk = next_blk->RLB_A_FLINK_BY_ADR;
            	} /* end if-then */
            else
            	/*
            	** End of list condition; the new block goes
            	** after the last block.  Insert it by address.
            	** Note that the "next_blk" pointer points to
            	** the last block in the list at this point.
            	*/
            	{
            	next_blk->RLB_A_FLINK_BY_ADR = new_blk;
            	new_blk->RLB_A_BLINK_BY_ADR = next_blk;
            	new_blk->RLB_A_FLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
            	rlist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
            	break;
            	} /* end if-else */
            } /* end if-else */
    	} /* end for-loop */
    } /* end outer-most if-else */

/*
** Increment the list count.
*/
++rlist_by_adr.LhdW_ListCnt;

return;
} /* end of Link_rlist_blk_by_adr */


/******************************************************************************
**  Link_rlist_blk_by_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Link a block into the returned list by ascending size,
**	i.e., by the address in the RLB_L_PAGESET_SIZE field.
**
**  FORMAL PARAMETERS:
**
**	new_blk		RLB to link into the returned-list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**  
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_rlist()
******************************************************************************/
static void Link_rlist_blk_by_size( new_blk )
struct	RLB	*new_blk;
{
int		 index;
int		 test1;
int		 test2;
struct	RLB	*next_blk	= 0;
struct	RLB	*prev_blk	= 0;

/*
** Figure out where the new block should go.
*/
if ( rlist_by_size.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, add it to the list
    ** with no further ado.
    */
    {
    rlist_by_size.LhdA_Flink = (struct BHD *) new_blk;
    rlist_by_size.LhdA_Blink = (struct BHD *) new_blk;
    new_blk->RLB_A_FLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
    new_blk->RLB_A_BLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
    } /* end if-then */
else if ( new_blk->RLB_L_PAGESET_SIZE <=
	  ((struct RLB *)(rlist_by_size.LhdA_Flink))->RLB_L_PAGESET_SIZE )
    {
    /*
    ** The block should be entered before the current first
    ** block in the list-by-address.
    **
    **	NOTE that if the block is the same size as the first block,
    **	it will be inserted as the new first block.
    */
    next_blk = (struct RLB *) rlist_by_size.LhdA_Flink;
    rlist_by_size.LhdA_Flink = (struct BHD *) new_blk;
    new_blk->RLB_A_BLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
    new_blk->RLB_A_FLINK_BY_SIZE = next_blk;
    next_blk->RLB_A_BLINK_BY_SIZE = new_blk;
    }
else if ( rlist_by_size.LhdW_ListCnt == 1 )
    {
    /*
    ** There's only one element in the list, and since the block
    ** to be inserted does not go before the current first block
    ** in the list, it must go after it.  This makes the new block
    ** the last element in the list.
    */
    prev_blk = (struct RLB *) rlist_by_size.LhdA_Flink;
    prev_blk->RLB_A_FLINK_BY_SIZE = new_blk;
    new_blk->RLB_A_BLINK_BY_SIZE = prev_blk;
    new_blk->RLB_A_FLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
    rlist_by_size.LhdA_Blink = (struct BHD *) new_blk;
    }
else
    {
    /*
    ** The block should be entered somewhere past the first
    ** block in the list.  Figure out where the new block will go.
    */
    prev_blk = (struct RLB *) rlist_by_size.LhdA_Flink;/* get first block */
    next_blk = prev_blk->RLB_A_FLINK_BY_SIZE;
    for ( index = 0; index < rlist_by_size.LhdW_ListCnt; index++ )
    	{
    	if ( prev_blk->RLB_L_PAGESET_SIZE < new_blk->RLB_L_PAGESET_SIZE && 
	     new_blk->RLB_L_PAGESET_SIZE <= next_blk->RLB_L_PAGESET_SIZE )
            /*
            ** We've found the right place:  the new block
            ** goes in between the current block and the next
            ** block;  therefore, insert it by address.
            **
            **	NOTE that if the block is equal in size to the next block
            **	it will be inserted before the next block, just as if it
            **	were smaller than it.
            */
            {
            prev_blk->RLB_A_FLINK_BY_SIZE = new_blk;
            new_blk->RLB_A_BLINK_BY_SIZE = prev_blk;
            new_blk->RLB_A_FLINK_BY_SIZE = next_blk;
            next_blk->RLB_A_BLINK_BY_SIZE = new_blk;
            break;
            } /* end if-then */
    	else
            /*
            ** Advance the previous and next block pointers.
            ** Detect end of list condition (when the index
            ** is two less than the list count).
            */
            {
            if ( index < (rlist_by_size.LhdW_ListCnt - 2) )
            	/*
            	** Advance the pointers and loop again.
            	*/
            	{
            	prev_blk = next_blk;
            	next_blk = next_blk->RLB_A_FLINK_BY_SIZE;
            	} /* end if-then */
            else
            	/*
            	** End of list condition; the new block goes
            	** after the last block.  Insert it by address.
            	** Note that the "next_blk" pointer points to
            	** the last block in the list at this point.
            	*/
            	{
            	next_blk->RLB_A_FLINK_BY_SIZE = new_blk;
            	new_blk->RLB_A_BLINK_BY_SIZE = next_blk;
            	new_blk->RLB_A_FLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
            	rlist_by_size.LhdA_Blink = (struct BHD *) new_blk;
            	break;
            	} /* end if-else */
            } /* end if-else */
    	} /* end for-loop */
    } /* end outer-most if-else */

/*
** Increment the list count.
*/
++rlist_by_size.LhdW_ListCnt;

return;
} /* end of Link_rlist_blk_by_size */


/******************************************************************************
**  Link_ulist_blk_by_adr()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Link a block into the userblock in-list, ordering it by address.
**
**  FORMAL PARAMETERS:
**
**	new_blk		New block to be added.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_ulist()
******************************************************************************/
static void Link_ulist_blk_by_adr( new_blk )
struct	FLB	*new_blk;
{
int		 index;
int		 test1;
int		 test2;
struct	FLB	*next_blk	= 0;
struct	FLB	*prev_blk	= 0;
        
/*
** Figure out where the new block should go.
*/
if ( ulist_by_adr.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, add it to the list
    ** with no further ado.
    */
    {
    ulist_by_adr.LhdA_Flink = (struct BHD *) new_blk;
    ulist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
    new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
    new_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
    } /* end if-then */
else if ( (int) new_blk < (int) ulist_by_adr.LhdA_Flink )
    {
    /*
    ** The block should be entered before the current first
    ** block in the list-by-address.
    */
    next_blk = (struct FLB *) ulist_by_adr.LhdA_Flink;
    ulist_by_adr.LhdA_Flink = (struct BHD *) new_blk;
    new_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
    new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) next_blk;
    next_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) new_blk;
    }
else if ( ulist_by_adr.LhdW_ListCnt == 1 )
    {
    /*
    ** There's only one element in the list, and since the block
    ** to be inserted does not go before the current first block
    ** in the list, it must go after it.  This makes the new block
    ** the last element in the list.
    */
    prev_blk = (struct FLB *) ulist_by_adr.LhdA_Flink;
    prev_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) new_blk;
    new_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) prev_blk;
    new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
    ulist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
    }
else
    {
    /*
    ** The block should be entered somewhere past the first
    ** block in the list.  Figure out where the new block will go.
    */
    prev_blk = (struct FLB *) ulist_by_adr.LhdA_Flink;/* get the first block */
    next_blk = (struct FLB *) prev_blk->FLB_A_FLINK_BY_ADR;
    for ( index = 0; index < ulist_by_adr.LhdW_ListCnt; index++ )
    	{
    	if ( prev_blk < new_blk && new_blk < next_blk )
            /*
            ** We've found the right place:  the new block
            ** goes in between the current block and the next
            ** block;  therefore, insert it by address.
            */
            {
            prev_blk->FLB_A_FLINK_BY_ADR = new_blk;
            new_blk->FLB_A_BLINK_BY_ADR = prev_blk;
            new_blk->FLB_A_FLINK_BY_ADR = next_blk;
            next_blk->FLB_A_BLINK_BY_ADR = new_blk;
            break;
            } /* end if-then */
    	else
            /*
            ** Advance the previous and next block pointers.
            ** Detect end of list condition (when the index
            ** is two less than the list count).
            */
            {
            if ( index < (ulist_by_adr.LhdW_ListCnt - 2) )
            	/*
            	** Advance the pointers and loop again.
            	*/
            	{
            	prev_blk = next_blk;
            	next_blk = next_blk->FLB_A_FLINK_BY_ADR;
            	} /* end if-then */
            else
            	/*
            	** End of list condition; the new block goes
            	** after the last block.  Insert it by address.
            	** Note that the "next_blk" pointer points to
            	** the last block in the list at this point.
            	*/
            	{
            	next_blk->FLB_A_FLINK_BY_ADR = new_blk;
            	new_blk->FLB_A_BLINK_BY_ADR = next_blk;
            	new_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
            	ulist_by_adr.LhdA_Blink = (struct BHD *) new_blk;
            	break;
            	} /* end if-else */
            } /* end if-else */                   
    	} /* end for-loop */
    } /* end outer-most if-else */

++ulist_by_adr.LhdW_ListCnt;
new_blk->FLB_W_FLAGS.FLB_V_ULIST_MEMBER = TRUE;

return;
} /* end of Link_ulist_blk_by_adr */


/******************************************************************************
**  Link_ulist_by_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Link a block into the userblock in-use-list, ordering it by size.
**
**  FORMAL PARAMETERS:
**
**	new_blk		Block to add to userblock in-use-list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	none (void)
**
**  SIGNAL CODES:          
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_ulist()
******************************************************************************/
static void Link_ulist_blk_by_size( new_blk )
struct	FLB	*new_blk;
{
int		 index;
int		 test1;
int		 test2;
struct	FLB	*next_blk	= 0;
struct	FLB	*prev_blk	= 0;

/*
** Figure out where the new block should go.
*/
if ( ulist_by_size.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, add it to the list
    ** with no further ado.
    */
    {
    ulist_by_size.LhdA_Flink = (struct BHD *) new_blk;
    ulist_by_size.LhdA_Blink = (struct BHD *) new_blk;
    new_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
    new_blk->FLB_A_BLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
    } /* end if-then */
else if ( new_blk->FLB_L_LENGTH <=
	((struct FLB *)(ulist_by_size.LhdA_Flink))->FLB_L_LENGTH )
    {
    /*
    ** The block should be entered before the current first
    ** block in the list-by-address.
    **
    **	NOTE that if the block is the same size as the first block
    **	in the free-list-by-size, it will be inserted before the
    **	current first block.
    */
    next_blk = (struct FLB *) ulist_by_size.LhdA_Flink;
    ulist_by_size.LhdA_Flink = (struct BHD *) new_blk;
    new_blk->FLB_A_BLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
    new_blk->FLB_A_FLINK_BY_SIZE = next_blk;
    next_blk->FLB_A_BLINK_BY_SIZE = new_blk;
    }
else if ( ulist_by_size.LhdW_ListCnt == 1 )
    {
    /*
    ** There's only one element in the list, and since the block
    ** to be inserted does not go before the current first block
    ** in the list, it must go after it.  This makes the new block
    ** the last element in the list.
    */
    prev_blk = (struct FLB *) ulist_by_size.LhdA_Flink;
    prev_blk->FLB_A_FLINK_BY_SIZE = new_blk;
    new_blk->FLB_A_BLINK_BY_SIZE = prev_blk;
    new_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
    ulist_by_size.LhdA_Blink = (struct BHD *) new_blk;
    }
else
    {
    /*
    ** The block should be entered somewhere past the first
    ** block in the list.  Figure out where the new block will go.
    */                                           
    prev_blk = (struct FLB *) ulist_by_size.LhdA_Flink;/* get first block */
    next_blk = prev_blk->FLB_A_FLINK_BY_SIZE;
    for ( index = 0; index < ulist_by_size.LhdW_ListCnt; index++ )
    	{
    	if ( prev_blk->FLB_L_LENGTH < new_blk->FLB_L_LENGTH && 
	     new_blk->FLB_L_LENGTH <= next_blk->FLB_L_LENGTH )
            /*
            ** We've found the right place:  the new block
            ** goes in between the current block and the next
            ** block;  therefore, insert it by address.
            **
            **	NOTE that if the block is the same size as the next_blk,
            **	it will be inserted before the next_blk, just as if it
            **	were smaller than it.
            */
            {
            prev_blk->FLB_A_FLINK_BY_SIZE = new_blk;
            new_blk->FLB_A_BLINK_BY_SIZE = prev_blk;
            new_blk->FLB_A_FLINK_BY_SIZE = next_blk;
            next_blk->FLB_A_BLINK_BY_SIZE = new_blk;
            break;
            } /* end if-then */
    	else
            /*
            ** Advance the previous and next block pointers.
            ** Detect end of list condition (when the index
            ** is two less than the list count).
            */
            {
            if ( index < (ulist_by_size.LhdW_ListCnt - 2) )
            	/*
            	** Advance the pointers and loop again.
            	*/
            	{
            	prev_blk = next_blk;
            	next_blk = next_blk->FLB_A_FLINK_BY_SIZE;
            	} /* end if-then */
            else
            	/*
            	** End of list condition; the new block goes
            	** after the last block.  Insert it by address.
            	** Note that the "next_blk" pointer points to
            	** the last block in the list at this point.
            	*/
            	{
            	next_blk->FLB_A_FLINK_BY_SIZE = new_blk;
            	new_blk->FLB_A_BLINK_BY_SIZE = next_blk;
            	new_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
            	ulist_by_size.LhdA_Blink = (struct BHD *) new_blk;
            	break;
            	} /* end if-else */
            } /* end if-else */
    	} /* end for-loop */
    } /* end outer-most if-else */

++ulist_by_size.LhdW_ListCnt;
new_blk->FLB_W_FLAGS.FLB_V_ULIST_MEMBER = TRUE;

return;                                                    
} /* end of Link_ulist_blk_by_size */


/******************************************************************************
**  Merge_flist_block()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Merge (or compact) a block (not yet in the free-list) with
**	all free-list blocks that are adjacent to the input block.
**	The input block is typically a new block to be added to the
**	free-list (e.g., having been returned from user usage).
**
**	    NOTE that this compaction is on an adjacent address
**	    basis (which is the only possible basis anyway).
**
**	Blocks merged with the input block will first be removed from
**	the free-list, and then merged with the input block.  This 
**	function DOES NOT add blocks to the free-list.
**
**	This function returns what is called a "merged block", which
**	is the block to be added to the free-list.  This may be either
**	the input block if no merges took place, or the result of a
**	merge operation.
**
**  FORMAL PARAMETERS:
**
**      new_blk		New block to be merged with existing blocks.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      merged_blk	Resulting block from the merge functions.
**			This may also be the original if no merge takes place.
**			Passed by reference.
**                     
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
******************************************************************************/
static struct FLB *Merge_flist_block( new_blk )
struct	FLB	*new_blk;
{
int		 index;
int		 test1;
int	   	 test2;
struct	FLB	*cur_blk	= 0;
struct	FLB	*first_blk	= 0;
struct	FLB	*merged_blk;
struct	FLB	*next_blk	= 0;
struct	FLB	*prev_blk	= 0;
struct	FLB	*second_blk	= 0;
struct	FLB	*third_blk	= 0;

/*
** Figure out where the new block would go if it were to be
** inserted into the free-list without being merged with
** another block.  (Merge testing is the next step.)
*/
if ( flist_by_adr.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, don't do anything else.
    */
    { /* begin if-level 1 */
    merged_blk = new_blk;		    /* The new_blk will be returned */
    } /* end if-level 1 */
else 
    { /* begin if-level 1 */
    if ( flist_by_adr.LhdW_ListCnt == 1 )
    	/*
    	** Since there's only one block, figure out whether it goes
    	** before or after.
    	*/
    	{ /* begin if-level 2 */
    	cur_blk = (struct FLB *) flist_by_adr.LhdA_Flink;
    	if ( cur_blk->FLB_A_FLINK_BY_ADR < new_blk->FLB_A_FLINK_BY_ADR )
            { /* begin if-level 3 */
            first_blk = cur_blk;	    /* goes after   */
	    second_blk = new_blk;
	    third_blk = 0;
            } /* end if-level 3 */
    	else
            { /* begin if-level 3 */
	    first_blk = 0;		    /* goes before  */
            second_blk = new_blk;
	    third_blk = cur_blk;
            } /* end if-level 3 */
    	} /* end if-level 2 */
    else
    	/*
    	** There are a least two elements in the list already.
    	** Figure out where the new block will go.
    	*/
    	{ /* begin if-level 2 */
    	prev_blk = (struct FLB *) flist_by_adr.LhdA_Flink;/* get first block */
    	next_blk = prev_blk->FLB_A_FLINK_BY_ADR;
	/*
	** Should it go before the first block in the list?
	*/
	if ( new_blk < prev_blk )
	    { /* begin if-level 3 */
	    /*
	    ** Yes, it goes before the first block.
	    */
	    first_blk = 0;
	    second_blk = new_blk;
	    third_blk = prev_blk;
	    } /* end if-level 3 */
	/*
	** Should it go after the last block in the list?
	*/
	else if ( (int) new_blk > (int) flist_by_adr.LhdA_Blink )
	    {
	    first_blk = (struct FLB *) flist_by_adr.LhdA_Blink;
	    second_blk = new_blk;
	    third_blk = (struct FLB *) HIGHEST_P0_ADDRESS;
	    }
	/*
	** It goes after the first block, but before the last one,
	** so check out the rest of the list.
	*/
	else
	    { /* begin if-level 3 */
	    for ( index = 0; index < flist_by_adr.LhdW_ListCnt; index++ )
		{ /* begin for-level 1 */
		if ( prev_blk < new_blk && new_blk < next_blk )
		    /*
		    ** We've found the right place:  the new block
		    ** goes in between the current block and the next
		    ** block.
		    */
		    { /* begin if-level 4 */
		    first_blk = prev_blk;
		    second_blk = new_blk;
		    third_blk = next_blk;
		    break;
		    } /* end if-level 4 */
		else
		    /*
		    ** Advance the previous and next block pointers.
		    ** NOTE that there is no need to check for an end-
		    ** of-list condition, since that was already taken
		    ** care of before this loop was entered.
		    */
		    { /* begin if-level 4 */
		    prev_blk = next_blk;
		    next_blk = next_blk->FLB_A_FLINK_BY_ADR;
		    } /* end if-level 4 */
		} /* end for-level 1 */
	    } /* end if-level 3 */
    	} /* end if if-level 2 */
    
    /*
    ** Now that we know where the block will go, test to see if
    ** it should be merged with either or both adjacent blocks.
    ** If it should be merged, then merge it.
    */
    test1 = Merge_flist_test( first_blk, second_blk );	/* Save the test values	*/
    test2 = Merge_flist_test( second_blk, third_blk );	/* for multiple use.	*/
    if ( test1 && test2)
    	{ /* begin if-level 2 */
    	/*              
        ** Unlink the previous and next blocks from the list.
        ** Merge new block with previous and next blocks, which
	** are the first and third blocks.
        */
        prev_blk = Unlink_flist_blk( first_blk );
        next_blk = Unlink_flist_blk( third_blk );
        merged_blk = Merge_flist_blocks( first_blk, second_blk );
        merged_blk = Merge_flist_blocks( merged_blk, third_blk );
        } /* end if-level 2 */
    else if ( test1 )
        { /* begin if-level 2 */
        /*
        ** Merge the first and second blocks. 
        */
        first_blk = Unlink_flist_blk( first_blk );
        merged_blk = Merge_flist_blocks( first_blk, second_blk );
        } /* end if-level 2 */
    else if ( test2 )
        { /* begin if-level 2 */
        /* 
        ** Merge the second and third blocks.
        */
        third_blk = Unlink_flist_blk( third_blk );
        merged_blk = Merge_flist_blocks( second_blk, third_blk );
        } /* end if-level 2 */
    else
        /*
        ** Don't merge any blocks -- just return the new block.
        */
        merged_blk = new_blk;
    } /* end if-level 1 */

/*
** Block header update:
**
**	* block may or may not be a pageset
*/
Pageset_test( merged_blk );

return merged_blk;
} /* end of Merge_flist_block */


/******************************************************************************
**  Merge_flist_blocks()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Merge two blocks (adjacent by address) in the free-list.
**	In effect, append the second to the first and adjust the
**	header of the first.
**
**  FORMAL PARAMETERS:
**
**      first_blk	First block of the merge pair.  The address identity
**			of this block is maintained.
**			Passed by reference.
**	second_blk	Second block of the merge pair.  This block is
**			appended to the first block.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      merged_blk	Resultant block from the merger of the two input
**			blocks.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Merge_flist_block()
******************************************************************************/
static struct FLB *Merge_flist_blocks( first_blk, second_blk )
struct	FLB	*first_blk;
struct	FLB	*second_blk;
{
struct	FLB	*merged_blk;

merged_blk = first_blk;                                     
merged_blk->FLB_L_LENGTH += second_blk->FLB_L_LENGTH;
merged_blk->FLB_W_FLAGS.FLB_V_DZERO_CLRD = FALSE;
if ( second_blk->FLB_W_FLAGS.FLB_V_LAST_LSTBLK )
    merged_blk->FLB_W_FLAGS.FLB_V_LAST_LSTBLK = TRUE;
if ( second_blk->FLB_W_FLAGS.FLB_V_PAGE_END )
    merged_blk->FLB_W_FLAGS.FLB_V_PAGE_END = TRUE;

return merged_blk;
} /* end of Merge_flist_blocks */


/******************************************************************************
**  Merge_flist_test()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Test to see if two freelist blocks should be merged, i.e., 
**	determine if the two blocks are physically adjacent in memory.
**
**  FORMAL PARAMETERS:
**
**      first_blk	Block with lower starting address.
**			Passed by reference.
**	second_blk	Block with higher starting address.
**			Passed by reference.
**
**			NOTE: no address comparison is done, so the addresses
**			MUST be supplied in the above order.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      status		False (0) if blocks are not adjacent;
**			True (1) if blocks are adjacent and should be merged.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Merge_flist_block()
******************************************************************************/
static int Merge_flist_test( first_blk, second_blk )
struct	FLB	*first_blk;
struct	FLB	*second_blk;
{
int    	status	= FALSE;

/*
** Blocks should be merged if the address of the first block plus
** the block length is equal to the address of the second block.
**
**	NOTE that the address of the second_blk may be zero, which
**	means that the first block will not be merged with anything.
*/
if ( first_blk != NULL )
    if ( ((int)first_blk + first_blk->FLB_L_LENGTH) == (int)second_blk )
    	status = TRUE;

return status;
} /* end of Merge_flist_test */
                         

/******************************************************************************
**  Merge_rlist_block()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Merge (or compact) a block (not yet in the returned-list) with
**	all returned-list blocks that are adjacent to the input block.
**
**	Blocks merged with the input block will first be removed from
**	the returned-list, and then merged with the input block.  This 
**	function DOES NOT add blocks to the returned-list.
**
**	This function returns what is called a "merged block", which
**	is the block to be added to the returned-list.  This may be either
**	the input block if no merges took place, or the result of a
**	merge operation.
**
**  FORMAL PARAMETERS:
**
**      new_blk		New block to be merged with existing blocks.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      merged_blk	Resulting block from the merge functions.
**			This may also be the original if no merge takes place.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_rlist()
******************************************************************************/
static struct RLB *Merge_rlist_block( new_blk )
struct	RLB   	*new_blk;
{
int		 index;
int		 test1;
int   		 test2;
struct	RLB	*cur_blk	= 0;
struct	RLB	*first_blk	= 0;
struct	RLB	*merged_blk;
struct	RLB	*next_blk	= 0;
struct	RLB	*prev_blk	= 0;
struct	RLB	*second_blk	= 0;
struct	RLB	*third_blk	= 0;

/*
** Figure out where the new block would go if it were to be
** inserted into the returned-list without being merged with
** another block.  (Merge testing is the next step.)
*/                           
if ( rlist_by_adr.LhdW_ListCnt == 0 )
    /*
    ** The list is empty, so the new block is the only
    ** element in the list.  Therefore, don't do anything else.
    */          
    { /* begin if-level 1 */
    merged_blk = new_blk;
    } /* end if-level 1 */
else
    { /* begin if-level 1 */ 
    if ( rlist_by_adr.LhdW_ListCnt == 1 )
    	/*
    	** Since there's only one block, figure out whether it goes
    	** before or after.
    	*/
    	{ /* begin if-level 2 */
    	cur_blk = (struct RLB *) rlist_by_adr.LhdA_Flink;
    	if ( cur_blk->RLB_A_FIRST_BYTE < new_blk->RLB_A_FIRST_BYTE )
            { /* begin if-level 3 */
            first_blk = cur_blk;	    /* goes after   */
	    second_blk = new_blk;
            third_blk = 0;
            } /* end if-level 3 */
    	else
            { /* begin if-level 3 */
            first_blk = 0;		    /* goes before  */
	    second_blk = new_blk;
            third_blk = cur_blk;
            } /* end if-level 3 */
    	} /* end if-level 2 */
    else
    	{ /* begin if-level 2 */
    	/*
    	** There are a least two elements in the list already.
    	** Figure out where the new block will go.
    	*/
    	prev_blk = (struct RLB *) rlist_by_adr.LhdA_Flink;/* get first block */
    	next_blk = prev_blk->RLB_A_FLINK_BY_ADR;
	/*
	** Should the new block go before the first block in the list?
	*/
	if ( new_blk->RLB_A_FIRST_BYTE < prev_blk->RLB_A_FIRST_BYTE )
	    { /* begin if-level 3 */
	    /*
	    ** Yes, it goes before the first block.
	    */
	    first_blk = 0;
	    second_blk = new_blk;
	    third_blk = prev_blk;
	    } /* end if-level 3 */
	/*
	** Should the new block go after the last block in the list?
	*/
	else if ( new_blk->RLB_A_FIRST_BYTE > 
		  ((struct RLB*)(rlist_by_adr.LhdA_Blink))->RLB_A_FIRST_BYTE )
	    {
	    first_blk = (struct RLB *) rlist_by_adr.LhdA_Blink;
	    second_blk = new_blk;
	    third_blk = &highest_rlb_by_adr;
	    }
	/*
	** The block goes after the first block, but before the last
	** one, so check out the rest of the list.
	*/
	else
	    { /* begin if-level 3 */
	    for ( index = 0; index < rlist_by_adr.LhdW_ListCnt; index++ )
		{ /* begin for-level 1 */
		if ( prev_blk->RLB_A_FIRST_BYTE < new_blk->RLB_A_FIRST_BYTE && 
		     new_blk->RLB_A_FIRST_BYTE < next_blk->RLB_A_FIRST_BYTE )
		    /*
		    ** We've found the right place:  the new block
		    ** goes in between the current block and the next
		    ** block.
		    */
		    { /* begin if-level 4 */
		    first_blk = prev_blk;
		    second_blk = new_blk;
		    third_blk = next_blk;
		    break;
		    } /* end if-level 4 */
		else
		    /*
		    ** Advance the previous and next block pointers.
		    */
		    { /* begin if-level 4 */
		    prev_blk = next_blk;
		    next_blk = next_blk->RLB_A_FLINK_BY_ADR;
		    } /* end if-level 4 */
		} /* end for-level 1 */
	    } /* end if-level 3 */
    	} /* end if-level 2 */
    
    /*
    ** Now that we have inserted the new block, test to see if
    ** it should be merged with either or both adjacent blocks.
    ** If it should be merged, then merge it.
    */
    test1 = Merge_rlist_test( first_blk, second_blk );	/* Save the test values	*/
    test2 = Merge_rlist_test( second_blk, third_blk );	/* for multiple use.	*/
    if ( test1 && test2 )
    	{ /* begin if-level 2 */
        /*              
        ** Unlink the previous and next blocks from the list.
        ** Merge new block with previous and next blocks.
        */
        prev_blk = Unlink_rlist_blk( first_blk );
        next_blk = Unlink_rlist_blk( third_blk );
        merged_blk = Merge_rlist_blocks( first_blk, second_blk );
        merged_blk = Merge_rlist_blocks( merged_blk, third_blk );
        } /* end if-level 2 */
    else if ( test1 )
        { /* begin if-level 2 */
        /*
        ** Merge the first and second blocks.
        */
        first_blk = Unlink_rlist_blk( first_blk );
        merged_blk = Merge_rlist_blocks( first_blk, second_blk );
        } /* end if-level 2 */
    else if ( test2 )
        { /* begin if-level 2 */
        /*
        ** Merge the second and third blocks.
        */
        third_blk = Unlink_rlist_blk( third_blk );
        merged_blk = Merge_rlist_blocks( second_blk, third_blk );
        } /* end if-level 2 */
    else
        /*
        ** Don't merge any blocks -- just return the new block.
        */
        merged_blk = new_blk;
    } /* end if-level 1 */

return merged_blk;
} /* end of Merge_rlist_block */


/******************************************************************************
**  Merge_rlist_blocks()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Merge two blocks (adjacent by address) in the returned-list.
**	In effect, append the deallocated pageset pointed to by the second 
**	rlb to the deallocated pageset pointed to by the first rlb, and 
**	return the second rlb to the free-list.
**
**  FORMAL PARAMETERS:
**
**      first_blk	First block of the merge pair.  The address of
**			the pageset pointed to by this block retained.
**			Passed by reference.
**	second_blk	Second block of the merge pair.  The block pointed
**			to by this block is appended to the block pointed to
**			by the first block.  This block is then returned to
**			the free-list.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      merged_blk	Resultant block from the merger of the two input
**			blocks.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Merge_rlist_block()
******************************************************************************/
static struct RLB *Merge_rlist_blocks( first_blk, second_blk )
struct	RLB	*first_blk;
struct	RLB	*second_blk;
{
struct	RLB	*merged_blk;

/*
** Since the pageset pointed to by the second block adjoins the
** pageset of the first block, simply increase the pageset size
** of the first block by the amount of the second block.
*/
merged_blk = first_blk;                                     
merged_blk->RLB_L_PAGESET_SIZE += second_blk->RLB_L_PAGESET_SIZE;
merged_blk->RLB_A_LAST_BYTE = second_blk->RLB_A_LAST_BYTE;
/*
** Now delete the second returned-list block by returning it to the free-list.
*/
Add_blk_to_flist( (struct FLB *)second_blk );

return merged_blk;
} /* end of Merge_rlist_blocks */


/******************************************************************************
**  Merge_rlist_test()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Test to see if the memory pointed to by two returned-list blocks 
**	should be merged, i.e., determine if the pagesets pointed to by
**	the two blocks are physically adjacent in memory.
**
**  FORMAL PARAMETERS:
**
**      first_blk	RLB pointing to the block with the lower starting 
**			address.
**			Passed by reference.
**	second_blk	RLB pointing to the block with the higher starting 
**			address.
**			Passed by reference.
**
**			NOTE: no address comparison is done, so the addresses
**			MUST be supplied in the above order.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      status		False (0) if blocks are not adjacent;
**			True (1) if blocks are adjacent and should be merged.
**
**  SIGNAL CODES:
**
**      none
**                          
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Merge_rlist_block()
******************************************************************************/
static int Merge_rlist_test( first_blk, second_blk )
struct	RLB	*first_blk;
struct	RLB	*second_blk;
{
int    	status	= FALSE;

/*
** Blocks should be merged if the address of the first block (or pageset) 
** plus the block (pageset) length is equal to the address of the second block.
**
**	NOTE that either FIRST_BYTE address may be zero, which
**	means that the blocks will not be merged.
*/
if ( first_blk != NULL && second_blk != NULL )
    if ( (first_blk->RLB_A_FIRST_BYTE + first_blk->RLB_L_PAGESET_SIZE) == 
          second_blk->RLB_A_FIRST_BYTE )
    	status = TRUE;

return status;
} /* end of Merge_rlist_test */
                         

/******************************************************************************
**  Pageset_test()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Test a block to see if it is a pageset.  (This functions also
**	sets the page begin/end flags to the appropriate values.)
**	A block is a pageset if:
**
**		* it begins on a page boundary
**		* it ends on a page boundary
**		* the page begin/end flags are set
**
**	At present, a block that contains a full pageset but does not
**	meet the above conditions will fail the test.
**
**  FORMAL PARAMETERS:
**
**      memblk		Memory block to test as a pageset.
**			Pass by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      status		True (1) if the block is a pageset.
**			False (0) if the block fails the test.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
**	Merge_flist_block()
**	Realloc_down()
******************************************************************************/
static int Pageset_test( memblk )
struct	FLB	*memblk;
{
int	status	= FALSE;

if ( PAGE_BOUNDARY_TEST_( memblk ) )
    memblk->FLB_W_FLAGS.FLB_V_PAGE_BEGIN = TRUE;
else
    memblk->FLB_W_FLAGS.FLB_V_PAGE_BEGIN = FALSE;

if ( PAGE_BOUNDARY_TEST_( (int)memblk + memblk->FLB_L_LENGTH ) )
    memblk->FLB_W_FLAGS.FLB_V_PAGE_END = TRUE;
else
    memblk->FLB_W_FLAGS.FLB_V_PAGE_END = FALSE;

if ( memblk->FLB_W_FLAGS.FLB_V_PAGE_BEGIN && 
     memblk->FLB_W_FLAGS.FLB_V_PAGE_END )
    status = TRUE;

return status;
} /* end of Pageset_test */


/******************************************************************************
**  Realloc_down()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reallocate a user buffer, making it smaller.  This function will
**	actually only make pageset blocks smaller by an integral number
**	of (512 byte) blocks.
**
**	Userblocks that cannot be reduced by 2 or more blocks will be
**	unchanged.  Userblocks that are not pagesets will also remain
**	unchanged.  This function presumes that memory will be reallocated
**	in smaller sizes when an operation has completed on a large
**	buffer of image data, and the extra memory is no longer needed.
**	[Compression is a typical example of this.]  Large buffers for
**	image data will be allocated as pagesets in the first place.
**
**  FORMAL PARAMETERS:
**
**	[@description_or_none@]
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	new_userbuf_adr	    Smaller buffer returned to the application.
**			    Passed by reference.  (This is actually the
**			    address of the buffer that was passed in;
**			    which buffer will have been shortened if possible.)
**
**  SIGNAL CODES:
**
**	ImgX_MEMREALER	    Memory reallocation error.  This is principly
**			    caused by an attempt to reallocate a block
**			    of memory that is not a page set.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	_ImgRealloc
******************************************************************************/
static struct FLB *Realloc_down( userbuf_adr, resize )
char	*userbuf_adr;
int	 resize;
{
char	    *new_userbuf_adr    = userbuf_adr;
int	     remainderblk_size;
int	     userblk_size;
int	     userbuf_size;
int	     working_resize;
struct	FLB *remainderblk;
struct	FLB *userblk		= (struct FLB *) (userbuf_adr - FLB_K_SIZE);

/*
** Take the current user block out of the user block list.
*/
Unlink_ulist_blk( userblk );

/*
** How long is the actual user block?  Also determine the resize
** length of the block to be returned, rounded up (in bytes) to the
** next block (or page).  Note that since the resize size is actually
** the size of the user-data buffer, the FLB listhead size has to be
** added.
*/
userblk_size = userblk->FLB_L_LENGTH;
working_resize = ROUNDUP_PAGESIZE_( resize + FLB_K_SIZE );

/*
** Verify that the user block is a page set, and that it is at least
** greater in size that the requested size by IMG_K_MAX_FLB_SIZE in bytes.
** Shorten the user block if these tests come back true.
*/
if ( Pageset_test( userblk ) )
    if ( (userblk_size - working_resize) > IMG_K_MAX_FLB_SIZE )
	{
	/*
	** Figure out and then 'cut off' the remainder, putting the
	** remainder onto the return-list.  Also resize the user block.
	*/
	remainderblk_size = userblk_size - working_resize;

	/*
	** Calculate address of remainder block, and initialize it
	** as an FLB and a pageset block.  Then return the remainder 
	** block to unused memory.
	*/
	remainderblk = (struct FLB *) ((int)userblk + working_resize);
	Initialize_flb( remainderblk, remainderblk_size );
	if ( !Pageset_test( remainderblk ) )
	    ChfStop( 1,  ImgX_MEMREALER );
	Add_blk_to_flist( remainderblk );

	/*
	** Resize user block and do a pageset test to verify
	** that everything's OK.
	*/
	userblk->FLB_L_LENGTH = working_resize;
	if ( !Pageset_test( userblk ) )
	    ChfStop( 1,  ImgX_MEMREALER );		    /* This shouldn't happen*/
	}
/*
** Since only the end of the original user block was chopped off,
** the user buffer address is still the same as it was before.
** First, though, return it to the user block list.
*/
Add_blk_to_ulist( userblk );

return (struct FLB *) new_userbuf_adr;
} /* end of Realloc_down */


/******************************************************************************
**  Reclaim_p0_flb()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Reclaim a deallocated pageset from P0 space, and return the pageset
**	as a free-list block (even though it may not be immediately placed
**	on the free-list).
**
**	The pageset address range to reallocate is passed in via the rlb
**	argument.  
**
**  FORMAL PARAMETERS:
**
**  	rlb		Returned list block pointing to the pageset
**			to reclaim from already deallocated P0 memory.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	flb		Free-list block that was reclaimed from P0 space.
**			Passed by reference.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Find_rlist_best_fit()
******************************************************************************/
static struct FLB *Reclaim_p0_flb( rlb )
struct	RLB	*rlb;
{
#if defined(__VMS) || defined(VMS)
int		 status;
struct	FLB	*flb;
struct {
    long	begin_adr;
    long	end_adr;
    }		 retadr;

/*
** Reallocate the deallocated memory with $CRETVA.  Note that only memory
** in the returned list will ever be reallocated by $CRETVA.  This is because
** it's the only memory that is known to IMM (apart from what is in the
** free-list and what has been handed out to users as user-blocks).
**
** As was stated elsewhere, never-before-allocated memory is acquired by
** using $EXPREG.
*/
status = sys$cretva( &(rlb->RLB_A_FIRST_BYTE), &retadr, 0 );
if ( !VMS_STATUS_SUCCESS_( status ) )
    ChfStop( 1,  status );

/*
** The reclaimed block is now the valid flb to return.  Initialize
** it as a new flb taken from p0 space.
*/
flb = (struct FLB *)retadr.begin_adr;
Initialize_p0_flb( flb, rlb->RLB_L_PAGESET_SIZE );

/*
** Return the unlinked input rlb to the free-list since it's not
** needed any longer.
*/
Add_blk_to_flist( rlb );

return flb;
#endif
} /* end of Reclaim_p0_blk */


/******************************************************************************
**  Roundup_blocksize()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Round up the size value for requested dynamic memory to include
**	a block header and to include any padding bytes necessary to
**	make the block quadword aligned.
**
**	If the input size exceeds IMG_K_MAX_FLB_SIZE, round up the size
**	to the nearest page.
**
**  FORMAL PARAMETERS:
**
**      size		Requested size of memory buffer in bytes.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      working_size	Rounded up size of the memory buffer -- actual
**			size of memory block to allocate (including block
**			header).
**
**  SIGNAL CODES:
**
**      ImgX_INVALLSIZ		Invalid memory allocation size
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Allocate_memory()
******************************************************************************/
static int Roundup_blocksize( size )
int	size;
{
int	padding;
int	remainder;
int	working_size;

/*
** If the size value is zero, forget it.
*/
if ( size == 0 )
    ChfStop( 1,  ImgX_INVALLSIZ );		/* invalid allocation size	*/

/*
** Calculate padding (for quadword alignment), and then
** calculate working size, adding the input size, the
** padding size and the block header size together.
*/                  
remainder = size % 8;			/* bytes used in last quadword	*/
padding = remainder? (8-remainder): 0;	/* padding if not qword aligned	*/
working_size = size + padding + FLB_K_SIZE;

/*
** Is the working size larger than the max flb size?  If so, then round
** it up to the nearest page boundary.
*/
if ( working_size > IMG_K_MAX_FLB_SIZE )
    {
    working_size = ROUNDUP_PAGESIZE_( working_size );
    }

return working_size;
} /* end of Roundup_blocksize */


/******************************************************************************
**  Unlink_flist_blk()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the free-list.  The block must be unlinked
**	from both free-list list-heads (by size and by address).
**
**  FORMAL PARAMETERS:
**
**      flist_blk	Free-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      flist_blk	Unlinked free-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**                                   
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Find_flist_best_fit()
**	Merge_flist_block()
******************************************************************************/
static struct FLB *Unlink_flist_blk( flist_blk )
struct	FLB	*flist_blk;
{
struct	FLB	*next_blk;
struct	FLB	*prev_blk;

/*
** Unlink from the free-list by size list.
*/
Unlink_flist_blk_by_size( flist_blk );

/*
** Unlink from the free-list by address list.
*/
Unlink_flist_blk_by_adr( flist_blk );

/*
** Clear list member flag                 
*/
flist_blk->FLB_W_FLAGS.FLB_V_FLIST_MEMBER = FALSE;

return flist_blk;
} /* end of Unlink_flist_blk */


/******************************************************************************
**  Unlink_flist_blk_by_adr()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the free-list by address.  
**	
**
**  FORMAL PARAMETERS:
**
**      flist_blk	Free-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      flist_blk	Unlinked free-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Unlink_flist_blk()
******************************************************************************/
static struct FLB *Unlink_flist_blk_by_adr( flist_blk )
struct	FLB	*flist_blk;
{
struct	FLB	*next_blk;
struct	FLB	*prev_blk;

/*
** Unlink from the free-list by address list.
*/
if ( flist_blk == (struct FLB *) flist_by_adr.LhdA_Flink && 
     flist_blk == (struct FLB *) flist_by_adr.LhdA_Blink )
    /*
    ** The block is the only element in the list, and is therefore
    ** both the first and the last block.  Set the list head to point
    ** to itself.
    */
    {
    flist_by_adr.LhdA_Flink = (struct BHD *)&flist_by_adr;
    flist_by_adr.LhdA_Blink = (struct BHD *)&flist_by_adr;
    }
else if ( flist_blk == (struct FLB *) flist_by_adr.LhdA_Flink )
    /*
    ** The block is the first block in the list.
    */
    {
    next_blk = flist_blk->FLB_A_FLINK_BY_ADR;
    next_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) &flist_by_adr;
    flist_by_adr.LhdA_Flink = (struct BHD *) next_blk;
    }
else if ( flist_blk == (struct FLB *) flist_by_adr.LhdA_Blink )
    /*
    ** The block is the last block in the list.
    */
    {
    prev_blk = flist_blk->FLB_A_BLINK_BY_ADR;
    prev_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &flist_by_adr;
    flist_by_adr.LhdA_Blink = (struct BHD *) prev_blk;
    }
else
    /*
    ** The block is somewhere between the end-points of the list.
    */
    {
    prev_blk = flist_blk->FLB_A_BLINK_BY_ADR;
    next_blk = flist_blk->FLB_A_FLINK_BY_ADR;
    prev_blk->FLB_A_FLINK_BY_ADR = next_blk;
    next_blk->FLB_A_BLINK_BY_ADR = prev_blk;
    }

--flist_by_adr.LhdW_ListCnt;			/* decrement the list count */
flist_blk->FLB_A_FLINK_BY_ADR = flist_blk;	/* point the block to itself */
flist_blk->FLB_A_BLINK_BY_ADR = flist_blk;  

return flist_blk;
} /* end of Unlink_flist_blk_by_adr */


/******************************************************************************
**  Unlink_flist_blk_by_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the free-list by size.
**
**  FORMAL PARAMETERS:
**
**      flist_blk	Free-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**                                         
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      flist_blk	Unlinked free-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Unlink_flist_blk()
******************************************************************************/
static struct FLB *Unlink_flist_blk_by_size( flist_blk )
struct	FLB	*flist_blk;
{
struct	FLB	*next_blk;
struct	FLB	*prev_blk;

/*
** Unlink from the free-list by size list.
**
**	Note that there are special case conditions if the block
**	to unlink is at either end of the list since the list-head
**	flink & blink structure does not correspond in position to the 
**	FLB flink & blink structure.
*/
if ( flist_blk == (struct FLB *) flist_by_size.LhdA_Flink && 
     flist_blk == (struct FLB *) flist_by_size.LhdA_Blink )
    /*
    ** The block is the only element in the list, and is therefore
    ** both the first and the last block.
    */
    {          
    flist_by_size.LhdA_Flink = (struct BHD *)&flist_by_size;
    flist_by_size.LhdA_Blink = (struct BHD *)&flist_by_size;
    }
else if ( flist_blk == (struct FLB *) flist_by_size.LhdA_Flink )
    /*
    ** The block is the first block in the list.
    */
    {
    next_blk = flist_blk->FLB_A_FLINK_BY_SIZE;
    next_blk->FLB_A_BLINK_BY_SIZE = (struct FLB *) &flist_by_size;
    flist_by_size.LhdA_Flink = (struct BHD *) next_blk;
    }
else if ( (int) flist_blk == (int) flist_by_size.LhdA_Blink )
    /*
    ** The block is the last block in the list.
    */
    {
    prev_blk = flist_blk->FLB_A_BLINK_BY_SIZE;
    prev_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &flist_by_size;
    flist_by_size.LhdA_Blink = (struct BHD *) prev_blk;
    }
else
    /*
    ** The block is somewhere between the end-points of the list.
    */
    {
    prev_blk = flist_blk->FLB_A_BLINK_BY_SIZE;
    next_blk = flist_blk->FLB_A_FLINK_BY_SIZE;
    prev_blk->FLB_A_FLINK_BY_SIZE = next_blk;
    next_blk->FLB_A_BLINK_BY_SIZE = prev_blk;
    }

--flist_by_size.LhdW_ListCnt;			/* decrement the list count */
flist_blk->FLB_A_FLINK_BY_SIZE = flist_blk;	/* point the block to itself */
flist_blk->FLB_A_BLINK_BY_SIZE = flist_blk;

return flist_blk;
} /* end of Unlink_flist_blk_by_size */


/******************************************************************************
**  Unlink_rlist_blk()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the returned-list.  The block must be unlinked
**	from both free-list list-heads (by size and by address).
**
**  FORMAL PARAMETERS:
**
**      rlist_blk	Returned-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**  
**      rlist_blk	Unlinked returned-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**                                   
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**                         
**	Find_rlist_best_fit()
**	Merge_rlist_block()
******************************************************************************/
static struct RLB *Unlink_rlist_blk( rlist_blk )
struct	RLB	*rlist_blk;
{
struct	RLB	*next_blk;
struct	RLB	*prev_blk;

/*
** Unlink from the returned-list by size list.
*/
Unlink_rlist_blk_by_size( rlist_blk );

/*                              
** Unlink from the returned-list by address list.
*/
Unlink_rlist_blk_by_adr( rlist_blk );

rlist_blk->RLB_W_FLAGS.RLB_V_RLIST_MEMBER = FALSE;

return rlist_blk;
} /* end of Unlink_rlist_blk */


/******************************************************************************
**  Unlink_rlist_blk_by_adr()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the returned-list by address.  
**	
**
**  FORMAL PARAMETERS:
**
**      rlist_blk	Returned-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      rlist_blk	Unlinked returned-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Unlink_rlist_blk()                      
******************************************************************************/
static struct RLB *Unlink_rlist_blk_by_adr( rlist_blk )
struct	RLB	*rlist_blk;
{
struct	RLB	*next_blk;
struct	RLB	*prev_blk;

/*             
** Unlink from the returned-list by address list.
*/
if ( (int) rlist_blk == (int) rlist_by_adr.LhdA_Flink && 
     (int) rlist_blk == (int) rlist_by_adr.LhdA_Blink )
    /*
    ** The block is the only element in the list, and is therefore
    ** both the first and the last block.
    */
    {
    rlist_by_adr.LhdA_Flink = (struct BHD *)&rlist_by_adr;
    rlist_by_adr.LhdA_Blink = (struct BHD *)&rlist_by_adr;
    }
else if ( (int) rlist_blk == (int) rlist_by_adr.LhdA_Flink )
    /*
    ** The block is the first block in the list.
    */
    {
    next_blk = rlist_blk->RLB_A_FLINK_BY_ADR;
    next_blk->RLB_A_BLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
    rlist_by_adr.LhdA_Flink = (struct BHD *) next_blk;
    }
else if ( (int) rlist_blk == (int) rlist_by_adr.LhdA_Blink )
    /*
    ** The block is the last block in the list.
    */
    {
    prev_blk = rlist_blk->RLB_A_BLINK_BY_ADR;
    prev_blk->RLB_A_FLINK_BY_ADR = (struct RLB *) &rlist_by_adr;
    rlist_by_adr.LhdA_Blink = (struct BHD *) prev_blk;
    }
else
    /*
    ** The block is somewhere between the end-points of the list.
    */
    {
    prev_blk = rlist_blk->RLB_A_BLINK_BY_ADR;
    next_blk = rlist_blk->RLB_A_FLINK_BY_ADR;
    prev_blk->RLB_A_FLINK_BY_ADR = next_blk;
    next_blk->RLB_A_BLINK_BY_ADR = prev_blk;
    }

--rlist_by_adr.LhdW_ListCnt;			/* decrement the list count */
rlist_blk->RLB_A_FLINK_BY_ADR = rlist_blk;	/* point the block to itself */
rlist_blk->RLB_A_BLINK_BY_ADR = rlist_blk;  

return rlist_blk;
} /* end of Unlink_rlist_blk_by_adr */


/******************************************************************************
**  Unlink_rlist_blk_by_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the returned-list by size.
**
**  FORMAL PARAMETERS:
**
**      rlist_blk	Returned-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**                                         
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      rlist_blk	Unlinked returned-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Unlink_rlist_blk()
******************************************************************************/
static struct RLB *Unlink_rlist_blk_by_size( rlist_blk )
struct	RLB	*rlist_blk;
{
struct	RLB	*next_blk;
struct	RLB	*prev_blk;

/*
** Unlink from the returned-list by size list.
**
**	Note that there are special case conditions if the block
**	to unlink is at either end of the list since the list-head
**	flink & blink structure does not correspond in position to the 
**	RLB flink & blink structure.
*/
if ( (int) rlist_blk == (int) rlist_by_size.LhdA_Flink && 
     (int) rlist_blk == (int) rlist_by_size.LhdA_Blink )
    /*
    ** The block is the only element in the list, and is therefore
    ** both the first and the last block.
    */
    {
    rlist_by_size.LhdA_Flink = (struct BHD *)&rlist_by_size;
    rlist_by_size.LhdA_Blink = (struct BHD *)&rlist_by_size;
    }
else if ( (int) rlist_blk == (int) rlist_by_size.LhdA_Flink )
    /*
    ** The block is the first block in the list.
    */
    {
    next_blk = rlist_blk->RLB_A_FLINK_BY_SIZE;
    next_blk->RLB_A_BLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
    rlist_by_size.LhdA_Flink = (struct BHD *) next_blk;
    }
else if ( (int) rlist_blk == (int) rlist_by_size.LhdA_Blink )
    /*
    ** The block is the last block in the list.
    */
    {
    prev_blk = rlist_blk->RLB_A_BLINK_BY_SIZE;
    prev_blk->RLB_A_FLINK_BY_SIZE = (struct RLB *) &rlist_by_size;
    rlist_by_size.LhdA_Blink = (struct BHD *) prev_blk;
    }
else
    /*
    ** The block is somewhere between the end-points of the list.
    */
    {
    prev_blk = rlist_blk->RLB_A_BLINK_BY_SIZE;
    next_blk = rlist_blk->RLB_A_FLINK_BY_SIZE;
    prev_blk->RLB_A_FLINK_BY_SIZE = next_blk;
    next_blk->RLB_A_BLINK_BY_SIZE = prev_blk;
    }

--rlist_by_size.LhdW_ListCnt;			/* decrement the list count */
rlist_blk->RLB_A_FLINK_BY_SIZE = rlist_blk;	/* point the block to itself */
rlist_blk->RLB_A_BLINK_BY_SIZE = rlist_blk;

return rlist_blk;
} /* end of Unlink_rlist_blk_by_size */


/******************************************************************************
**  Unlink_ulist_blk()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the user-block in-use list.  The block must be 
**	unlinked from both list-heads (by size and by address).
**
**  FORMAL PARAMETERS:
**
**      ulist_blk	User-block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      ulist_blk	Unlinked in-use-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**                                   
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
******************************************************************************/
static struct FLB *Unlink_ulist_blk( ulist_blk )
struct	FLB	*ulist_blk;
{

/*
** Make sure the ulists aren't messed up.
*/
Verify_ulist();

/*
** Unlink from the in-use list by size.
*/
Unlink_ulist_blk_by_size( ulist_blk );

/*
** Unlink from the in-use list by address.
*/
Unlink_ulist_blk_by_adr( ulist_blk );

/*
** Make sure the ulists didn't get messed up.
*/
Verify_ulist();

/*
** Clear list member flag                 
*/
ulist_blk->FLB_W_FLAGS.FLB_V_ULIST_MEMBER = FALSE;

return ulist_blk;
} /* end of Unlink_ulist_blk */


/******************************************************************************
**  Unlink_ulist_blk_by_adr()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the in-use list by address.  
**	
**
**  FORMAL PARAMETERS:
**
**      ulist_blk	in-use-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      ulist_blk	Unlinked in-use-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Unlink_ulist_blk()
******************************************************************************/
static struct FLB *Unlink_ulist_blk_by_adr( ulist_blk )
struct	FLB	*ulist_blk;
{
struct	FLB	*next_blk;
struct	FLB	*prev_blk;

/*
** Unlink from the free-list by address list.
*/
if ( (int) ulist_blk == (int) ulist_by_adr.LhdA_Flink && 
     (int) ulist_blk == (int) ulist_by_adr.LhdA_Blink )
    /*
    ** The block is the only element in the list, and is therefore
    ** both the first and the last block.  Set the list head to point
    ** to itself.
    */
    {
    ulist_by_adr.LhdA_Flink = (struct BHD *)&ulist_by_adr;
    ulist_by_adr.LhdA_Blink = (struct BHD *)&ulist_by_adr;
    }
else if ( (int) ulist_blk == (int) ulist_by_adr.LhdA_Flink )
    /*
    ** The block is the first block in the list.
    */
    {
    next_blk = ulist_blk->FLB_A_FLINK_BY_ADR;
    next_blk->FLB_A_BLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
    ulist_by_adr.LhdA_Flink = (struct BHD *) next_blk;
    }
else if ( (int) ulist_blk == (int) ulist_by_adr.LhdA_Blink )
    /*
    ** The block is the last block in the list.
    */
    {
    prev_blk = ulist_blk->FLB_A_BLINK_BY_ADR;
    prev_blk->FLB_A_FLINK_BY_ADR = (struct FLB *) &ulist_by_adr;
    ulist_by_adr.LhdA_Blink = (struct BHD *) prev_blk;
    }
else
    /*
    ** The block is somewhere between the end-points of the list.
    */
    {
    prev_blk = ulist_blk->FLB_A_BLINK_BY_ADR;
    next_blk = ulist_blk->FLB_A_FLINK_BY_ADR;
    prev_blk->FLB_A_FLINK_BY_ADR = next_blk;
    next_blk->FLB_A_BLINK_BY_ADR = prev_blk;
    }

--ulist_by_adr.LhdW_ListCnt;			/* decrement the list count */
ulist_blk->FLB_A_FLINK_BY_ADR = ulist_blk;	/* point the block to itself */
ulist_blk->FLB_A_BLINK_BY_ADR = ulist_blk;  

return ulist_blk;
} /* end of Unlink_ulist_blk_by_adr */


/******************************************************************************
**  Unlink_ulist_blk_by_size()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Unlink a block from the in-use-list by size.
**
**  FORMAL PARAMETERS:
**
**      ulist_blk	In-use-list block to unlink.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**       none
**                                         
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      ulist_blk	Unlinked in-use-list block.
**			Passed by reference.
**
**			Note: this is the same as the input argument.
**
**  SIGNAL CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Unlink_ulist_blk()
******************************************************************************/
static struct FLB *Unlink_ulist_blk_by_size( ulist_blk )
struct	FLB	*ulist_blk;
{
struct	FLB	*next_blk;
struct	FLB	*prev_blk;

/*
** Unlink from the free-list by size list.
**
**	Note that there are special case conditions if the block
**	to unlink is at either end of the list since the list-head
**	flink & blink structure does not correspond in position to the 
**	FLB flink & blink structure.
*/
if ( (int) ulist_blk == (int) ulist_by_size.LhdA_Flink && 
     (int) ulist_blk == (int) ulist_by_size.LhdA_Blink )
    /*
    ** The block is the only element in the list, and is therefore
    ** both the first and the last block.
    */
    {          
    ulist_by_size.LhdA_Flink = (struct BHD *)&ulist_by_size;
    ulist_by_size.LhdA_Blink = (struct BHD *)&ulist_by_size;
    }
else if ( (int) ulist_blk == (int) ulist_by_size.LhdA_Flink )
    /*
    ** The block is the first block in the list.
    */
    {
    next_blk = ulist_blk->FLB_A_FLINK_BY_SIZE;
    next_blk->FLB_A_BLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
    ulist_by_size.LhdA_Flink = (struct BHD *) next_blk;
    }
else if ( (int) ulist_blk == (int) ulist_by_size.LhdA_Blink )
    /*
    ** The block is the last block in the list.
    */
    {
    prev_blk = ulist_blk->FLB_A_BLINK_BY_SIZE;
    prev_blk->FLB_A_FLINK_BY_SIZE = (struct FLB *) &ulist_by_size;
    ulist_by_size.LhdA_Blink = (struct BHD *) prev_blk;
    }
else
    /*
    ** The block is somewhere between the end-points of the list.
    */
    {
    prev_blk = ulist_blk->FLB_A_BLINK_BY_SIZE;
    next_blk = ulist_blk->FLB_A_FLINK_BY_SIZE;
    prev_blk->FLB_A_FLINK_BY_SIZE = next_blk;
    next_blk->FLB_A_BLINK_BY_SIZE = prev_blk;
    }

--ulist_by_size.LhdW_ListCnt;			/* decrement the list count */
ulist_blk->FLB_A_FLINK_BY_SIZE = ulist_blk;	/* point the block to itself */
ulist_blk->FLB_A_BLINK_BY_SIZE = ulist_blk;

return ulist_blk;
} /* end of Unlink_ulist_blk_by_size */


/******************************************************************************
**  Verify_block_type()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Verify that a block is of the type passed in.
**	Valid types are:
**
**		Free-list blocks	(IMG_K_TYPE_FLB)
**		User blocks		(IMG_K_TYPE_USB)
**		Returned-list blocks	(IMG_K_TYPE_RLB)
**
**	The address of the block is checked against the range of
**	allocated pages, and is checked against on of the above
**	valid block type codes.
**
**	If the address passed in is completely invalid (i.e., it is not in
**	allocated P0 space, such as might happen if the user tries to
**	free the same block twice), an access violation may occur.
**
**  FORMAL PARAMETERS:
**
**      block		Block to validate.  Passed by reference.
**	type		Type code to compare block type against.
**			Passed by value.
**
**  IMPLICIT INPUTS:
**
**       none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**	ImgX_DYNBLKTYP		Dynamic block type-code mismatch on verify
**      ImgX_INVDYNADR		Invalid dynamic memory block address
**	ImgX_NOTUSRMEM		Not a user memory block
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Free_memory()
******************************************************************************/
static void Verify_block_type( block, type )
struct	FLB	*block;
int		 type;
{
int		 match = FALSE;
struct	FLB	*ulist_blk;

/*
** Search the user-block in-use list (by address) to see if the block
** address being returned by the user (directly or indirectly via _ImgFree)
** is actually valid - i.e., is the block really user-memory?
*/
if ( ulist_by_adr.LhdW_ListCnt == 0 )
    ChfStop( 1,  ImgX_NOTUSRMEM );
else
    {

/*
**  This is commented out because it takes too long to run through
**  the whole user list to see if the block passed in is on the user list.
**
**    ulist_blk = (struct FLB *) ulist_by_adr.LhdA_Flink;
**    while ( ulist_blk != 0 )
**	{
**	if ( block == ulist_blk )
**	    {
**	    match = TRUE;
**	    break;
**	    }
**	ulist_blk = (struct FLB *) (NEXT_ULIST_BLK_BY_ADR_( ulist_blk ));
**
**	}
**    if ( !match )
**	ChfStop( 1,  ImgX_NOTUSRMEM );
**
** Instead if the above, just see if the block type is a user block
** and that it matches the passed in type (which, as it turns out, can
** only be a user block).
*/
if ( type == IMG_K_TYPE_USB &&
     block->FLB_B_TYPE != type )
    ChfStop( 1, ImgX_NOTUSRMEM );
    }

return;
} /* end of Verify_block_type */


/******************************************************************************
**  Verify_flb()
**
**  FUNCTIONAL DESCRIPTION:
**
**	Verify the contents of a free-list block.
**
**  FORMAL PARAMETERS:
**
**	flb		Free-list block to verify.
**			Passed by reference.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	ImgX_INVFLBLEN	    Invalid Free-list block length.  Was zero.
**
**  SIDE EFFECTS:
**
**	none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
******************************************************************************/
static void Verify_flb( flb )
struct	FLB	*flb;
{

if ( flb->FLB_L_LENGTH == 0 )
    ChfStop( 1,  ImgX_INVFLBLEN );

return;
} /* end of Verify_flb */


/******************************************************************************
**  Verify_flist()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Verify that the free-lists (by address and by size) are consistant.
**
**  FORMAL PARAMETERS:
**
**      none.
**
**  IMPLICIT INPUTS:
**
**      flist_by_adr	Free-list by address list-head.
**	flist_by_size	Free-list by size list-head.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      ImgX_INCLSTCNT	    Inconsistent list counts.
**	ImgX_INVFLAORD	    Invalid free list by address order.
**	ImgX_INVFLSORD	    Invalid free list by size order.
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_flist()
******************************************************************************/
static void Verify_flist()
{
int 	 	 count;
int	 	 index		= 2;
int		 force_return	= 1;
struct	FLB	*cur_blk	= 0;
struct	FLB	*prev_blk	= 0;

/*
** Are the list counts the same?
*/
if ( flist_by_adr.LhdW_ListCnt != flist_by_size.LhdW_ListCnt )
    ChfStop( 1,  ImgX_INCLSTCNT );		/* inconsistent list counts */

count = flist_by_adr.LhdW_ListCnt;

/*
** TEMPORARY HACK:
**
** Force a return at this point for now so that the rest of this routine
** is not exercized (because it's too costly ... i.e., it takes too much time).
*/
if ( force_return )
    return;

/*
** Verify the ordering of the free-list by address
*/
if ( count == 1 )
    {
    if ( flist_by_adr.LhdA_Flink != flist_by_adr.LhdA_Blink )
	ChfStop( 1,  ImgX_INVFLAORD );	/* not the best error code */
    if ( flist_by_adr.LhdA_Flink == (struct BHD *)&flist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );
    if ( flist_by_adr.LhdA_Blink == (struct BHD *)&flist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );

    cur_blk = (struct FLB *) flist_by_adr.LhdA_Flink;
    if ( cur_blk->FLB_A_FLINK_BY_ADR != cur_blk->FLB_A_BLINK_BY_ADR )
	ChfStop( 1,  ImgX_INVFLAORD );
    if ( (int) cur_blk->FLB_A_FLINK_BY_ADR != (int) &flist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );
    if ( (int) cur_blk->FLB_A_BLINK_BY_ADR != (int) &flist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );
    }
else if ( count >= 2 )
    {
    prev_blk = (struct FLB *) flist_by_adr.LhdA_Flink;
    cur_blk = (struct FLB *) (NEXT_FLIST_BLK_BY_ADR( prev_blk ));
    
    while ( cur_blk != 0 )
    	{
    	if ( cur_blk < prev_blk )
            /* invalid flist address order */
            ChfStop( 1,  ImgX_INVFLAORD );
    	prev_blk = cur_blk;
    	cur_blk = (struct FLB *) (NEXT_FLIST_BLK_BY_ADR( prev_blk ));
    	if ( cur_blk != 0 ) ++index;
    	}
    if ( index != count )
	ChfStop( 1,  ImgX_INVFLAORD );
    }

/*
** Verify the ordering of the free-list by size
*/
index = 2;
if ( count == 1 )
    {
    if ( flist_by_size.LhdA_Flink != flist_by_size.LhdA_Blink )
	ChfStop( 1,  ImgX_INVFLSORD );	/* not the best error code */
    if ( flist_by_size.LhdA_Flink == (struct BHD *)&flist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );
    if ( flist_by_size.LhdA_Blink == (struct BHD *)&flist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );

    cur_blk = (struct FLB *) flist_by_size.LhdA_Flink;
    if ( cur_blk->FLB_A_FLINK_BY_SIZE != cur_blk->FLB_A_BLINK_BY_SIZE )
	ChfStop( 1,  ImgX_INVFLSORD );
    if ( (int) cur_blk->FLB_A_FLINK_BY_SIZE != (int) &flist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );
    if ( (int) cur_blk->FLB_A_BLINK_BY_SIZE != (int) &flist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );
    }
else if ( count >= 2 )
    {
    prev_blk = (struct FLB *) flist_by_size.LhdA_Flink;
    cur_blk = (struct FLB *) (NEXT_FLIST_BLK_BY_SIZE_( prev_blk ));
    
    while ( cur_blk != 0 )
    	{
    	if ( cur_blk->FLB_L_LENGTH < prev_blk->FLB_L_LENGTH )
            /* invalid flist address order */
            ChfStop( 1,  ImgX_INVFLSORD );
    	prev_blk = cur_blk;
    	cur_blk = (struct FLB *) (NEXT_FLIST_BLK_BY_SIZE_( prev_blk ));
    	if ( cur_blk != 0 ) ++index;
    	}
    if ( index != count )
	ChfStop( 1,  ImgX_INVFLSORD );
    }

return;
} /* end of Verify_flist */


/******************************************************************************
**  Verify_rlist()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Verify that the returned-lists (by address and by size) are consistant.
**
**  FORMAL PARAMETERS:
**
**      none.
**
**  IMPLICIT INPUTS:
**
**      rlist_by_adr	Returned-list by address list-head.
**	rlist_by_size	Returned-list by size list-head.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      ImgX_INCLSTCNT		Inconsitent list counts.
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_rlist()
******************************************************************************/
static void Verify_rlist()
{

/*
** Are the list counts the same?
*/
if ( rlist_by_adr.LhdW_ListCnt != rlist_by_size.LhdW_ListCnt )
    ChfStop( 1,  ImgX_INCLSTCNT );		/* inconsistent list counts */

return;
} /* end of Verify_rlist */


/******************************************************************************
**  Verify_ulist()
**
**  FUNCTIONAL DESCRIPTION:
**
**      Verify that the user-lists (by address and by size) are consistant.
**
**  FORMAL PARAMETERS:
**
**      none.
**
**  IMPLICIT INPUTS:
**
**      ulist_by_adr	User-list by address list-head.
**	ulist_by_size	User-list by size list-head.
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void (none)
**
**  SIGNAL CODES:
**
**      ImgX_INCLSTCNT	    Inconsistent list counts.
**	ImgX_INVFLAORD	    Invalid free list by address order.
**	ImgX_INVFLSORD	    Invalid free list by size order.
**
**  SIDE EFFECTS:
**
**      none
**
**  CALLED BY:
**
**	Add_blk_to_ulist()
******************************************************************************/
static void Verify_ulist()
{
int 	 	 count;
int	 	 index		= 2;
int		 force_return	= 1;
struct	FLB	*cur_blk	= 0;
struct	FLB	*prev_blk	= 0;

/*
** Are the list counts the same?
*/
if ( ulist_by_adr.LhdW_ListCnt != ulist_by_size.LhdW_ListCnt )
    ChfStop( 1,  ImgX_INCLSTCNT );		/* inconsistent list counts */

count = ulist_by_adr.LhdW_ListCnt;

/*
** TEMPORARY HACK:
**
** Force a return at this point for now so that the rest of this routine
** is not exercized (because it's too costly ... i.e., it takes too much time).
*/
if ( force_return )
    return;

/*
** Verify the ordering of the free-list by address
*/
if ( count == 1 )
    {
    if ( ulist_by_adr.LhdA_Flink != ulist_by_adr.LhdA_Blink )
	ChfStop( 1,  ImgX_INVFLAORD );	/* not the best error code */
    if ( ulist_by_adr.LhdA_Flink == (struct BHD *)&ulist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );
    if ( ulist_by_adr.LhdA_Blink == (struct BHD *)&ulist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );

    cur_blk = (struct FLB *) ulist_by_adr.LhdA_Flink;
    if ( cur_blk->FLB_A_FLINK_BY_ADR != cur_blk->FLB_A_BLINK_BY_ADR )
	ChfStop( 1,  ImgX_INVFLAORD );
    if ( (int) cur_blk->FLB_A_FLINK_BY_ADR != (int) &ulist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );
    if ( (int) cur_blk->FLB_A_BLINK_BY_ADR != (int) &ulist_by_adr )
	ChfStop( 1,  ImgX_INVFLAORD );
    }
else if ( count >= 2 )
    {
    prev_blk = (struct FLB *) ulist_by_adr.LhdA_Flink;
    cur_blk = (struct FLB *) (NEXT_ULIST_BLK_BY_ADR_( prev_blk ));
    
    while ( cur_blk != 0 )
    	{
    	if ( cur_blk < prev_blk )
            /* invalid ulist address order */
            ChfStop( 1,  ImgX_INVFLAORD );
	if ( prev_blk->FLB_A_FLINK_BY_ADR != cur_blk )
	    ChfStop( 1,  ImgX_INVFLAORD );
	if ( cur_blk->FLB_A_BLINK_BY_ADR != prev_blk )
	    ChfStop( 1,  ImgX_INVFLAORD );
    	prev_blk = cur_blk;
    	cur_blk = (struct FLB *) (NEXT_ULIST_BLK_BY_ADR_( prev_blk ));
    	if ( cur_blk != 0 ) ++index;
    	}
    if ( index != count )
	ChfStop( 1,  ImgX_INVFLAORD );
    }

/*
** Verify the ordering of the free-list by size
*/
index = 2;
if ( count == 1 )
    {
    if ( ulist_by_size.LhdA_Flink != ulist_by_size.LhdA_Blink )
	ChfStop( 1,  ImgX_INVFLSORD );	/* not the best error code */
    if ( ulist_by_size.LhdA_Flink == (struct BHD *)&ulist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );
    if ( ulist_by_size.LhdA_Blink == (struct BHD *)&ulist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );

    cur_blk = (struct FLB *) ulist_by_size.LhdA_Flink;
    if ( cur_blk->FLB_A_FLINK_BY_SIZE != cur_blk->FLB_A_BLINK_BY_SIZE )
	ChfStop( 1,  ImgX_INVFLSORD );
    if ( (int) cur_blk->FLB_A_FLINK_BY_SIZE != (int) &ulist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );
    if ( (int) cur_blk->FLB_A_BLINK_BY_SIZE != (int) &ulist_by_size )
	ChfStop( 1,  ImgX_INVFLSORD );
    }
else if ( count >= 2 )
    {
    prev_blk = (struct FLB *) ulist_by_size.LhdA_Flink;
    cur_blk = (struct FLB *) (NEXT_ULIST_BLK_BY_SIZE_( prev_blk ));
    
    while ( cur_blk != 0 )
    	{
    	if ( cur_blk->FLB_L_LENGTH < prev_blk->FLB_L_LENGTH )
            /* invalid ulist address order */
            ChfStop( 1,  ImgX_INVFLSORD );
	if ( prev_blk->FLB_A_FLINK_BY_SIZE != cur_blk )
	    ChfStop( 1,  ImgX_INVFLSORD );
	if ( cur_blk->FLB_A_BLINK_BY_SIZE != prev_blk )
	    ChfStop( 1,  ImgX_INVFLSORD );
    	prev_blk = cur_blk;
    	cur_blk = (struct FLB *) (NEXT_ULIST_BLK_BY_SIZE_( prev_blk ));
    	if ( cur_blk != 0 ) ++index;
    	}
    if ( index != count )
	ChfStop( 1,  ImgX_INVFLSORD );
    }

return;
} /* end of Verify_ulist */


/******************************************************************************
**  Select_memmgt
**
**  FUNCTIONAL DESCRIPTION:
**
**	Under VMS, this routine will look for the process logical
**	IMG$MEMORY_MGT to determine which type of memory mgt scheme
**	to use to initialize the memory mgt routine pointers with.
**
**      On non-VMS operating systems, this routine will look for the
**      environment variable IMG_MEMORY_MGT to determine which type of
**      memory mgt scheme to use to initialize the memory mgt routine pointers.
**
**	Valid values of this logical are:  "CDA", "ISL", & "LIB"
**
**  FORMAL PARAMETERS:
**
**	none.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	scheme		IMG_K_MEMMGT_ISL_DEFAULTED
**			    Signifies logical was not defined, or given
**			    a valid value.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
static long Select_memmgt()
{
#if defined(__VMS) || defined(VMS)
char	*lognam		= "IMG$MEMORY_MGT";
#else
char	*lognam		= "IMG_MEMORY_MGT";
#endif
char	*scheme_cda1	= "CDA1";
char	*scheme_cda2	= "CDA2";
char	*scheme_isl	= "ISL";
char	*scheme_lib	= "LIB";
char	*scheme_string;

long	length;
long	scheme;

scheme_string = (char *) getenv(lognam);

if (scheme_string == 0)
  scheme = IMG_K_MEMMGT_ISL_DEFAULTED;
else
  {
    length = strlen(scheme_string);
    if (strncmp(scheme_isl,scheme_string, length) == 0)
      scheme = IMG_K_MEMMGT_ISL;
    else
      if (strncmp(scheme_cda1,scheme_string, length) == 0)
	scheme = IMG_K_MEMMGT_CDA1;
      else 
	if (strncmp(scheme_cda2,scheme_string, length) == 0)
	  scheme = IMG_K_MEMMGT_CDA2;
	else
	  if (strncmp(scheme_lib,scheme_string, length) == 0)
	    scheme = IMG_K_MEMMGT_LIB;	
	  else
	    scheme = IMG_K_MEMMGT_ISL_DEFAULTED;
  }

return scheme;
} /* end of Select_memmgt */


#ifdef VMS
/******************************************************************************
**  _ImgGetAutoTuneSize
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function is called by _ImgCalloc, _ImgMalloc, and _ImgRealloc
**	to remap the requested size to an auto-tuned size.
**
**  FORMAL PARAMETERS:
**
**	user_size   Original request size in bytes to allocate.
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	none
**
**  FUNCTION VALUE:
**
**	working_size	Actual size in bytes to allocate after
**			the user_size was auto-tune remapped.
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	If the working_size is a value different than what the 
**	underlying VMS RTL memory mgt scheme would have used, more
**	memory will be allocated, but we take the chance that the
**	size used will more likely be reused by a later request size,
**	and thus keep down calls to system services for new virtual
**	memory.
**
******************************************************************************/
static long _ImgGetAutoTuneSize( user_size )
long	user_size;
{
long	 effective_index;
long	 index;
long	 status;
long	 working_size	    = user_size;
FILE	*fp1		    = 0;
FILE	*fp2		    = ImgL_AutoTuneOutputFile;
FILE	*output		    = ImgL_AutoTuneOutputFile;

CVT_SIZE_TO_INDEX_( working_size, index );
if ( ImgL_LoMemIndex <= index && index <= ImgL_HiMemIndex )
    {
    working_size = ImgR_MemRoundupTable[ index ] - OVERHEAD;
    if ( ImgL_AutoTuneDebug )
	{
	if ( ImgL_AutoTuneDebugShowTrace )
	    fprintf( output,
			"requested size: %d, index: %3d, rounded size: %d\n",
			user_size, index, working_size );

	if ( ImgL_AutoTuneDebugShowHistogram )
	    {
	    ++ImgA_ReqSizes[ index ];
	    CVT_SIZE_TO_INDEX_( working_size, effective_index );
	    ++ImgA_WrkSizes[ effective_index ];
	    }
	}
    }

/*
** Before returning, check to see if we have to print the
** auto-tune histograms (based on a trigger file).
*/
if ( ImgL_LookForHistFile )
    {
    /*
    ** Look for the trigger file
    */
    fp1 = fopen( ImgA_AutoTuneHistTriggerFile, "r" );
    
    /*
    ** If the trigger file was found:
    **
    **	    Look for an output filename on the first line
    **		- use it if found
    **		- if not found, use default output
    **
    **	    Call ImgShowAutoTuneHistogram
    **
    **	    Close the trigger file and delete it.
    */
    if ( fp1 != 0 )
	{
	memset( ImgA_AutoTuneHistOutputFile, 0,
		    sizeof( ImgA_AutoTuneHistOutputFile ) );
	status = fgets( ImgA_AutoTuneHistOutputFile, 
			sizeof( ImgA_AutoTuneHistOutputFile ),
			fp1 );
	/*
	** If something was on the first time, assume it's a filename
	** and attempt to open a file of that name ...
	*/
	if ( status != 0 )
	    {
	    long    str_len;
	    /*
	    ** Turn the end-of-line character into a zero before
	    ** using the string as a filename, and then attempt
	    ** to open the output file.
	    */
	    str_len = strlen( ImgA_AutoTuneHistOutputFile );
	    ImgA_AutoTuneHistOutputFile[ str_len - 1 ] = 0;
	    fp2 = fopen( ImgA_AutoTuneHistOutputFile, "a" );
	    if ( fp2 == 0 )
		fp2 = ImgL_AutoTuneOutputFile;
	    }

	ImgL_AutoTuneHistOutputFile = fp2;

	ImgShowAutoTuneHistogram();

	if ( fp2 != ImgL_AutoTuneOutputFile )
	    fclose( fp2 );

	fclose( fp1 );
	delete( ImgA_AutoTuneHistTriggerFile );
	}

    }

return working_size;
} /* end of _ImgGetAutoTuneSize */


/******************************************************************************
**  _ImgInitializeAutoTune
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function initializes the auto-tuning scheme allocation-size
**	remap table.  It reads the logicals described below as
**	implicit inputs and sets up internal state flags and the remap
**	table which will round request sizes within the specified range
**	up to the nearest requantized sub-range point.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	This function reads the following logical names with getenv():
**
**	    IMG_AUTO_TUNE_ON
**	    IMG_DEBUG_AUTO_TUNE
**	    IMG_DEBUG_SHOW_TABLE
**	    IMG_DEBUG_SHOW_HISTOGRAM
**	    IMG_DEBUG_SHOW_TRACE
**	    IMG_LO_MEM_SIZE
**	    IMG_HI_MEM_SIZE
**	    IMG_TUNING_FACTOR
**	    IMG_TUNING_BIAS
**
**  IMPLICIT OUTPUTS:
**
**	If IMG_DEBUG_AUTO_TUNE is defined, basic state information is
**	printed to the user's terminal.  If IMG_DEBUG_SHOW_TABLE is
**	printed, the contents of the remap table are printed to the
**	user's terminal.
**
**	NOTE that the IMG_DEBUG_SHOW_HISTOGRAM logical does not directly
**	cause info the be printed by this function.  The IMG_SHOW_TRACE
**	logical also doesn't cause info to be printed by this function.
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	When IMG_AUTO_TUNE_ON is defined, this function effects how much
**	memory is actually allocated for a given request for dynamic
**	memory.  How much is allocated depends on whether the default
**	parameter values are used, or whether users specify values for
**	IMG_LO_MEM_SIZE, IMG_HI_MEM_SIZE, IMG_TUNING_FACTOR, and
**	IMG_TUNING_BIAS.
**
******************************************************************************/
static void _ImgInitializeAutoTune()
{
char	*debug_str		    = 0;
char	*debug_hist_trigger_file    = 0;
char	*debug_output_file	    = 0;
char	*debug_show_hist_str	    = 0;
char	*debug_show_table_str	    = 0;
char	*debug_show_trace_str	    = 0;
char	*img_auto_tune_on_str	    = 0;
char	*hi_size_limit_str	    = 0;
char	*lo_size_limit_str	    = 0;
char	*tuning_bias_str	    = 0;
char	*tuning_factor_str	    = 0;

double	tuning_bias		    = 1.0;

FILE	*fp;
FILE	*output;

long	index_range;
long	hi_index_range;
long	hi_size_limit		    = 0;
long	hi_step_cnt;
long	hi_step_size;
long	hi_user_index;
long	hi_work_index;
long	lo_index_range;
long	lo_size_limit		    = 0;
long	lo_step_cnt;
long	lo_step_size;
long	lo_user_index;
long	lo_work_index;
long	step_count;
long	table_index;
long	tuning_factor		    = 0;
long	tuning_point_index;
long	tuning_step_size;

static long MemRoundupTable[ 256 ]  = {
     0,0,0,0,0,0,0,0,0,0        /*   0 -   9		    */
    ,0,0,0,0,0,0,0,0,0,0        /*  10 -  19		    */
    ,0,0,0,0,0,0,0,0,0,0        /*  20 -  29		    */
    ,0,0,0,0,0,0,0,0,0,0	/*  30 -  39		    */
    ,0,0,0,0,0,0,0,0,0,0        /*  40 -  49		    */
    ,0,0,0,0,0,0,0,0,0,0	/*  50 -  59		    */
    ,0,0,0,0,0,0,0,0,0,0	/*  60 -  69		    */
    ,0,0,0,0,0,0,0,0,0,0	/*  70 -  79		    */
    ,0,0,0,0,0,0,0,0,0,0	/*  80 -  89		    */
    ,0,0,0,0,0,0,0,0,0,0	/*  90 -  99		    */
    ,0,0,0,0,0,0,0,0,0,0	/* 100 - 109		    */
    ,0                  	/* 110      		    */
    ,8192			/* 111  blocks:  16	    */
    ,9216			/* 112  blocks:  18	    */
    ,10240			/* 113  blocks:  20	    */
    ,11264			/* 114  blocks:  22	    */
    ,12288			/* 115  blocks:  24	    */
    ,13312			/* 116  blocks:  26	    */
    ,14336			/* 117  blocks:  28	    */
    ,15360			/* 118  blocks:  30	    */
    ,16384			/* 119  blocks:  32	    */
    ,18432			/* 120  blocks:  36	    */
    ,20480			/* 121  blocks:  40	    */
    ,22528			/* 122  blocks:  44	    */
    ,24576			/* 123  blocks:  48	    */
    ,26624			/* 124  blocks:  52	    */
    ,28672			/* 125  blocks:  56	    */
    ,30720			/* 126  blocks:  60	    */
    ,32768			/* 127  blocks:  64	    */
    ,36864			/* 128  blocks:  72	    */
    ,40960			/* 129  blocks:  80	    */
    ,45056			/* 130  blocks:  88	    */
    ,49152			/* 131  blocks:  96	    */
    ,53248			/* 132  blocks:  104	    */
    ,57344			/* 133  blocks:  112	    */
    ,61440			/* 134  blocks:  120	    */
    ,65536			/* 135  blocks:  128	    */
    ,73728			/* 136  blocks:  144	    */
    ,81920			/* 137  blocks:  160	    */
    ,90112			/* 138  blocks:  176	    */
    ,98304			/* 139  blocks:  192	    */
    ,106496			/* 140  blocks:  208	    */
    ,114688			/* 141  blocks:  224	    */
    ,122880			/* 142  blocks:  240	    */
    ,131072			/* 143  blocks:  256	    */
    ,147456			/* 144  blocks:  288	    */
    ,163840			/* 145  blocks:  320	    */
    ,180224			/* 146  blocks:  352	    */
    ,196608			/* 147  blocks:  384	    */
    ,212992			/* 148  blocks:  416	    */
    ,229376			/* 149  blocks:  448	    */
    ,245760			/* 150  blocks:  480	    */
    ,262144			/* 151  blocks:  512	    */
    ,294912			/* 152  blocks:  576	    */
    ,327680			/* 153  blocks:  640	    */
    ,360448			/* 154  blocks:  704	    */
    ,393216			/* 155  blocks:  768	    */
    ,425984			/* 156  blocks:  832	    */
    ,458752			/* 157  blocks:  896	    */
    ,491520			/* 158  blocks:  960	    */
    ,524288			/* 159  blocks:  1024	    */
    ,589824			/* 160  blocks:  1152	    */
    ,655360			/* 161  blocks:  1280	    */
    ,720896			/* 162  blocks:  1408	    */
    ,786432			/* 163  blocks:  1536	    */
    ,851968			/* 164  blocks:  1664	    */
    ,917504			/* 165  blocks:  1792	    */
    ,983040			/* 166  blocks:  1920	    */
    ,1048576			/* 167  blocks:  2048	    */
    ,1179648			/* 168  blocks:  2304	    */
    ,1310720			/* 169  blocks:  2560	    */
    ,1441792			/* 170  blocks:  2816	    */
    ,1572864			/* 171  blocks:  3072	    */
    ,1703936			/* 172  blocks:  3328	    */
    ,1835008			/* 173  blocks:  3584	    */
    ,1966080			/* 174  blocks:  3840	    */
    ,2097152			/* 175  blocks:  4096	    */
    ,2359296			/* 176  blocks:  4608	    */
    ,2621440			/* 177  blocks:  5120	    */
    ,2883584			/* 178  blocks:  5632	    */
    ,3145728			/* 179  blocks:  6144	    */
    ,3407872			/* 180  blocks:  6656	    */
    ,3670016			/* 181  blocks:  7168	    */
    ,3932160			/* 182  blocks:  7680	    */
    ,4194304			/* 183  blocks:  8192	    */
    ,4718592			/* 184  blocks:  9216	    */
    ,5242880			/* 185  blocks:  10240	    */
    ,5767168			/* 186  blocks:  11264	    */
    ,6291456			/* 187  blocks:  12288	    */
    ,6815744			/* 188  blocks:  13312	    */
    ,7340032			/* 189  blocks:  14336	    */
    ,7864320			/* 190  blocks:  15360	    */
    ,8388608			/* 191  blocks:  16384	    */
    ,9437184			/* 192  blocks:  18432	    */
    ,10485760			/* 193  blocks:  20480	    */
    ,11534336			/* 194  blocks:  22528	    */
    ,12582912			/* 195  blocks:  24576	    */
    ,13631488			/* 196  blocks:  26624	    */
    ,14680064			/* 197  blocks:  28672	    */
    ,15728640			/* 198  blocks:  30720	    */
    ,16777216			/* 199  blocks:  32768	    */
    ,18874368			/* 200  blocks:  36864	    */
    ,20971520			/* 201  blocks:  40960	    */
    ,23068672			/* 202  blocks:  45056	    */
    ,25165824			/* 203  blocks:  49152	    */
    ,27262976			/* 204  blocks:  53248	    */
    ,29360128			/* 205  blocks:  57344	    */
    ,31457280			/* 206  blocks:  61440	    */
    ,33554432			/* 207  blocks:  65536	    */
    ,37748736			/* 208  blocks:  73728	    */
    ,41943040			/* 209  blocks:  81920	    */
    ,46137344			/* 210  blocks:  90112	    */
    ,50331648			/* 211  blocks:  98304	    */
    ,54525952			/* 212  blocks:  106496	    */
    ,58720256			/* 213  blocks:  114688	    */
    ,62914560			/* 214  blocks:  122880	    */
    ,67108864			/* 215  blocks:  131072	    */
    ,75497472			/* 216  blocks:  147456	    */
    ,83886080			/* 217  blocks:  163840	    */
    ,92274688			/* 218  blocks:  180224	    */
    ,100663296			/* 219  blocks:  196608	    */
    ,109051904			/* 220  blocks:  212992	    */
    ,117440512			/* 221  blocks:  229376	    */
    ,125829120			/* 222  blocks:  245760	    */
    ,134217728			/* 223  blocks:  262144	    */
    ,150994944			/* 224  blocks:  294912	    */
    ,167772160			/* 225  blocks:  327680	    */
    ,184549376			/* 226  blocks:  360448	    */
    ,201326592			/* 227  blocks:  393216	    */
    ,218103808			/* 228  blocks:  425984	    */
    ,234881024			/* 229  blocks:  458752	    */
    ,251658240			/* 230  blocks:  491520	    */
    ,268435456			/* 231  blocks:  524288	    */
    ,301989888			/* 232  blocks:  589824	    */
    ,335544320			/* 233  blocks:  655360	    */
    ,369098752			/* 234  blocks:  720896	    */
    ,402653184			/* 235  blocks:  786432	    */
    ,436207616			/* 236  blocks:  851968	    */
    ,469762048			/* 237  blocks:  917504	    */
    ,503316480			/* 238  blocks:  983040	    */
    ,536870912			/* 239  blocks:  1048576    */
    ,603979776			/* 240  blocks:  1179648    */
    ,671088640			/* 241  blocks:  1310720    */
    ,738197504			/* 242  blocks:  1441792    */
    ,805306368			/* 243  blocks:  1572864    */
    ,872415232			/* 244  blocks:  1703936    */
    ,939524096			/* 245  blocks:  1835008    */
    ,1006632960			/* 246  blocks:  1966080    */
    ,1073741824			/* 247  blocks:  2097152    */
    ,1207959552			/* 248  blocks:  2359296    */
    ,1342177280			/* 249  blocks:  2621440    */
    ,1476395008			/* 250  blocks:  2883584    */
    ,1610612736			/* 251  blocks:  3145728    */
    ,1744830464			/* 252  blocks:  3407872    */
    ,1879048192			/* 253  blocks:  3670016    */
    ,2013265920			/* 254  blocks:  3932160    */
    ,2147483648			/* 255  blocks:  4194304    */
    }; 	/* end init of MemRoundupTable */

/*
** Mark AutoTune as initialized
*/
ImgL_AutoTuneInitialized = 1;

img_auto_tune_on_str	= getenv( "IMG_AUTO_TUNE_ON" );
if ( img_auto_tune_on_str != 0 )
    {
    ImgL_AutoTune = 1;
    }
else
    /*
    ** Auto tuning is off.
    */
    return;

/*
** Store address of routine static memory roundup table in
** module global pointer variable.  This also indicates that
** memory roundup has been initialized.
*/
ImgR_MemRoundupTable = (long *)&MemRoundupTable;

/*
** Initialize free-list size histograms
*/
memset( ImgA_ReqSizes, 0, sizeof( ImgA_ReqSizes ) );
memset( ImgA_WrkSizes, 0, sizeof( ImgA_WrkSizes ) );

/*
** Get environment variables that affect tuning.
*/
debug_str		= getenv( "IMG_DEBUG_AUTO_TUNE" );
if ( debug_str )
    {
    ImgL_AutoTuneDebug = 1;
    debug_show_table_str = getenv( "IMG_DEBUG_SHOW_TABLE" );
    if ( debug_show_table_str )
	{
	ImgL_AutoTuneDebugShowTable = 1;
	}

    debug_show_hist_str = getenv( "IMG_DEBUG_SHOW_HISTOGRAM" );
    if ( debug_show_hist_str )
	{
	ImgL_AutoTuneDebugShowHistogram = 1;
	}

    ImgL_AutoTuneOutputFile = stdout;
    output = ImgL_AutoTuneOutputFile;
    debug_output_file = getenv( "IMG_DEBUG_OUTPUT_FILE" );
    if ( debug_output_file )
	{
	fp = fopen( debug_output_file, "a" );
	if ( fp )
	    {
	    output = fp;
	    ImgL_AutoTuneOutputFile = fp;
	    }
	}

    debug_hist_trigger_file = getenv( "IMG_DEBUG_HISTOGRAM_FILE" );
    if ( debug_hist_trigger_file )
	{
	ImgA_AutoTuneHistTriggerFile = debug_hist_trigger_file;
	ImgL_LookForHistFile = 1;
	}

    debug_show_trace_str = getenv( "IMG_DEBUG_SHOW_TRACE" );
    if ( debug_show_trace_str )
	{
	ImgL_AutoTuneDebugShowTrace = 1;
	}
    }

lo_size_limit_str	= getenv( "IMG_LO_MEM_SIZE" );
hi_size_limit_str	= getenv( "IMG_HI_MEM_SIZE" );
tuning_factor_str	= getenv( "IMG_TUNING_FACTOR" );
tuning_bias_str		= getenv( "IMG_TUNING_BIAS" );

/*
** Convert environment variables (if supplied) into working integers.
** NOTE that for the low limit in particular, if the size is really
** small, we're going to bump it up to 16 just to make sure that we
** get a good index value.  This is just to ensure that the test to
** see if the specified value is less than the absolute low works.
*/
if ( lo_size_limit_str != 0 )
    lo_size_limit = atoi( lo_size_limit_str );
if ( lo_size_limit < 16 )
    lo_size_limit = 16;

if ( hi_size_limit_str != 0 )
    hi_size_limit = atoi( hi_size_limit_str );
if ( hi_size_limit < lo_size_limit )
    hi_size_limit = lo_size_limit + 1;

if ( tuning_factor_str != 0 )
    tuning_factor = atoi( tuning_factor_str );
if ( tuning_factor <= 0 )
    tuning_factor = DEFAULT_MEM_TUNING_FACTOR;

if ( tuning_bias_str != 0 )
    {
    tuning_bias = atof( tuning_bias_str );
    if ( tuning_bias < 0.0 || tuning_bias > 1.0 )
	tuning_bias = 0.5;
    }
else
    tuning_bias = 0.5;

CVT_SIZE_TO_INDEX_( lo_size_limit, lo_user_index );
CVT_SIZE_TO_INDEX_( hi_size_limit, hi_user_index );

if ( lo_user_index < ABSOLUTE_LOWEST_MEM_INDEX )
    lo_work_index = DEFAULT_LO_MEM_INDEX;
else if ( lo_user_index > ABSOLUTE_MAX_MEM_INDEX )
    lo_work_index = ABSOLUTE_MAX_MEM_INDEX;
else
    lo_work_index = lo_user_index;

if ( hi_user_index < lo_work_index )
    hi_work_index = DEFAULT_HI_MEM_INDEX;
else if ( hi_user_index > ABSOLUTE_MAX_MEM_INDEX )
    hi_work_index = ABSOLUTE_MAX_MEM_INDEX;
else
    hi_work_index = hi_user_index;

index_range = hi_work_index - lo_work_index;
tuning_step_size = index_range / tuning_factor;

/*
** If tuning parameters don't make a difference (i.e., they
** result in an index range or tuning step size of zero),
** don't bother to do any more work.
*/
if ( index_range == 0 || tuning_step_size == 0 )
    return;

/*
** Let's tune it ...
*/

tuning_point_index = lo_work_index + (long)(index_range * tuning_bias);
lo_index_range = tuning_point_index - lo_work_index;
hi_index_range = hi_work_index - tuning_point_index;

if ( tuning_factor % 2 == 0 )
    /*
    ** If tuning factor is even, divide range evenly between
    ** lo and hi sides of the tuning point.
    */
    {
    lo_step_cnt = (long)(((double)tuning_factor / 2.0) + 0.5);
    hi_step_cnt = lo_step_cnt;
    }
else
    {
    /*
    ** If tuning factor is odd, divide range between the lo and
    ** hi sides of the tuning point, but let the hi side have the
    ** odd (or extra) step, so that higher sizes are more finely
    ** quantized.
    */
    lo_step_cnt = (long)(((double)tuning_factor / 2.0) + 0.5);
    hi_step_cnt = lo_step_cnt + 1;
    }

hi_step_size = hi_index_range / hi_step_cnt;
lo_step_size = lo_index_range / lo_step_cnt;

/*
** Replace table entry values by rounding them up to the
** max value for a given step size.  Start high and go backwards
** copying higher values into lower ones.  Do this separately for
** the lo side of the tuning point and the hi side.
*/
step_count = 0;
for ( table_index = tuning_point_index; 
	table_index > lo_work_index;
	    --table_index )
    {
    ++step_count;
    if ( step_count != lo_step_size )
	MemRoundupTable[ table_index - 1 ] = MemRoundupTable[ table_index ];
    else
	step_count = 0;
    } /* end for */

step_count = 0;
for ( table_index = hi_work_index; 
	table_index > tuning_point_index;
	    --table_index )
    {
    ++step_count;
    if ( step_count != hi_step_size )
	MemRoundupTable[ table_index - 1 ] = MemRoundupTable[ table_index ];
    else
	step_count = 0;
    } /* end for */

/*
** Store tuning parameters in module global storage for reference
** when debugging.
*/
ImgL_HiMemIndex		= hi_work_index;
ImgL_HiMemSize		= hi_size_limit;
ImgL_LoMemIndex		= lo_work_index;
ImgL_LoMemSize		= lo_size_limit;
ImgL_TuningFactor	= tuning_factor;
ImgL_TuningPointIndex	= tuning_point_index;
ImgD_TuningBias		= tuning_bias;

/*
** Print out tuning info if debug flag is true
*/
if ( ImgL_AutoTuneDebug )
    {
    long    idx;

    fprintf( output, "ImgL_LoMemSize        : %d\n", ImgL_LoMemSize );
    fprintf( output, "ImgL_LoMemIndex       : %d\n", ImgL_LoMemIndex );
    fprintf( output, "ImgL_HiMemSize        : %d\n", ImgL_HiMemSize );
    fprintf( output, "ImgL_HiMemIndex       : %d\n", ImgL_HiMemIndex );
    fprintf( output, "ImgL_TuningFactor     : %d\n", ImgL_TuningFactor );
    fprintf( output, "ImgL_TuningPointIndex : %d\n", ImgL_TuningPointIndex );
    fprintf( output, "ImgD_TuningBias       : %f\n", ImgD_TuningBias );

    if ( ImgL_AutoTuneDebugShowTable )
	for ( idx = ABSOLUTE_LOWEST_MEM_INDEX; idx < 256; ++idx )
	    fprintf( output, 
		    "index: %3d, size: %11u\n", idx, MemRoundupTable[ idx ] );
    }

return;
} /* end of _ImgInitilializeAutoTune */


/******************************************************************************
**  ImgShowAutoTuneHistogram
**
**  FUNCTIONAL DESCRIPTION:
**
**	This function prints out histograms of memory allocation request
**	sizes based on the free-list index number that the sizes 
**	correspond to.  The first histogram describes which free-list
**	sizes *would* have been used if auto-tuning wasn't in use.  The
**	second histogram describes the sizes that were actually used
**	after auto-tuning roundup.
**
**	NOTE that the histograms only keep track of sizes which are
**	within the specified (or defaulted) hi and lo memory range
**	sizes.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**	none
**
**  IMPLICIT OUTPUTS:
**
**	Histogram lists printed to the user's terminal.
**
**  FUNCTION VALUE:
**
**	void (none)
**
**  SIGNAL CODES:
**
**	none
**
**  SIDE EFFECTS:
**
**	none
**
******************************************************************************/
void ImgShowAutoTuneHistogram()
{
long	 list_used_cnt;
long	 idx;
long	 idx2;
FILE	*fp		= ImgL_AutoTuneHistOutputFile;

if ( !ImgL_AutoTuneDebugShowHistogram )
    {
    fprintf( fp, "Auto tuning histograms were not enabled.\n" );
    return;
    }

list_used_cnt = 0;
for ( idx = ABSOLUTE_LOWEST_MEM_INDEX; idx < 256; idx++ )
    {
    if ( ImgA_ReqSizes[ idx ] != 0 )
	++list_used_cnt;
    }

fprintf( fp, "Histogram of Projected Free-list Usage for Untuned Requested Sizes\n");
fprintf( fp, "Projected Free-lists used:  %d\n", list_used_cnt );
fprintf( fp, "\n" );
for ( idx = ABSOLUTE_LOWEST_MEM_INDEX; idx < 256; idx++ )
    {
    fprintf( fp, "Index:  %3d, Count:  %5d", idx, ImgA_ReqSizes[ idx ] );
    if ( ImgA_ReqSizes[ idx ] == 0 )
	fprintf( fp, "\n" );
    else
	{
	fprintf( fp, " " );
	for ( idx2 = 0; idx2 < ImgA_ReqSizes[ idx ]; ++idx2 )
	    fprintf( fp, "*" );
	fprintf( fp, "\n" );
	}
    }


list_used_cnt = 0;
for ( idx = ABSOLUTE_LOWEST_MEM_INDEX; idx < 256; idx++ )
    {
    if ( ImgA_WrkSizes[ idx ] != 0 )
	++list_used_cnt;
    }

fprintf( fp, "\n" );
fprintf( fp, "Histogram of Actual Free-list Usage for Autotuned Requested Sizes\n" );
fprintf( fp, "Actual Free-lists used:  %d\n", list_used_cnt );
fprintf( fp, "\n" );
for ( idx = ABSOLUTE_LOWEST_MEM_INDEX; idx < 256; idx++ )
    {
    fprintf( fp, "Index:  %3d, Count:  %5d", idx, ImgA_WrkSizes[ idx ] );
    if ( ImgA_WrkSizes[ idx ] == 0 )
	fprintf( fp, "\n" );
    else
	{
	fprintf( fp, " " );
	for ( idx2 = 0; idx2 < ImgA_WrkSizes[ idx ]; ++idx2 )
	    fprintf( fp, "*" );
	fprintf( fp, "\n" );
	}
    }

return;
} /* end ImgShowAutoTuneHistogram */
#endif


/*
** i_print
**
**  This routine prints the memory address and routine name
**  to the output file specified by the environment variable
**  "IMG_TRACE_MEM_FILE".
*/
static void i_print ( 
     char	*memadr
    ,long	 user_size
    ,char	*rtn_name
    )
{

if ( ImgL_MemTraceOutputFile )
    {
    fprintf( ImgL_MemTraceOutputFile, "%d (%12s, trace id: %7d, size: %d)\n", 
		memadr, rtn_name, ImgL_MemTraceId, user_size );
    }

++ImgL_MemTraceId;

return;
} /* end of i_print */
