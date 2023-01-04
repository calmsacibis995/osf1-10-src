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
 * @(#)$RCSfile: krash_arrays.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:57:46 $
 */
#define DEFINE_ARRAY(name, type) \
typedef struct _##name##Array { \
  unsigned int alloced; \
  unsigned int used; \
  type *elements; \
} name##Array
DEFINE_ARRAY(String,  char *);
#define StringArraySize(a) ArrayUsed((Array *) (a))
#define StringArrayElements(a) (( char * *) ArrayElements((Array *) (a)))
#define StringArrayElement(a, n) (( char * *) ArrayElement(a, n))
#define StringArrayFirstElement(a) (( char * *) ArrayElement(a, 0))
#define StringArrayLastElement(a) (( char * *) ArrayElement(a, ArrayUsed(a) - 1))
#define StringArrayNew(a) ((StringArray *) ArrayNew((Array *) (a), sizeof( char *)))
#define StringArrayDestroy(a) ArrayDestroy((Array *) (a))
#define StringArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( char *))
#define StringArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( char *))
#define StringArrayNextElement(a) (( char * *) ArrayNextElement((Array *) (a), sizeof( char *)))
#define StringArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( char *))
#define StringArraySize(a) ArrayUsed((Array *) (a))
#define StringArrayEmpty(a) ArrayEmpty((Array *) (a))
#define StringArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( char *))
#define StringArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( char *), (void (*)()) NULL)
#define StringArrayInsertBefore(a, e, ele) StringArrayInsert((a), StringArrayIndex((a), ((char *) e)), (ele))
#define StringArrayInsertAfter(a, e, ele) StringArrayInsert((a), StringArrayIndex((a), ((char *) e)) + 1, (ele))
#define StringArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( char *), (void (*)()) NULL)
#define StringArrayDeleteElement(a, e) StringArrayDelete((a), StringArrayIndex((a), ((char *) e)))
#define StringArrayAppend(a, e) StringArrayInsert((a), StringArraySize(a), (e))
#define StringArraySort(a, p) ArraySort((Array *) (a), p, sizeof( char *))
#define StringArrayIterPtr(a, e) for((e)=StringArrayFirstElement(a);(e)<StringArrayElement(a, StringArraySize(a));(e)++)
#define StringArrayIter(a, e, i) for((i)=0,(e)= *StringArrayElement(a, i);(i)<StringArraySize(a);(i)++,(e)= *StringArrayElement(a, i))
#define StringArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *StringArrayElement(a, i);(i)!=(f);(n),(e)= *StringArrayElement(a, i))
#define StringArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=StringArrayElement(a, i);(i)!=(f);(n),(e)=StringArrayElement(a, i))
#define StringArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( char *), (void (*)()) NULL)
#define StringArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( char *), (void (*)()) NULL)

DEFINE_ARRAY(Alias,  struct _AliasRec);
#define AliasArraySize(a) ArrayUsed((Array *) (a))
#define AliasArrayElements(a) (( struct _AliasRec *) ArrayElements((Array *) (a)))
#define AliasArrayElement(a, n) (( struct _AliasRec *) ArrayElement(a, n))
#define AliasArrayFirstElement(a) (( struct _AliasRec *) ArrayElement(a, 0))
#define AliasArrayLastElement(a) (( struct _AliasRec *) ArrayElement(a, ArrayUsed(a) - 1))
#define AliasArrayNew(a) ((AliasArray *) ArrayNew((Array *) (a), sizeof( struct _AliasRec)))
#define AliasArrayDestroy(a) ArrayDestroy((Array *) (a))
#define AliasArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _AliasRec))
#define AliasArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _AliasRec))
#define AliasArrayNextElement(a) (( struct _AliasRec *) ArrayNextElement((Array *) (a), sizeof( struct _AliasRec)))
#define AliasArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _AliasRec))
#define AliasArraySize(a) ArrayUsed((Array *) (a))
#define AliasArrayEmpty(a) ArrayEmpty((Array *) (a))
#define AliasArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _AliasRec))
#define AliasArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _AliasRec), (void (*)()) NULL)
#define AliasArrayInsertBefore(a, e, ele) AliasArrayInsert((a), AliasArrayIndex((a), ((char *) e)), (ele))
#define AliasArrayInsertAfter(a, e, ele) AliasArrayInsert((a), AliasArrayIndex((a), ((char *) e)) + 1, (ele))
#define AliasArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _AliasRec), (void (*)()) NULL)
#define AliasArrayDeleteElement(a, e) AliasArrayDelete((a), AliasArrayIndex((a), ((char *) e)))
#define AliasArrayAppend(a, e) AliasArrayInsert((a), AliasArraySize(a), (e))
#define AliasArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _AliasRec))
#define AliasArrayIterPtr(a, e) for((e)=AliasArrayFirstElement(a);(e)<AliasArrayElement(a, AliasArraySize(a));(e)++)
#define AliasArrayIter(a, e, i) for((i)=0,(e)= *AliasArrayElement(a, i);(i)<AliasArraySize(a);(i)++,(e)= *AliasArrayElement(a, i))
#define AliasArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *AliasArrayElement(a, i);(i)!=(f);(n),(e)= *AliasArrayElement(a, i))
#define AliasArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=AliasArrayElement(a, i);(i)!=(f);(n),(e)=AliasArrayElement(a, i))
#define AliasArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _AliasRec), (void (*)()) NULL)
#define AliasArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _AliasRec), (void (*)()) NULL)

DEFINE_ARRAY(Pid,  struct _PidRec);
#define PidArraySize(a) ArrayUsed((Array *) (a))
#define PidArrayElements(a) (( struct _PidRec *) ArrayElements((Array *) (a)))
#define PidArrayElement(a, n) (( struct _PidRec *) ArrayElement(a, n))
#define PidArrayFirstElement(a) (( struct _PidRec *) ArrayElement(a, 0))
#define PidArrayLastElement(a) (( struct _PidRec *) ArrayElement(a, ArrayUsed(a) - 1))
#define PidArrayNew(a) ((PidArray *) ArrayNew((Array *) (a), sizeof( struct _PidRec)))
#define PidArrayDestroy(a) ArrayDestroy((Array *) (a))
#define PidArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _PidRec))
#define PidArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _PidRec))
#define PidArrayNextElement(a) (( struct _PidRec *) ArrayNextElement((Array *) (a), sizeof( struct _PidRec)))
#define PidArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _PidRec))
#define PidArraySize(a) ArrayUsed((Array *) (a))
#define PidArrayEmpty(a) ArrayEmpty((Array *) (a))
#define PidArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _PidRec))
#define PidArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _PidRec), (void (*)()) NULL)
#define PidArrayInsertBefore(a, e, ele) PidArrayInsert((a), PidArrayIndex((a), ((char *) e)), (ele))
#define PidArrayInsertAfter(a, e, ele) PidArrayInsert((a), PidArrayIndex((a), ((char *) e)) + 1, (ele))
#define PidArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _PidRec), (void (*)()) NULL)
#define PidArrayDeleteElement(a, e) PidArrayDelete((a), PidArrayIndex((a), ((char *) e)))
#define PidArrayAppend(a, e) PidArrayInsert((a), PidArraySize(a), (e))
#define PidArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _PidRec))
#define PidArrayIterPtr(a, e) for((e)=PidArrayFirstElement(a);(e)<PidArrayElement(a, PidArraySize(a));(e)++)
#define PidArrayIter(a, e, i) for((i)=0,(e)= *PidArrayElement(a, i);(i)<PidArraySize(a);(i)++,(e)= *PidArrayElement(a, i))
#define PidArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *PidArrayElement(a, i);(i)!=(f);(n),(e)= *PidArrayElement(a, i))
#define PidArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=PidArrayElement(a, i);(i)!=(f);(n),(e)=PidArrayElement(a, i))
#define PidArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _PidRec), (void (*)()) NULL)
#define PidArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _PidRec), (void (*)()) NULL)

DEFINE_ARRAY(Token,  struct _Token);
#define TokenArraySize(a) ArrayUsed((Array *) (a))
#define TokenArrayElements(a) (( struct _Token *) ArrayElements((Array *) (a)))
#define TokenArrayElement(a, n) (( struct _Token *) ArrayElement(a, n))
#define TokenArrayFirstElement(a) (( struct _Token *) ArrayElement(a, 0))
#define TokenArrayLastElement(a) (( struct _Token *) ArrayElement(a, ArrayUsed(a) - 1))
#define TokenArrayNew(a) ((TokenArray *) ArrayNew((Array *) (a), sizeof( struct _Token)))
#define TokenArrayDestroy(a) ArrayDestroy((Array *) (a))
#define TokenArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _Token))
#define TokenArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _Token))
#define TokenArrayNextElement(a) (( struct _Token *) ArrayNextElement((Array *) (a), sizeof( struct _Token)))
#define TokenArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _Token))
#define TokenArraySize(a) ArrayUsed((Array *) (a))
#define TokenArrayEmpty(a) ArrayEmpty((Array *) (a))
#define TokenArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _Token))
#define TokenArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _Token), (void (*)()) NULL)
#define TokenArrayInsertBefore(a, e, ele) TokenArrayInsert((a), TokenArrayIndex((a), ((char *) e)), (ele))
#define TokenArrayInsertAfter(a, e, ele) TokenArrayInsert((a), TokenArrayIndex((a), ((char *) e)) + 1, (ele))
#define TokenArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _Token), (void (*)()) NULL)
#define TokenArrayDeleteElement(a, e) TokenArrayDelete((a), TokenArrayIndex((a), ((char *) e)))
#define TokenArrayAppend(a, e) TokenArrayInsert((a), TokenArraySize(a), (e))
#define TokenArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _Token))
#define TokenArrayIterPtr(a, e) for((e)=TokenArrayFirstElement(a);(e)<TokenArrayElement(a, TokenArraySize(a));(e)++)
#define TokenArrayIter(a, e, i) for((i)=0,(e)= *TokenArrayElement(a, i);(i)<TokenArraySize(a);(i)++,(e)= *TokenArrayElement(a, i))
#define TokenArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *TokenArrayElement(a, i);(i)!=(f);(n),(e)= *TokenArrayElement(a, i))
#define TokenArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=TokenArrayElement(a, i);(i)!=(f);(n),(e)=TokenArrayElement(a, i))
#define TokenArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _Token), (void (*)()) NULL)
#define TokenArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _Token), (void (*)()) NULL)

DEFINE_ARRAY(IO,  struct _IORec);
#define IOArraySize(a) ArrayUsed((Array *) (a))
#define IOArrayElements(a) (( struct _IORec *) ArrayElements((Array *) (a)))
#define IOArrayElement(a, n) (( struct _IORec *) ArrayElement(a, n))
#define IOArrayFirstElement(a) (( struct _IORec *) ArrayElement(a, 0))
#define IOArrayLastElement(a) (( struct _IORec *) ArrayElement(a, ArrayUsed(a) - 1))
#define IOArrayNew(a) ((IOArray *) ArrayNew((Array *) (a), sizeof( struct _IORec)))
#define IOArrayDestroy(a) ArrayDestroy((Array *) (a))
#define IOArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _IORec))
#define IOArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _IORec))
#define IOArrayNextElement(a) (( struct _IORec *) ArrayNextElement((Array *) (a), sizeof( struct _IORec)))
#define IOArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _IORec))
#define IOArraySize(a) ArrayUsed((Array *) (a))
#define IOArrayEmpty(a) ArrayEmpty((Array *) (a))
#define IOArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _IORec))
#define IOArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _IORec), (void (*)()) NULL)
#define IOArrayInsertBefore(a, e, ele) IOArrayInsert((a), IOArrayIndex((a), ((char *) e)), (ele))
#define IOArrayInsertAfter(a, e, ele) IOArrayInsert((a), IOArrayIndex((a), ((char *) e)) + 1, (ele))
#define IOArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _IORec), (void (*)()) NULL)
#define IOArrayDeleteElement(a, e) IOArrayDelete((a), IOArrayIndex((a), ((char *) e)))
#define IOArrayAppend(a, e) IOArrayInsert((a), IOArraySize(a), (e))
#define IOArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _IORec))
#define IOArrayIterPtr(a, e) for((e)=IOArrayFirstElement(a);(e)<IOArrayElement(a, IOArraySize(a));(e)++)
#define IOArrayIter(a, e, i) for((i)=0,(e)= *IOArrayElement(a, i);(i)<IOArraySize(a);(i)++,(e)= *IOArrayElement(a, i))
#define IOArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *IOArrayElement(a, i);(i)!=(f);(n),(e)= *IOArrayElement(a, i))
#define IOArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=IOArrayElement(a, i);(i)!=(f);(n),(e)=IOArrayElement(a, i))
#define IOArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _IORec), (void (*)()) NULL)
#define IOArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _IORec), (void (*)()) NULL)

DEFINE_ARRAY(Level,  struct _CleanupRec);
#define LevelArraySize(a) ArrayUsed((Array *) (a))
#define LevelArrayElements(a) (( struct _CleanupRec *) ArrayElements((Array *) (a)))
#define LevelArrayElement(a, n) (( struct _CleanupRec *) ArrayElement(a, n))
#define LevelArrayFirstElement(a) (( struct _CleanupRec *) ArrayElement(a, 0))
#define LevelArrayLastElement(a) (( struct _CleanupRec *) ArrayElement(a, ArrayUsed(a) - 1))
#define LevelArrayNew(a) ((LevelArray *) ArrayNew((Array *) (a), sizeof( struct _CleanupRec)))
#define LevelArrayDestroy(a) ArrayDestroy((Array *) (a))
#define LevelArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _CleanupRec))
#define LevelArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _CleanupRec))
#define LevelArrayNextElement(a) (( struct _CleanupRec *) ArrayNextElement((Array *) (a), sizeof( struct _CleanupRec)))
#define LevelArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _CleanupRec))
#define LevelArraySize(a) ArrayUsed((Array *) (a))
#define LevelArrayEmpty(a) ArrayEmpty((Array *) (a))
#define LevelArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _CleanupRec))
#define LevelArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _CleanupRec), (void (*)()) NULL)
#define LevelArrayInsertBefore(a, e, ele) LevelArrayInsert((a), LevelArrayIndex((a), ((char *) e)), (ele))
#define LevelArrayInsertAfter(a, e, ele) LevelArrayInsert((a), LevelArrayIndex((a), ((char *) e)) + 1, (ele))
#define LevelArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _CleanupRec), (void (*)()) NULL)
#define LevelArrayDeleteElement(a, e) LevelArrayDelete((a), LevelArrayIndex((a), ((char *) e)))
#define LevelArrayAppend(a, e) LevelArrayInsert((a), LevelArraySize(a), (e))
#define LevelArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _CleanupRec))
#define LevelArrayIterPtr(a, e) for((e)=LevelArrayFirstElement(a);(e)<LevelArrayElement(a, LevelArraySize(a));(e)++)
#define LevelArrayIter(a, e, i) for((i)=0,(e)= *LevelArrayElement(a, i);(i)<LevelArraySize(a);(i)++,(e)= *LevelArrayElement(a, i))
#define LevelArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *LevelArrayElement(a, i);(i)!=(f);(n),(e)= *LevelArrayElement(a, i))
#define LevelArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=LevelArrayElement(a, i);(i)!=(f);(n),(e)=LevelArrayElement(a, i))
#define LevelArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _CleanupRec), (void (*)()) NULL)
#define LevelArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _CleanupRec), (void (*)()) NULL)

DEFINE_ARRAY(Cleanup,  struct _LevelArray *);
#define CleanupArraySize(a) ArrayUsed((Array *) (a))
#define CleanupArrayElements(a) (( struct _LevelArray * *) ArrayElements((Array *) (a)))
#define CleanupArrayElement(a, n) (( struct _LevelArray * *) ArrayElement(a, n))
#define CleanupArrayFirstElement(a) (( struct _LevelArray * *) ArrayElement(a, 0))
#define CleanupArrayLastElement(a) (( struct _LevelArray * *) ArrayElement(a, ArrayUsed(a) - 1))
#define CleanupArrayNew(a) ((CleanupArray *) ArrayNew((Array *) (a), sizeof( struct _LevelArray *)))
#define CleanupArrayDestroy(a) ArrayDestroy((Array *) (a))
#define CleanupArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( struct _LevelArray *))
#define CleanupArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( struct _LevelArray *))
#define CleanupArrayNextElement(a) (( struct _LevelArray * *) ArrayNextElement((Array *) (a), sizeof( struct _LevelArray *)))
#define CleanupArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( struct _LevelArray *))
#define CleanupArraySize(a) ArrayUsed((Array *) (a))
#define CleanupArrayEmpty(a) ArrayEmpty((Array *) (a))
#define CleanupArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( struct _LevelArray *))
#define CleanupArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( struct _LevelArray *), (void (*)()) NULL)
#define CleanupArrayInsertBefore(a, e, ele) CleanupArrayInsert((a), CleanupArrayIndex((a), ((char *) e)), (ele))
#define CleanupArrayInsertAfter(a, e, ele) CleanupArrayInsert((a), CleanupArrayIndex((a), ((char *) e)) + 1, (ele))
#define CleanupArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( struct _LevelArray *), (void (*)()) NULL)
#define CleanupArrayDeleteElement(a, e) CleanupArrayDelete((a), CleanupArrayIndex((a), ((char *) e)))
#define CleanupArrayAppend(a, e) CleanupArrayInsert((a), CleanupArraySize(a), (e))
#define CleanupArraySort(a, p) ArraySort((Array *) (a), p, sizeof( struct _LevelArray *))
#define CleanupArrayIterPtr(a, e) for((e)=CleanupArrayFirstElement(a);(e)<CleanupArrayElement(a, CleanupArraySize(a));(e)++)
#define CleanupArrayIter(a, e, i) for((i)=0,(e)= *CleanupArrayElement(a, i);(i)<CleanupArraySize(a);(i)++,(e)= *CleanupArrayElement(a, i))
#define CleanupArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *CleanupArrayElement(a, i);(i)!=(f);(n),(e)= *CleanupArrayElement(a, i))
#define CleanupArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=CleanupArrayElement(a, i);(i)!=(f);(n),(e)=CleanupArrayElement(a, i))
#define CleanupArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( struct _LevelArray *), (void (*)()) NULL)
#define CleanupArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( struct _LevelArray *), (void (*)()) NULL)

DEFINE_ARRAY(Proc,  pid_t);
#define ProcArraySize(a) ArrayUsed((Array *) (a))
#define ProcArrayElements(a) (( pid_t *) ArrayElements((Array *) (a)))
#define ProcArrayElement(a, n) (( pid_t *) ArrayElement(a, n))
#define ProcArrayFirstElement(a) (( pid_t *) ArrayElement(a, 0))
#define ProcArrayLastElement(a) (( pid_t *) ArrayElement(a, ArrayUsed(a) - 1))
#define ProcArrayNew(a) ((ProcArray *) ArrayNew((Array *) (a), sizeof( pid_t)))
#define ProcArrayDestroy(a) ArrayDestroy((Array *) (a))
#define ProcArraySetElement(a, i, d) ArraySetElement((Array *) (a), (i), ((char *) d), sizeof( pid_t))
#define ProcArrayNextIndex(a) ArrayNextIndex((Array *) (a), sizeof( pid_t))
#define ProcArrayNextElement(a) (( pid_t *) ArrayNextElement((Array *) (a), sizeof( pid_t)))
#define ProcArrayIndex(a, e) ArrayIndex((Array *) (a), (char *) (e), sizeof( pid_t))
#define ProcArraySize(a) ArrayUsed((Array *) (a))
#define ProcArrayEmpty(a) ArrayEmpty((Array *) (a))
#define ProcArrayContains(a, d) ArrayContains((Array *) (a), ((char *) d), sizeof( pid_t))
#define ProcArrayInsert(a, i, d) ArrayInsert((Array *) (a), (unsigned int) (i), ((char *) d), sizeof( pid_t), (void (*)()) NULL)
#define ProcArrayInsertBefore(a, e, ele) ProcArrayInsert((a), ProcArrayIndex((a), ((char *) e)), (ele))
#define ProcArrayInsertAfter(a, e, ele) ProcArrayInsert((a), ProcArrayIndex((a), ((char *) e)) + 1, (ele))
#define ProcArrayDelete(a, i) ArrayDelete((Array *) (a), (i), sizeof( pid_t), (void (*)()) NULL)
#define ProcArrayDeleteElement(a, e) ProcArrayDelete((a), ProcArrayIndex((a), ((char *) e)))
#define ProcArrayAppend(a, e) ProcArrayInsert((a), ProcArraySize(a), (e))
#define ProcArraySort(a, p) ArraySort((Array *) (a), p, sizeof( pid_t))
#define ProcArrayIterPtr(a, e) for((e)=ProcArrayFirstElement(a);(e)<ProcArrayElement(a, ProcArraySize(a));(e)++)
#define ProcArrayIter(a, e, i) for((i)=0,(e)= *ProcArrayElement(a, i);(i)<ProcArraySize(a);(i)++,(e)= *ProcArrayElement(a, i))
#define ProcArrayIterFull(a, e, i, s, f, n) for((i)=(s),(e)= *ProcArrayElement(a, i);(i)!=(f);(n),(e)= *ProcArrayElement(a, i))
#define ProcArrayIterPtrFull(a, e, i, s, f, n) for((i)=(s),(e)=ProcArrayElement(a, i);(i)!=(f);(n),(e)=ProcArrayElement(a, i))
#define ProcArrayCopyInsert(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 1, sizeof( pid_t), (void (*)()) NULL)
#define ProcArrayCopyOver(fa, fi, ta, ti) \
	(void) ArrayCopy((Array *) fa, (unsigned int) fi, (Array *) ta, (unsigned int) ti, 0, sizeof( pid_t), (void (*)()) NULL)

