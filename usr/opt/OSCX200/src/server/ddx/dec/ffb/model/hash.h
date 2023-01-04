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
 * @(#)$RCSfile: hash.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/19 21:39:52 $
 */
typedef int PROC();

typedef struct HASHENTRY {
  char	*Symbol;
  char	*Data;
  struct HASHENTRY *Next;
} HASHENTRY;

typedef struct {
  int	  size;
  HASHENTRY **table;
  PROC	  *hashProc;
  PROC	  *compareProc;
} HASHTAB;

/*
 * create a new hashtable with at least n-entries,
 * and return a pointer to it.
 */
extern HASHTAB *NewHashTable(PROC *hashProc, PROC *compareProc, int n);

/*
 * removes a key/value pair from hashtab.
 */
extern void Delete(char *s, HASHTAB *phashtab);

/*
 * removes all entries from hashtab.
 */
extern void Clear(HASHTAB *phashtab);

/*
 * calls "(*proc)(p->Data)" once for each entry in hashtab.
 */
extern void Enumerate(PROC *proc, HASHTAB *phashtab);

/*
 * lookup the string s using hashtab.  if found, return a
 * pointer to the entry.  else return NULL.
 */
extern HASHENTRY *Lookup(char *s, HASHTAB *phashtab);

/*
 * returns TRUE if s is found in hashtab, else FALSE.
 */
extern int Find(char *s, HASHTAB *phashtab);

/*
 * enter s into hashtab.  if s already exists in hashtab,
 * substitute new data.
 */
extern void Enter(char *s, char *data, HASHTAB *phashtab);

/*
 * enter s into hashtab if it doesn't already exist.
 * return a pointer to the entry found or entered.
 */
extern HASHENTRY *FindOrEnter(char *s, char *data, HASHTAB *phashtab);

/*
 * return a computed hash value for the string s.
 */
extern int StringHash(char *s, int hashsiz);

/*
 * allocate an entry and init it with pnext and a copy of s.
 */
extern HASHENTRY *NewEntry(char *s, char *data, HASHENTRY *pnext);

/*
 * returns the data for key s.
 * s MUST be in hashtab.
 */
extern char *GetData(char *s, HASHTAB *phashtab);
