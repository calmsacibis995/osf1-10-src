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
 * @(#)$RCSfile: krashlib.h,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/11/05 17:58:21 $
 */
#ifndef __KRASHLIB_H__
#define __KRASHLIB_H__

#include "krashlib_arrays.h"
#include "util.h"
#include <sys/types.h>

#define IS_IDCHAR(c) (isalnum(c) || ((c) == '_'))

#define IS_FIELDNAME_CHAR(c) (IS_IDCHAR(c) || ((c) == '.'))

#define FIELD_ERROR(field, nfields, error) \
{ \
  if(nfields > 0) fields[0].error = error; \
  else fprintf(stderr, "%s\n", error); \
} 

#define STRUCT (1<<0)
#define WHATIS (1<<1)

typedef enum { Pointer, TmpStruct, Structure, TmpUnion, Union, TmpArray,
		 ArrayType, ErrorType, StringType, Number, BitString }
StructType;

typedef struct {
  char *name;
  int type;
  caddr_t data;
  char *error;
} FieldRec;

typedef struct _Field {
  char *name;
  struct _DataStructRec *data;
} Field;

typedef struct _Type {
  char *name;
  struct _DataStructRec *data;
  Boolean whatisable;
} Type;

typedef struct _TmpField {
  char *field;
  int len;
} TmpField;

typedef enum { Dot, Arrow, Index } FieldType;

typedef struct {
  FieldArray *fields;
  int low;
  int high;
  char *signature;
} StructU;

typedef struct {
  char *name;
  int name_len;
  TmpFieldArray *fields;
  char *signature_end;
  char *after;
} TmpStructU;

typedef struct {
  int size;
  struct _DataStructRec *type;
} ArrayU;

typedef struct {
  int size;
  char *element;
  int len;
  char *name;
  int name_len;
  char *after;
} TmpArrayU;

typedef struct _DataStructRec {
  StructType type;
  unsigned int method;
  char *addr;
  long *data;
  Boolean flag;
  int offset;
  int size;
  int bit_offset;	/* for type=BitString */
  int bit_size;		/* for type=BitString */
  int info;
  union {
    StructU structure;
    TmpStructU tmpstruct;
    struct _DataStructRec *pointer;
    ArrayU array;
    TmpArrayU tmparray;
    char *error;
  } u;
} DataStructRec, *DataStruct;

typedef struct {
  DataStruct data;
  DataStruct *dataptr;
  char *base_name;
  Boolean whatisable;
  unsigned int parent_offset;
  char *hint;
} OffsetSizeRec;

typedef struct {
  DataStruct base;
  char *name;
  int type;
  int info;
} ReadValRec;

typedef struct _Batch {
  Boolean (*proc)();
  Boolean send_output;
  caddr_t arg;
  Boolean free;
} Batch;

typedef struct {
  DataStruct data;
  DataStruct parent;
  OffsetSizeRec *rec;
  char *name;
  Boolean set_offset;
  FieldRec *field;
  FieldType type;  
} OffsetCbRec;

typedef struct {
  Boolean replace;
  DataStruct data;
  FieldRec *field;
} SizeCbRec;

#endif
