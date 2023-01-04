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
 * @(#)$RCSfile: krashlib_arrays.h,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/07/29 15:26:27 $
 */
#define DEFINE_ARRAY(name, type) \
typedef struct _##name##Array { \
  unsigned int alloced; \
  unsigned int used; \
  type *elements; \
} name##Array
DEFINE_ARRAY(Field,  struct _Field);
#define FieldArraySize(a) ArrayUsed((Array *) (a))
#define FieldArrayElements(a) (( struct _Field *) ArrayElements((Array *) (a)))
#define FieldArrayElement(a, n) (( struct _Field *) ArrayElement(a, n))
#define FieldArrayFirstElement(a) (( struct _Field *) ArrayElement(a, 0))
#define FieldArrayLastElement(a) (( struct _Field *) ArrayElement(a, ArrayUsed(a) - 1))
#define FieldArrayNew(a) ((FieldArray *) ArrayNew((Array *) (a), sizeof( struct _Field)))
#define FieldArrayDestroy(a) ArrayDestroy((Array *) (a))
#define FieldArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _Field))
#define FieldArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _Field))
#define FieldArrayNextElement(a) (( struct _Field *) ArrayNextElement((Array *) (a), sizeof( struct _Field)))
#define FieldArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _Field))
#define FieldArraySize(a) ArrayUsed((Array *) (a))
#define FieldArrayEmpty(a) ArrayEmpty((Array *) (a))
#define FieldArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _Field))
#define FieldArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _Field), (void (*)()) NULL)
#define FieldArrayInsertBefore(a, e, ele) FieldArrayInsert((a), FieldArrayIndex((a), ((char *) e)), (ele))
#define FieldArrayInsertAfter(a, e, ele) FieldArrayInsert((a), FieldArrayIndex((a), ((char *) e)) + 1, (ele))
#define FieldArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _Field), (void (*)()) NULL)
#define FieldArrayDeleteElement(a, e) FieldArrayDelete((a), FieldArrayIndex((a), ((char *) e)))
#define FieldArrayAppend(a, e) FieldArrayInsert((a), FieldArraySize(a), (e))
#define FieldArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _Field))
#define FieldArrayIterPtr(a, e) for((e)=FieldArrayFirstElement(a);(e)<FieldArrayElement(a, FieldArraySize(a));(e)++)
#define FieldArrayIter(a, e, i) for((i)=0,(e)= *FieldArrayElement(a, i);(i)<FieldArraySize(a);(i)++,(e)= *FieldArrayElement(a, i))
#define FieldArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *FieldArrayElement(a, i);(i)!=(f);(n),(e)= *FieldArrayElement(a, i))
#define FieldArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=FieldArrayElement(a, i);(i)!=(f);(n),(e)=FieldArrayElement(a, i))
#define FieldArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _Field), (void (*)()) NULL)
#define FieldArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _Field), (void (*)()) NULL)

DEFINE_ARRAY(DataStruct,  struct _DataStructRec *);
#define DataStructArraySize(a) ArrayUsed((Array *) (a))
#define DataStructArrayElements(a) (( struct _DataStructRec * *) ArrayElements((Array *) (a)))
#define DataStructArrayElement(a, n) (( struct _DataStructRec * *) ArrayElement(a, n))
#define DataStructArrayFirstElement(a) (( struct _DataStructRec * *) ArrayElement(a, 0))
#define DataStructArrayLastElement(a) (( struct _DataStructRec * *) ArrayElement(a, ArrayUsed(a) - 1))
#define DataStructArrayNew(a) ((DataStructArray *) ArrayNew((Array *) (a), sizeof( struct _DataStructRec *)))
#define DataStructArrayDestroy(a) ArrayDestroy((Array *) (a))
#define DataStructArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _DataStructRec *))
#define DataStructArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _DataStructRec *))
#define DataStructArrayNextElement(a) (( struct _DataStructRec * *) ArrayNextElement((Array *) (a), sizeof( struct _DataStructRec *)))
#define DataStructArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _DataStructRec *))
#define DataStructArraySize(a) ArrayUsed((Array *) (a))
#define DataStructArrayEmpty(a) ArrayEmpty((Array *) (a))
#define DataStructArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _DataStructRec *))
#define DataStructArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _DataStructRec *), (void (*)()) NULL)
#define DataStructArrayInsertBefore(a, e, ele) DataStructArrayInsert((a), DataStructArrayIndex((a), ((char *) e)), (ele))
#define DataStructArrayInsertAfter(a, e, ele) DataStructArrayInsert((a), DataStructArrayIndex((a), ((char *) e)) + 1, (ele))
#define DataStructArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _DataStructRec *), (void (*)()) NULL)
#define DataStructArrayDeleteElement(a, e) DataStructArrayDelete((a), DataStructArrayIndex((a), ((char *) e)))
#define DataStructArrayAppend(a, e) DataStructArrayInsert((a), DataStructArraySize(a), (e))
#define DataStructArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _DataStructRec *))
#define DataStructArrayIterPtr(a, e) for((e)=DataStructArrayFirstElement(a);(e)<DataStructArrayElement(a, DataStructArraySize(a));(e)++)
#define DataStructArrayIter(a, e, i) for((i)=0,(e)= *DataStructArrayElement(a, i);(i)<DataStructArraySize(a);(i)++,(e)= *DataStructArrayElement(a, i))
#define DataStructArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *DataStructArrayElement(a, i);(i)!=(f);(n),(e)= *DataStructArrayElement(a, i))
#define DataStructArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=DataStructArrayElement(a, i);(i)!=(f);(n),(e)=DataStructArrayElement(a, i))
#define DataStructArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _DataStructRec *), (void (*)()) NULL)
#define DataStructArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _DataStructRec *), (void (*)()) NULL)

DEFINE_ARRAY(Type,  struct _Type);
#define TypeArraySize(a) ArrayUsed((Array *) (a))
#define TypeArrayElements(a) (( struct _Type *) ArrayElements((Array *) (a)))
#define TypeArrayElement(a, n) (( struct _Type *) ArrayElement(a, n))
#define TypeArrayFirstElement(a) (( struct _Type *) ArrayElement(a, 0))
#define TypeArrayLastElement(a) (( struct _Type *) ArrayElement(a, ArrayUsed(a) - 1))
#define TypeArrayNew(a) ((TypeArray *) ArrayNew((Array *) (a), sizeof( struct _Type)))
#define TypeArrayDestroy(a) ArrayDestroy((Array *) (a))
#define TypeArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _Type))
#define TypeArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _Type))
#define TypeArrayNextElement(a) (( struct _Type *) ArrayNextElement((Array *) (a), sizeof( struct _Type)))
#define TypeArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _Type))
#define TypeArraySize(a) ArrayUsed((Array *) (a))
#define TypeArrayEmpty(a) ArrayEmpty((Array *) (a))
#define TypeArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _Type))
#define TypeArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _Type), (void (*)()) NULL)
#define TypeArrayInsertBefore(a, e, ele) TypeArrayInsert((a), TypeArrayIndex((a), ((char *) e)), (ele))
#define TypeArrayInsertAfter(a, e, ele) TypeArrayInsert((a), TypeArrayIndex((a), ((char *) e)) + 1, (ele))
#define TypeArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _Type), (void (*)()) NULL)
#define TypeArrayDeleteElement(a, e) TypeArrayDelete((a), TypeArrayIndex((a), ((char *) e)))
#define TypeArrayAppend(a, e) TypeArrayInsert((a), TypeArraySize(a), (e))
#define TypeArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _Type))
#define TypeArrayIterPtr(a, e) for((e)=TypeArrayFirstElement(a);(e)<TypeArrayElement(a, TypeArraySize(a));(e)++)
#define TypeArrayIter(a, e, i) for((i)=0,(e)= *TypeArrayElement(a, i);(i)<TypeArraySize(a);(i)++,(e)= *TypeArrayElement(a, i))
#define TypeArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *TypeArrayElement(a, i);(i)!=(f);(n),(e)= *TypeArrayElement(a, i))
#define TypeArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=TypeArrayElement(a, i);(i)!=(f);(n),(e)=TypeArrayElement(a, i))
#define TypeArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _Type), (void (*)()) NULL)
#define TypeArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _Type), (void (*)()) NULL)

DEFINE_ARRAY(Packet,  struct _com_res);
#define PacketArraySize(a) ArrayUsed((Array *) (a))
#define PacketArrayElements(a) (( struct _com_res *) ArrayElements((Array *) (a)))
#define PacketArrayElement(a, n) (( struct _com_res *) ArrayElement(a, n))
#define PacketArrayFirstElement(a) (( struct _com_res *) ArrayElement(a, 0))
#define PacketArrayLastElement(a) (( struct _com_res *) ArrayElement(a, ArrayUsed(a) - 1))
#define PacketArrayNew(a) ((PacketArray *) ArrayNew((Array *) (a), sizeof( struct _com_res)))
#define PacketArrayDestroy(a) ArrayDestroy((Array *) (a))
#define PacketArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _com_res))
#define PacketArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _com_res))
#define PacketArrayNextElement(a) (( struct _com_res *) ArrayNextElement((Array *) (a), sizeof( struct _com_res)))
#define PacketArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _com_res))
#define PacketArraySize(a) ArrayUsed((Array *) (a))
#define PacketArrayEmpty(a) ArrayEmpty((Array *) (a))
#define PacketArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _com_res))
#define PacketArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _com_res), (void (*)()) NULL)
#define PacketArrayInsertBefore(a, e, ele) PacketArrayInsert((a), PacketArrayIndex((a), ((char *) e)), (ele))
#define PacketArrayInsertAfter(a, e, ele) PacketArrayInsert((a), PacketArrayIndex((a), ((char *) e)) + 1, (ele))
#define PacketArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _com_res), (void (*)()) NULL)
#define PacketArrayDeleteElement(a, e) PacketArrayDelete((a), PacketArrayIndex((a), ((char *) e)))
#define PacketArrayAppend(a, e) PacketArrayInsert((a), PacketArraySize(a), (e))
#define PacketArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _com_res))
#define PacketArrayIterPtr(a, e) for((e)=PacketArrayFirstElement(a);(e)<PacketArrayElement(a, PacketArraySize(a));(e)++)
#define PacketArrayIter(a, e, i) for((i)=0,(e)= *PacketArrayElement(a, i);(i)<PacketArraySize(a);(i)++,(e)= *PacketArrayElement(a, i))
#define PacketArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *PacketArrayElement(a, i);(i)!=(f);(n),(e)= *PacketArrayElement(a, i))
#define PacketArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=PacketArrayElement(a, i);(i)!=(f);(n),(e)=PacketArrayElement(a, i))
#define PacketArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _com_res), (void (*)()) NULL)
#define PacketArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _com_res), (void (*)()) NULL)

DEFINE_ARRAY(TmpField,  struct _TmpField);
#define TmpFieldArraySize(a) ArrayUsed((Array *) (a))
#define TmpFieldArrayElements(a) (( struct _TmpField *) ArrayElements((Array *) (a)))
#define TmpFieldArrayElement(a, n) (( struct _TmpField *) ArrayElement(a, n))
#define TmpFieldArrayFirstElement(a) (( struct _TmpField *) ArrayElement(a, 0))
#define TmpFieldArrayLastElement(a) (( struct _TmpField *) ArrayElement(a, ArrayUsed(a) - 1))
#define TmpFieldArrayNew(a) ((TmpFieldArray *) ArrayNew((Array *) (a), sizeof( struct _TmpField)))
#define TmpFieldArrayDestroy(a) ArrayDestroy((Array *) (a))
#define TmpFieldArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _TmpField))
#define TmpFieldArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _TmpField))
#define TmpFieldArrayNextElement(a) (( struct _TmpField *) ArrayNextElement((Array *) (a), sizeof( struct _TmpField)))
#define TmpFieldArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _TmpField))
#define TmpFieldArraySize(a) ArrayUsed((Array *) (a))
#define TmpFieldArrayEmpty(a) ArrayEmpty((Array *) (a))
#define TmpFieldArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _TmpField))
#define TmpFieldArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _TmpField), (void (*)()) NULL)
#define TmpFieldArrayInsertBefore(a, e, ele) TmpFieldArrayInsert((a), TmpFieldArrayIndex((a), ((char *) e)), (ele))
#define TmpFieldArrayInsertAfter(a, e, ele) TmpFieldArrayInsert((a), TmpFieldArrayIndex((a), ((char *) e)) + 1, (ele))
#define TmpFieldArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _TmpField), (void (*)()) NULL)
#define TmpFieldArrayDeleteElement(a, e) TmpFieldArrayDelete((a), TmpFieldArrayIndex((a), ((char *) e)))
#define TmpFieldArrayAppend(a, e) TmpFieldArrayInsert((a), TmpFieldArraySize(a), (e))
#define TmpFieldArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _TmpField))
#define TmpFieldArrayIterPtr(a, e) for((e)=TmpFieldArrayFirstElement(a);(e)<TmpFieldArrayElement(a, TmpFieldArraySize(a));(e)++)
#define TmpFieldArrayIter(a, e, i) for((i)=0,(e)= *TmpFieldArrayElement(a, i);(i)<TmpFieldArraySize(a);(i)++,(e)= *TmpFieldArrayElement(a, i))
#define TmpFieldArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *TmpFieldArrayElement(a, i);(i)!=(f);(n),(e)= *TmpFieldArrayElement(a, i))
#define TmpFieldArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=TmpFieldArrayElement(a, i);(i)!=(f);(n),(e)=TmpFieldArrayElement(a, i))
#define TmpFieldArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _TmpField), (void (*)()) NULL)
#define TmpFieldArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _TmpField), (void (*)()) NULL)

DEFINE_ARRAY(Int,  unsigned int);
#define IntArraySize(a) ArrayUsed((Array *) (a))
#define IntArrayElements(a) (( unsigned int *) ArrayElements((Array *) (a)))
#define IntArrayElement(a, n) (( unsigned int *) ArrayElement(a, n))
#define IntArrayFirstElement(a) (( unsigned int *) ArrayElement(a, 0))
#define IntArrayLastElement(a) (( unsigned int *) ArrayElement(a, ArrayUsed(a) - 1))
#define IntArrayNew(a) ((IntArray *) ArrayNew((Array *) (a), sizeof( unsigned int)))
#define IntArrayDestroy(a) ArrayDestroy((Array *) (a))
#define IntArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( unsigned int))
#define IntArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( unsigned int))
#define IntArrayNextElement(a) (( unsigned int *) ArrayNextElement((Array *) (a), sizeof( unsigned int)))
#define IntArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( unsigned int))
#define IntArraySize(a) ArrayUsed((Array *) (a))
#define IntArrayEmpty(a) ArrayEmpty((Array *) (a))
#define IntArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( unsigned int))
#define IntArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( unsigned int), (void (*)()) NULL)
#define IntArrayInsertBefore(a, e, ele) IntArrayInsert((a), IntArrayIndex((a), ((char *) e)), (ele))
#define IntArrayInsertAfter(a, e, ele) IntArrayInsert((a), IntArrayIndex((a), ((char *) e)) + 1, (ele))
#define IntArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( unsigned int), (void (*)()) NULL)
#define IntArrayDeleteElement(a, e) IntArrayDelete((a), IntArrayIndex((a), ((char *) e)))
#define IntArrayAppend(a, e) IntArrayInsert((a), IntArraySize(a), (e))
#define IntArraySort(a, p) ArraySort((Array *) (a), p, sizeof( unsigned int))
#define IntArrayIterPtr(a, e) for((e)=IntArrayFirstElement(a);(e)<IntArrayElement(a, IntArraySize(a));(e)++)
#define IntArrayIter(a, e, i) for((i)=0,(e)= *IntArrayElement(a, i);(i)<IntArraySize(a);(i)++,(e)= *IntArrayElement(a, i))
#define IntArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *IntArrayElement(a, i);(i)!=(f);(n),(e)= *IntArrayElement(a, i))
#define IntArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=IntArrayElement(a, i);(i)!=(f);(n),(e)=IntArrayElement(a, i))
#define IntArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( unsigned int), (void (*)()) NULL)
#define IntArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( unsigned int), (void (*)()) NULL)

DEFINE_ARRAY(Addr,  long);
#define AddrArraySize(a) ArrayUsed((Array *) (a))
#define AddrArrayElements(a) (( long *) ArrayElements((Array *) (a)))
#define AddrArrayElement(a, n) (( long *) ArrayElement(a, n))
#define AddrArrayFirstElement(a) (( long *) ArrayElement(a, 0))
#define AddrArrayLastElement(a) (( long *) ArrayElement(a, ArrayUsed(a) - 1))
#define AddrArrayNew(a) ((AddrArray *) ArrayNew((Array *) (a), sizeof( long)))
#define AddrArrayDestroy(a) ArrayDestroy((Array *) (a))
#define AddrArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( long))
#define AddrArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( long))
#define AddrArrayNextElement(a) (( long *) ArrayNextElement((Array *) (a), sizeof( long)))
#define AddrArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( long))
#define AddrArraySize(a) ArrayUsed((Array *) (a))
#define AddrArrayEmpty(a) ArrayEmpty((Array *) (a))
#define AddrArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( long))
#define AddrArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( long), (void (*)()) NULL)
#define AddrArrayInsertBefore(a, e, ele) AddrArrayInsert((a), AddrArrayIndex((a), ((char *) e)), (ele))
#define AddrArrayInsertAfter(a, e, ele) AddrArrayInsert((a), AddrArrayIndex((a), ((char *) e)) + 1, (ele))
#define AddrArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( long), (void (*)()) NULL)
#define AddrArrayDeleteElement(a, e) AddrArrayDelete((a), AddrArrayIndex((a), ((char *) e)))
#define AddrArrayAppend(a, e) AddrArrayInsert((a), AddrArraySize(a), (e))
#define AddrArraySort(a, p) ArraySort((Array *) (a), p, sizeof( long))
#define AddrArrayIterPtr(a, e) for((e)=AddrArrayFirstElement(a);(e)<AddrArrayElement(a, AddrArraySize(a));(e)++)
#define AddrArrayIter(a, e, i) for((i)=0,(e)= *AddrArrayElement(a, i);(i)<AddrArraySize(a);(i)++,(e)= *AddrArrayElement(a, i))
#define AddrArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *AddrArrayElement(a, i);(i)!=(f);(n),(e)= *AddrArrayElement(a, i))
#define AddrArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=AddrArrayElement(a, i);(i)!=(f);(n),(e)=AddrArrayElement(a, i))
#define AddrArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( long), (void (*)()) NULL)
#define AddrArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( long), (void (*)()) NULL)

DEFINE_ARRAY(Batch,  struct _Batch);
#define BatchArraySize(a) ArrayUsed((Array *) (a))
#define BatchArrayElements(a) (( struct _Batch *) ArrayElements((Array *) (a)))
#define BatchArrayElement(a, n) (( struct _Batch *) ArrayElement(a, n))
#define BatchArrayFirstElement(a) (( struct _Batch *) ArrayElement(a, 0))
#define BatchArrayLastElement(a) (( struct _Batch *) ArrayElement(a, ArrayUsed(a) - 1))
#define BatchArrayNew(a) ((BatchArray *) ArrayNew((Array *) (a), sizeof( struct _Batch)))
#define BatchArrayDestroy(a) ArrayDestroy((Array *) (a))
#define BatchArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _Batch))
#define BatchArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _Batch))
#define BatchArrayNextElement(a) (( struct _Batch *) ArrayNextElement((Array *) (a), sizeof( struct _Batch)))
#define BatchArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _Batch))
#define BatchArraySize(a) ArrayUsed((Array *) (a))
#define BatchArrayEmpty(a) ArrayEmpty((Array *) (a))
#define BatchArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _Batch))
#define BatchArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _Batch), (void (*)()) NULL)
#define BatchArrayInsertBefore(a, e, ele) BatchArrayInsert((a), BatchArrayIndex((a), ((char *) e)), (ele))
#define BatchArrayInsertAfter(a, e, ele) BatchArrayInsert((a), BatchArrayIndex((a), ((char *) e)) + 1, (ele))
#define BatchArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _Batch), (void (*)()) NULL)
#define BatchArrayDeleteElement(a, e) BatchArrayDelete((a), BatchArrayIndex((a), ((char *) e)))
#define BatchArrayAppend(a, e) BatchArrayInsert((a), BatchArraySize(a), (e))
#define BatchArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _Batch))
#define BatchArrayIterPtr(a, e) for((e)=BatchArrayFirstElement(a);(e)<BatchArrayElement(a, BatchArraySize(a));(e)++)
#define BatchArrayIter(a, e, i) for((i)=0,(e)= *BatchArrayElement(a, i);(i)<BatchArraySize(a);(i)++,(e)= *BatchArrayElement(a, i))
#define BatchArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *BatchArrayElement(a, i);(i)!=(f);(n),(e)= *BatchArrayElement(a, i))
#define BatchArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=BatchArrayElement(a, i);(i)!=(f);(n),(e)=BatchArrayElement(a, i))
#define BatchArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _Batch), (void (*)()) NULL)
#define BatchArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _Batch), (void (*)()) NULL)

