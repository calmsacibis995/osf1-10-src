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
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: UilSarDef.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/07 00:37:04 $ */

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This include file defines the interface to the UIL parser.
**	UIL uses YACC as its parsing tool.
**
**--
**/

#ifndef UilSarDef_h
#define UilSarDef_h



/*
**  Format of a value on YACC value stack.  This is also the form of a
**  token created by the lexical analyzer.
*/

#define	    sar_k_null_frame	0	/* tag for an epsilon production */
#define	    sar_k_token_frame	1	/* tag for a token frame */
#define	    sar_k_value_frame	2	/* tag for a value frame */
#define	    sar_k_module_frame	3	/* tag for module frame */
#define     sar_k_object_frame	4	/* tag for object frame */
#define     sar_k_root_frame    5	/* tag for root frame */

typedef struct
{
    src_source_record_type  *az_source_record;	/* actual record where token exists */
    unsigned char	    b_source_pos;	/* the character in az_source_record
						   where this token begins */
    unsigned char	    b_source_end;	/* the character in az_source_record
						   where this token ends */
    unsigned char	    b_tag;		/* tag of stack frame */
    unsigned char	    b_type;		/* for tokens - token number
						   for value - the data type */
    unsigned short	    b_flags;		/* used by value */
    unsigned char	    b_direction;	/* used by value */
    unsigned char	    b_charset;		/* used by value */
    union
    {
#ifdef ALPHA_BUG_FIX
    /* must be capable of holding a pointer */
	long		    l_integer;		/* integer value*/
#else
	int		    l_integer;		/* integer value*/
#endif
	sym_entry_type	    *az_symbol_entry;	/* symbol entry */
	key_keytable_entry_type
			    *az_keyword_entry;	/* keyword entry */
    }	value;
} yystype;


/*
**  Macros for moving source information to and from parse stack frames
*/

#define    _sar_move_source_info( _target, _source )			\
	   {								\
		yystype	    *__target;					\
		yystype	    *__source;					\
									\
		__target = (_target);  __source = (_source);		\
		__target->az_source_record = __source->az_source_record;\
		__target->b_source_pos = __source->b_source_pos;	\
		__target->b_source_end = __source->b_source_end;	\
	   }

#define    _sar_move_source_info_2( _target, _source )		\
   {								\
	sym_entry_header_type	*__target;			\
	sym_entry_header_type	*__source;			\
								\
	__target = (_target);  __source = (_source);		\
								\
	__target->az_src_rec = __source->az_src_rec;		\
	__target->b_src_pos = __source->b_src_pos;		\
	__target->b_end_pos = __source->b_end_pos;		\
   }


#define _sar_save_source_info( _target, _src_beg, _src_end )	\
   {								\
	sym_entry_header_type	*__target;			\
	XmConst yystype	    		*__src_end;		\
								\
	__target = (_target); 					\
	__src_end = (_src_end);					\
								\
	__target->az_src_rec	= __src_end->az_source_record;	\
	__target->b_src_pos	= __src_end->b_source_pos;	\
	__target->b_end_pos	= __src_end->b_source_end;	\
   }

#define _sar_save_source_pos( _target, _src )			\
   {								\
	sym_entry_header_type	*__target;			\
	XmConst yystype		*__src;				\
								\
	__target = (_target);	__src = (_src);			\
								\
	__target->az_src_rec	= __src->az_source_record;	\
	__target->b_src_pos	= __src->b_source_pos;		\
	__target->b_end_pos	= __src->b_source_end;		\
   }

#define    _sar_source_position( _source )			\
		_source->az_source_record, 			\
		_source->b_source_pos

#define    _sar_source_pos2( _source )				\
		_source->header.az_src_rec, 			\
		_source->header.b_src_pos



#endif /* UilSarDef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
