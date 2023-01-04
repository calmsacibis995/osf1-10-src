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
static char *rcsid = "@(#)$RCSfile: array.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/11/05 17:48:09 $";
#endif
/*LINTLIBRARY*/

#include <stdio.h>
#include "array.h"
#include "util.h"

extern void bcopy();

#define NUM_ELEMENTS 4

Array *ArrayNew(ptr, size)
Array *ptr;
unsigned int size;
{
  Array *ret;

  if(ptr == NULL){
    NEW_TYPE(ret, 1, Array, Array *, "ArrayNew");
  }
  else ret = ptr;
  ArrayAlloced(ret) = NUM_ELEMENTS;
  ArrayUsed(ret) = 0;
  NEW_TYPE(ArrayElements(ret), size * NUM_ELEMENTS, char, char *, "ArrayNew");
  return(ret);
}

void ArrayDestroy(ptr)
Array *ptr;
{
  free((void *) ArrayElements(ptr));
  free((void *) ptr);
}

void ArraySetElement(array, ind, data, size)
Array *array;
unsigned int ind;
char *data;
unsigned int size;
{
  unsigned int num;

  if(ind >= ArrayAlloced(array)){
    if(ind < 2 * ArrayAlloced(array)) num = 2 * ArrayAlloced(array);
    else num = ind + 1;
    ArrayElements(array) = 
      (char *) realloc((void *) ArrayElements(array),
		       num * sizeof(char) * size);
    ArrayAlloced(array) = num;
  }
  bcopy(data, ArrayElement(array, ind * size), (int) size);
  if(ind >= ArrayUsed(array)) ArrayUsed(array) = ind + 1;
}

unsigned int ArrayNextIndex(array, size)
Array *array;
unsigned int size;
{
  unsigned int num;

  if(ArrayUsed(array) == ArrayAlloced(array)){
    num = 2 * ArrayAlloced(array);
    ArrayElements(array) = 
      (char *) realloc((void *) ArrayElements(array),
		       num * sizeof(char) * size);
    ArrayAlloced(array) = num;
  }
  return(ArrayUsed(array)++);
}

char *ArrayNextElement(array, size)
Array *array;
unsigned int size;
{
  return(ArrayElement(array, ArrayNextIndex(array, size) * size));
}
#ifdef notdef
unsigned int ArrayIndex(array, element, size)
Array *array;
char *element;
unsigned int size;
{
  return((element - ArrayElements(array))/size);
}
#endif

void ArrayEmpty(array)
Array *array;
{
  ArrayUsed(array) = 0;
}

Boolean ArrayContains(array, data, size)
Array *array;
char *data;
unsigned int size;
{
  int i;

  for(i=0;i<ArrayUsed(array);i++){
    if(!bcmp(data, ArrayElement(array, i * size), (int) size)) return(True);
  }
  return(False);
}

void ArrayInsert(array, ind, data, size, proc)
Array *array;
unsigned int ind;
char *data;
unsigned int size;
void (*proc)();
{
  unsigned int num, i;

  if(ArrayUsed(array) == ArrayAlloced(array)){
    num = 2 * ArrayAlloced(array);
    ArrayElements(array) = 
      (char *) realloc((void *) ArrayElements(array),
		       num * sizeof(char) * size);
    ArrayAlloced(array) = num;
  }
  bcopy(ArrayElement(array, ind * size), 
	ArrayElement(array, (ind + 1) * size),
	(int) (size * (ArrayUsed(array) - ind)));
  bcopy(data, ArrayElement(array, ind * size), (int) size);
  if(ArrayUsed(array) > ind) ArrayUsed(array)++;
  else ArrayUsed(array) = ind + 1;
  if(proc){
    for(i=ArrayUsed(array)-1;i>ind;i--){
      (*proc)(array, i - 1, array, i, ArrayElement(array, i * size));
    }
  }
}

void ArrayDelete(array, ind, size, proc)
Array *array;
unsigned int ind;
unsigned int size;
void (*proc)();
{
  int i;

  if(ind < ArrayUsed(array)){
    bcopy(ArrayElement(array, (ind + 1) * size), 
	  ArrayElement(array, ind * size),
	  (int) (size * (ArrayUsed(array) - ind - 1)));
  }
  ArrayUsed(array)--;
  if(proc){
    for(i=ind;i<ArrayUsed(array);i++){
      (*proc)(array, i + 1, array, i, ArrayElement(array, i * size));
    }
  }
}

void ArraySort(array, sort_proc, size)
Array *array;
int (*sort_proc)();
unsigned int size;
{
  qsort((void *) ArrayElements(array), ArrayUsed(array), (size_t) size,
	sort_proc);
}

void ArrayCopy(from_array, from_index, to_array, to_index, insert, size, proc)
Array *from_array;
unsigned int from_index;
Array *to_array;
unsigned int to_index;
Boolean insert;
unsigned int size;
void (*proc)();
{
  if(insert)
    ArrayInsert(to_array, to_index,
		ArrayElement(from_array, from_index * size), size, proc);
  else bcopy(ArrayElement(from_array, from_index * size),
	     ArrayElement(to_array, to_index * size), (int) size);
  if(proc){
    (*proc)(from_array, from_index, to_array, to_index,
	    ArrayElement(to_array, to_index * size));
  }
}
