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
static char	*sccsid = "@(#)$RCSfile: release_db.c,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/01/30 09:02:31 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#include <sys/param.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/errno.h>
extern int errno;
#include <stdio.h>
#include <varargs.h>
#ifndef	USE_REFERENCE_COUNTING
#define	USE_REFERENCE_COUNTING	1
#endif	/* USE_REFERENCE_COUNTING */
#include "release_db.h"

#ifdef	lint
char _argbreak;
#else	lint
extern char _argbreak;
#endif	lint

#ifndef	FALSE
#define FALSE	0
#endif
#ifndef	TRUE
#define TRUE	1
#endif

extern char *malloc();
extern char *concat();
extern char *nxtarg();
extern char *getenv();
extern char *ttyname();

extern int debug;

#if	USE_REFERENCE_COUNTING
#define	INCREF(x)	\
{ \
    (x)->refcnt++; \
}
#define	DECREF(x)	\
{ \
    if ((x)->refcnt == 0) { \
	diag("bad refcnt"); \
	abort(); \
    } \
    (x)->refcnt--; \
}
#else	/* USE_REFERENCE_COUNTING */
#define	INCREF(x)
#define	DECREF(x)
#endif	/* USE_REFERENCE_COUNTING */

/* hash algorithm constants */
#define HASHBITS 8		/* bits used in hash function */
#define HASHSIZE (1<<HASHBITS)	/* range of hash function */
#define HASHMASK (HASHSIZE-1)	/* mask for hash function */

struct table string_table;
struct table file_info_table;
struct table file_table;
struct table dir_table;
struct strhdr string_hashtable[HASHSIZE];

int lock_fd = -1;		/* database lock descriptor */

/*
 * Binsearch does a binary search on the array.  It returns either the index
 * of the matching element, the index of the largest element less than the
 * key, if such an element exists, otherwise returns -1 if not.  The caller
 * must use the compar function to determine if the key was found or not
 * when the index returned is non-negative.
 */
int 
binsearch(base, nel, width, key, compar)
char *base;			/* start of the array to search */
int nel;			/* size of the array */
int width;			/* size of an element of the array */
char *key;			/* pointer to a key to find */
int (*compar)();		/* comparison routine like in qsort */
{
    int     l = 0;
    int     u = nel - 1;
    int     m;
    int     r;

    /* Invariant: key > all elements in [0..l) and key < all elements in
       (u..nel-1]. */
    while (l <= u) {
	m = (l + u) / 2;
	r = (*compar)(key, base + (m * width));
	if (r == 0)
	    return m;
	else if (r < 0)
	    u = m - 1;
	else
	    l = m + 1;
    }
    return u;			/* Which should == l - 1 */
}

cmpstring(s, len, xhash, str)
register struct string *s;
register int len, xhash;
register char *str;
{
    if (s->len != len)
	return(s->len - len);
    if (s->xhash != xhash)
	return(s->xhash - xhash);
    return(bcmp(s->data, str, len));
}

struct strhdr *
prevstring(sdata, slen, ahash, xhash, sptr)
register char *sdata;
register int slen;
register int ahash;
register int xhash;
struct string **sptr;
{
    register struct strhdr *s, *sp;
    register int cmp;

    sp = string_hashtable[ahash].sh_last;
    if (sp == sp->sh_prev) {
	if (sptr)
	    *sptr = NULL;
	return(sp);
    }
    cmp = cmpstring((struct string *)sp, slen, xhash, sdata);
    if (cmp == 0) {
	if (sptr != NULL) {
	    *sptr = (struct string *)sp;
	    return(sp->sh_prev);
	} else
	    return(NULL);
    }
    if (cmp < 0) {
	if (sptr)
	    *sptr = NULL;
	return(sp);
    }
    for (s = string_hashtable[ahash].sh_first; s != sp; s = s->sh_next) {
	cmp = cmpstring((struct string *)s, slen, xhash, sdata);
	if (cmp == 0) {
	    if (sptr != NULL) {
		*sptr = (struct string *)s;
		return(s->sh_prev);
	    } else
		return(NULL);
	}
	if (cmp > 0)
	    break;
    }
    if (sptr)
	*sptr = NULL;
    return(s->sh_prev);
}

linkstring(s, sp)
register struct strhdr *s;
register struct strhdr *sp;
{
    s->sh_next = sp->sh_next;
    s->sh_prev = sp;
    sp->sh_next->sh_prev = s;
    sp->sh_next = s;
    return(0);
}

hashstring(ptr, ahashp, xhashp)
register char *ptr;
int *ahashp, *xhashp;
{
    register int ahash = 0, xhash = 0;
    register char *p;

    p = ptr;
    while (*p != '\0') {
	ahash = (ahash + *p)&HASHMASK;
	xhash ^= ((int)*p++)&HASHMASK;
    }
    *ahashp = ahash;
    *xhashp = xhash;
    return(p - ptr);
}

create_string(ptr, string_ptr_ptr)
register char *ptr;
register struct string **string_ptr_ptr;
{
    struct string *s;
    struct strhdr *sp;
    int len, ahash, xhash;

    *string_ptr_ptr = NULL;
    len = hashstring(ptr, &ahash, &xhash);
    sp = prevstring(ptr, len, ahash, xhash, &s);
    if (s != NULL) {
	*string_ptr_ptr = s;
	return(0);
    }
    if ((s = (struct string *)calloc(1, sizeof(struct string))) == NULL) {
	warn("calloc failed");
	return(1);
    }
    if ((s->data = malloc((unsigned)len + 1)) == NULL) {
	warn("malloc failed");
	return(1);
    }
    bcopy(ptr, s->data, len);
    s->data[len] = '\0';
    s->len = len;
    s->ahash = ahash;
    s->xhash = xhash;
    if (linkstring((struct strhdr *)s, sp) < 0) {
	warn("cannot link string in create_string");
	return(1);
    }
    s->index = string_table.cur++;
    if (string_table.cur > string_table.max &&
	alloc_table(&string_table, 128) < 0)
	return(1);
    string_table.stab[s->index] = s;
    INCREF(s);
    *string_ptr_ptr = s;
    return(0);
}

cmp_string(string1, string2)
struct string *string1, *string2;
{
    if (string1->ahash != string2->ahash)
	return(string1->ahash - string2->ahash);
    if (string1->len != string2->len)
	return(string1->len - string2->len);
    if (string1->xhash != string2->xhash)
	return(string1->xhash - string2->xhash);
    return(bcmp(string1->data, string2->data, (int)string2->len));
}

alloc_table(table_ptr, initial_size)
register struct table *table_ptr;
{
    if (table_ptr->max == 0) {
	table_ptr->max = initial_size;
	table_ptr->tab_u.u_tab =
	    (char **)malloc(table_ptr->max*sizeof(char *));
    } else {
	table_ptr->max <<= 1;
	table_ptr->tab_u.u_tab =
	    (char **)realloc((char *)table_ptr->tab_u.u_tab,
			     table_ptr->max*sizeof(char *));
    }
    if (table_ptr->tab_u.u_tab == NULL) {
	warn("table malloc/realloc failed");
	return(-1);
    }
    return(0);
}

check_dir_name(dir_name, dir_ptr)
register struct string *dir_name;
register struct dir **dir_ptr;
{
    return(cmp_string(dir_name, (*dir_ptr)->name));
}

check_file_name(file_name, file_ptr)
register struct string *file_name;
register struct file **file_ptr;
{
    return(cmp_string(file_name, (*file_ptr)->name));
}

create_dir(dir_name, dir_ptr_ptr)
register struct string *dir_name;
struct dir **dir_ptr_ptr;
{
    struct dir *dir_ptr;

    dir_ptr = (struct dir *) calloc(1, sizeof(struct dir));
    if (dir_ptr == NULL) {
	warn("calloc failed");
	return(-1);
    }
    dir_ptr->name = dir_name;
    INCREF(dir_name);
    dir_ptr->index = dir_table.cur++;
    if (dir_table.cur > dir_table.max && alloc_table(&dir_table, 64) < 0)
	return(-1);
    dir_table.dtab[dir_ptr->index] = dir_ptr;
    INCREF(dir_ptr);
    *dir_ptr_ptr = dir_ptr;
    return(0);
}

create_file(file_name, file_ptr_ptr)
register struct string *file_name;
struct file **file_ptr_ptr;
{
    struct file *file_ptr;

    file_ptr = (struct file *) calloc(1, sizeof(struct file));
    if (file_ptr == NULL) {
	warn("calloc failed");
	return(-1);
    }
    file_ptr->name = file_name;
    INCREF(file_name);
    file_ptr->index = file_table.cur++;
    if (file_table.cur > file_table.max && alloc_table(&file_table, 64) < 0)
	return(-1);
    file_table.ftab[file_ptr->index] = file_ptr;
    INCREF(file_ptr);
    *file_ptr_ptr = file_ptr;
    return(0);
}

insert_file(filename, file_ptr_ptr, dir_ptr_ptr, file_info_ptr_ptr)
char *filename;
struct file **file_ptr_ptr;
struct dir **dir_ptr_ptr;
struct file_info **file_info_ptr_ptr;
{
    char file[BUFSIZ];
    char *p;
    char *part;
    struct string *part_string;
    struct file *new_file_ptr;
    struct dir *dir_ptr;
    struct dir *new_dir_ptr;
    struct file_info *file_info_ptr;
    int isdir;
    int i;

    dir_ptr = dir_table.dtab[0];
    file_info_ptr = dir_ptr->info;
    if (file_ptr_ptr != NULL)
	*file_ptr_ptr = NULL;
    if (dir_ptr_ptr != NULL)
	*dir_ptr_ptr = NULL;
    if (file_info_ptr_ptr != NULL)
	*file_info_ptr_ptr = NULL;
    p = concat(file, sizeof(file), filename, NULL);
    if (p == NULL) {
	warn("No buffer space for file '%s'", filename);
	return(-1);
    }
    if (*--p == '/') {
	if (dir_ptr_ptr == NULL) {
	    warn("Expected file, not directory '%s'", filename);
	    return(-1);
	}
	isdir = TRUE;
	*p = '\0';
    } else {
	if (file_ptr_ptr == NULL) {
	    warn("Expected directory, not file '%s'", filename);
	    return(-1);
	}
	isdir = FALSE;
    }
    p = file;
    part = nxtarg(&p, "/");
    if (*part != '\0') {
	warn("Path '%s' is not absolute", filename);
	return(-1);
    }
    if (_argbreak == '\0') {
	if (!isdir) {
	    warn("Must refer to root as '/' not ''");
	    return(-1);
	}
	if (dir_ptr_ptr != NULL)
	    *dir_ptr_ptr = dir_ptr;
	if (file_info_ptr_ptr != NULL)
	    *file_info_ptr_ptr = file_info_ptr;
	return(0);
    }
    for (;;) {
	part = nxtarg(&p, "/");
	if (create_string(part, &part_string) != 0) {
	    warn("Unable to create string '%s'", part);
	    return(-1);
	}
	if (_argbreak == '/' || isdir) {
	    if (dir_ptr->dtable.cur != 0) {
		i = binsearch((char *)dir_ptr->dtable.dtab,
			      dir_ptr->dtable.cur,
			      sizeof(struct dir *),
			      part_string, check_dir_name);
		if (i++ != -1 &&
		    check_dir_name(part_string,
				   &dir_ptr->dtable.dtab[i-1]) == 0) {
		    dir_ptr = dir_ptr->dtable.dtab[i-1];
		    if (dir_ptr->info != NULL)
			file_info_ptr = dir_ptr->info;
		    if (_argbreak == '/')
			continue;
		    if (dir_ptr_ptr != NULL)
			*dir_ptr_ptr = dir_ptr;
		    if (file_info_ptr_ptr != NULL)
			*file_info_ptr_ptr = file_info_ptr;
		    return(0);
		}
	    } else
		i = 0;
	    if (create_dir(part_string, &new_dir_ptr) < 0)
		return(-1);
	    new_dir_ptr->parent = dir_ptr;
	    INCREF(dir_ptr);
	    if (dir_ptr->dtable.cur >= dir_ptr->dtable.max) {
		if (alloc_table(&dir_ptr->dtable, 4) < 0)
		    return(-1);
		bzero((char *)&dir_ptr->dtable.dtab[dir_ptr->dtable.cur],
		      (int) (dir_ptr->dtable.max - dir_ptr->dtable.cur) *
		      sizeof(struct dir *));
	    }
	    if (dir_ptr->dtable.cur != i)
		(void) bcopy((char *)&dir_ptr->dtable.dtab[i],
			     (char *)&dir_ptr->dtable.dtab[i+1],
			     (int) (dir_ptr->dtable.cur - i) *
			     sizeof(struct dir *));
	    dir_ptr->dtable.dtab[i] = new_dir_ptr;
	    INCREF(new_dir_ptr);
	    dir_ptr->dtable.cur++;
	    dir_ptr = new_dir_ptr;
	    if (_argbreak == '/')
		continue;
	    if (dir_ptr_ptr != NULL)
		*dir_ptr_ptr = dir_ptr;
	    if (file_info_ptr_ptr != NULL)
		*file_info_ptr_ptr = file_info_ptr;
	    return(0);
	} else {
	    if (dir_ptr->ftable.cur != 0) {
		i = binsearch((char *)dir_ptr->ftable.ftab,
			      dir_ptr->ftable.cur,
			      sizeof(struct file *),
			      part_string, check_file_name);
		if (i++ != -1 &&
		    check_file_name(part_string, (struct file **)
				    &dir_ptr->ftable.ftab[i-1]) == 0) {
		    new_file_ptr = dir_ptr->ftable.ftab[i-1];
		    if (new_file_ptr->info != NULL ||
#if	USE_REFERENCE_COUNTING
			new_file_ptr->refcnt > 1 ||
#endif	USE_REFERENCE_COUNTING
			new_file_ptr->srcdir != NULL) {
			warn("File '%s' already in database", filename);
			return(-1);
		    }
		    if (file_ptr_ptr != NULL)
			*file_ptr_ptr = new_file_ptr;
		    if (file_info_ptr_ptr != NULL)
			*file_info_ptr_ptr = file_info_ptr;
		    return(0);
		}
	    } else
		i = 0;
	    if (create_file(part_string, &new_file_ptr) < 0)
		return(-1);
	    new_file_ptr->parent = dir_ptr;
	    INCREF(dir_ptr);
	    if (dir_ptr->ftable.cur >= dir_ptr->ftable.max) {
		if (alloc_table(&dir_ptr->ftable, 4) < 0)
		    return(-1);
		bzero((char *)&dir_ptr->ftable.ftab[dir_ptr->ftable.cur],
		      (int) (dir_ptr->ftable.max - dir_ptr->ftable.cur) *
		      sizeof(struct file *));
	    }
	    if (dir_ptr->ftable.cur != i)
		(void) bcopy((char *)&dir_ptr->ftable.ftab[i],
			     (char *)&dir_ptr->ftable.ftab[i+1],
			     (int) (dir_ptr->ftable.cur - i) *
			     sizeof(struct file *));
	    dir_ptr->ftable.ftab[i] = new_file_ptr;
	    INCREF(new_file_ptr);
	    dir_ptr->ftable.cur++;
	    if (file_ptr_ptr != NULL)
		*file_ptr_ptr = new_file_ptr;
	    if (file_info_ptr_ptr != NULL)
		*file_info_ptr_ptr = file_info_ptr;
	    return(0);
	}
    }
}

insert_info(owner, group, mode, file_info_ptr_ptr)
struct string *owner, *group;
u_short mode;
struct file_info **file_info_ptr_ptr;
{
    struct file_info *fi;
    int i;

    for (i = 0; i < file_info_table.cur; i++) {
	fi = file_info_table.fitab[i];
	if (fi->owner == owner && fi->group == group && fi->mode == mode) {
	    *file_info_ptr_ptr = fi;
	    return(0);
	}
    }
    fi = (struct file_info *) calloc(1, sizeof(struct file_info));
    if (fi == NULL) {
	ewarn("calloc failed");
	return(-1);
    }
    fi->owner = owner;
    INCREF(owner);
    fi->group = group;
    INCREF(group);
    fi->mode = mode;
    fi->index = file_info_table.cur++;
    if (file_info_table.cur > file_info_table.max &&
	alloc_table(&file_info_table, 32) < 0)
	return(-1);
    file_info_table.fitab[fi->index] = fi;
    INCREF(fi);
    *file_info_ptr_ptr = fi;
    return(0);
}

read_database_description(db_path)
char *db_path;
{
    FILE *fp;
    int lineno;
    int errors;
    int value;
    char line[BUFSIZ];
    char *p;
    char *files, *file, *owner, *group, *perm, *srcdir;
    struct string *owner_ptr, *o;
    struct string *group_ptr, *g;
    u_short mode, m;
    struct file *file_ptr;
    struct file *prev_file_ptr;
    struct dir *dir_ptr;
    struct file_info *file_info_ptr;
    struct string *null_string;

    if (create_string("", &null_string) != 0)
	return(-1);
    if (create_dir(null_string, &dir_ptr) < 0)
	return(-1);
    if (dir_ptr->index != 0) {
	warn("root directory not index 0");
	return(-1);
    }
    dir_ptr->parent = dir_ptr;
    INCREF(dir_ptr);
    if ((fp = fopen(db_path, "r")) == NULL) {
	ewarn("fopen %s", db_path);
	return(-1);
    }
    lineno = 0;
    errors = 0;
    while (fgets(line, sizeof (line), fp) != NULL) {
	lineno++;
	if (index("#\n", line[0]) != NULL)
	    continue;
	p = line;
	files = nxtarg(&p, ":");
	if (_argbreak == '\0') {
	    warn("line %d: Missing ':' after files", lineno);
	    errors++;
	    continue;
	}
	owner = nxtarg(&p, ":");
	if (_argbreak == '\0') {
	    warn("line %d: Missing ':' after owner", lineno);
	    errors++;
	    continue;
	}
	if (*owner == '\0')
	    owner_ptr = NULL;
	else if (create_string(owner, &owner_ptr) != 0) {
	    warn("line %d: Unable to create owner string '%s'", lineno, owner);
	    (void) fclose(fp);
	    return(-1);
	}
	group = nxtarg(&p, ":");
	if (_argbreak == '\0') {
	    warn("line %d: Missing ':' after group", lineno);
	    errors++;
	    continue;
	}
	if (*group == '\0')
	    group_ptr = NULL;
	else if (create_string(group, &group_ptr) != 0) {
	    warn("line %d: Unable to create group string '%s'", lineno, group);
	    (void) fclose(fp);
	    return(-1);
	}
	perm = nxtarg(&p, ":");
	if (_argbreak == '\0') {
	    warn("line %d: Missing ':' after mode", lineno);
	    errors++;
	    continue;
	}
	if (*perm == '\0')
	    mode = (u_short)-1;
	else if ((value = atoo(perm)) == -1) {
	    warn("line %d: Unable to convert mode '%s'", lineno, perm);
	    (void) fclose(fp);
	    return(-1);
	} else
	    mode = (u_short)value&07777;
	srcdir = nxtarg(&p, ":");
	if (_argbreak == '\0') {
	    warn("line %d: Missing ':' after srcdir", lineno);
	    errors++;
	    continue;
	}
	if (*srcdir == '\0')
	    srcdir = NULL;
	file = nxtarg(&files, " ");
	if (*file == '\0') {
	    warn("line %d: Empty file list", lineno);
	    errors++;
	    continue;
	}
	prev_file_ptr = NULL;
	for (;;) {
	    if (insert_file(file, &file_ptr, &dir_ptr, &file_info_ptr) < 0) {
		warn("line %d: Error inserting file '%s'", lineno, file);
		(void) fclose(fp);
		return(-1);
	    }
	    if (dir_ptr != NULL) {
		if (prev_file_ptr != NULL) {
		    warn("line %d: Illegal to have file and directory links",
			 lineno);
		    (void) fclose(fp);
		    return(-1);
		}
		if (srcdir != NULL) {
		    warn("line %d: Illegal to have srcdir for directory",
			 lineno);
		    (void) fclose(fp);
		    return(-1);
		}
	    } else if (prev_file_ptr != NULL) {
		if (prev_file_ptr->links != NULL)
		    file_ptr->links = prev_file_ptr->links;
		else {
		    file_ptr->links = prev_file_ptr;
		    INCREF(file_ptr->links);
		}
		prev_file_ptr->links = file_ptr;
		if (file_ptr != NULL)
		    INCREF(file_ptr);
	    } else {
		if (srcdir == NULL) {
		    file_ptr->srcdir = file_ptr->parent;
		    INCREF(file_ptr->parent);
		} else {
		    if (insert_file(srcdir, (struct file **)NULL,
				    &dir_ptr, (struct file_info **)NULL) < 0) {
			warn("line %d: Error inserting srcdir '%s'",
			     lineno, file);
			(void) fclose(fp);
			return(-1);
		    }
		    file_ptr->srcdir = dir_ptr;
		    if (dir_ptr != NULL)
			INCREF(dir_ptr);
		    dir_ptr = NULL;
		}
	    }
	    if (owner_ptr == NULL && group_ptr == NULL &&
		mode == (u_short)-1) {
		if (file_info_ptr == NULL) {
		    warn("No defaults available");
		    (void) fclose(fp);
		    return(-1);
		}
	    } else if (file_info_ptr == NULL) {
		if (owner_ptr == NULL || group_ptr == NULL ||
		    mode == (u_short)-1) {
		    warn("Incomplete defaults available");
		    (void) fclose(fp);
		    return(-1);
		}
		if (insert_info(owner_ptr, group_ptr, mode,
				&file_info_ptr) < 0) {
		    warn("line %d: Error inserting '%s' info", lineno, file);
		    (void) fclose(fp);
		    return(-1);
		}
		if (dir_ptr != NULL)
		    dir_ptr->info = file_info_ptr;
		else
		    file_ptr->info = file_info_ptr;
		if (file_info_ptr != NULL)
		    INCREF(file_info_ptr);
	    } else if ((owner_ptr != NULL &&
			file_info_ptr->owner != owner_ptr) ||
		       (group_ptr != NULL &&
			file_info_ptr->group != group_ptr) ||
		       (mode != (u_short)-1 &&
			file_info_ptr->mode != mode)) {
		if (owner_ptr == NULL)
		    o = file_info_ptr->owner;
		else
		    o = owner_ptr;
		if (group_ptr == NULL)
		    g = file_info_ptr->group;
		else
		    g = group_ptr;
		if (mode == (u_short)-1)
		    m = file_info_ptr->mode;
		else
		    m = mode;
		if (insert_info(o, g, m, &file_info_ptr) < 0) {
		    warn("line %d: Error inserting '%s' info", lineno, file);
		    (void) fclose(fp);
		    return(-1);
		}
		if (dir_ptr != NULL)
		    dir_ptr->info = file_info_ptr;
		else
		    file_ptr->info = file_info_ptr;
		if (file_info_ptr != NULL)
		    INCREF(file_info_ptr);
	    } else if (dir_ptr != NULL) {
		warn("line %d: Redundant directory info", lineno);
	    } else
		warn("line %d: Redundant file info", lineno);
	    file = nxtarg(&files, " ");
	    if (*file == '\0')
		break;
	    if (dir_ptr != NULL) {
		warn("line %d: Illegal to have directory links", lineno);
		(void) fclose(fp);
		return(-1);
	    }
	    prev_file_ptr = file_ptr;
	}
    }
    (void) fclose(fp);
    return(errors);
}

subdirectory_path(dir_ptr, buf)
struct dir *dir_ptr;
char *buf;
{
    if (dir_ptr->index != 0) {
	subdirectory_path(dir_ptr->parent, buf);
	(void) strcat(buf, dir_ptr->name->data);
    }
    (void) strcat(buf, "/");
}

directory_path(dir_ptr, buf)
struct dir *dir_ptr;
char *buf;
{
    buf[0] = '\0';
    if (dir_ptr->index != 0) {
	subdirectory_path(dir_ptr->parent, buf);
	(void) strcat(buf, dir_ptr->name->data);
    }
    if (buf[0] == '\0')
	(void) strcat(buf, "/");
}

component_path(file_ptr, buf)
struct file *file_ptr;
char *buf;
{
    buf[0] = '\0';
    subdirectory_path(file_ptr->parent, buf);
    (void) strcat(buf, file_ptr->name->data);
}

print_dir_path(fp, dir_ptr)
FILE *fp;
struct dir *dir_ptr;
{
    if (dir_ptr->index != 0) {
	print_dir_path(fp, dir_ptr->parent);
	(void) fputs(dir_ptr->name->data, fp);
    }
    (void) putc('/', fp);
}

print_file_path(fp, file_ptr)
FILE *fp;
struct file *file_ptr;
{
    print_dir_path(fp, file_ptr->parent);
    (void) fputs(file_ptr->name->data, fp);
}

write_file(fp, file_ptr, info, path, endp)
FILE *fp;
struct file *file_ptr;
struct file_info *info;
char *path, *endp;
{
    char *p;
    struct file *f;

    if (file_ptr->links != NULL && file_ptr->srcdir == NULL)
	return;
    for (p = file_ptr->name->data; *p != '\0'; p++)
	*endp++ = *p;
    *endp = '\0';
    (void) fputs(path, fp);
    if ((f = file_ptr->links) != NULL)
	while (f != file_ptr) {
	    (void) putc(' ', fp);
	    print_file_path(fp, f);
	    f = f->links;
	}
    (void) putc(':', fp);
    if (info == NULL ||
	(file_ptr->info && file_ptr->info->owner != info->owner))
	(void) fputs(file_ptr->info->owner->data, fp);
    (void) putc(':', fp);
    if (info == NULL ||
	(file_ptr->info && file_ptr->info->group != info->group))
	(void) fputs(file_ptr->info->group->data, fp);
    (void) putc(':', fp);
    if (info == NULL ||
	(file_ptr->info && file_ptr->info->mode != info->mode))
	(void) fprintf(fp, "%o", file_ptr->info->mode);
    (void) putc(':', fp);
    if (file_ptr->srcdir != file_ptr->parent)
	print_dir_path(fp, file_ptr->srcdir);
    (void) putc(':', fp);
    (void) putc('\n', fp);
}

write_dir(fp, dir_ptr, info, path, endp)
FILE *fp;
struct dir *dir_ptr;
struct file_info *info;
char *path, *endp;
{
    int i;
    char *p;

    for (p = dir_ptr->name->data; *p != '\0'; p++)
	*endp++ = *p;
    *endp++ = '/';
    if (dir_ptr->info) {
	*endp = '\0';
	(void) fputs(path, fp);
	(void) putc(':', fp);
	if (info == NULL ||
	    (dir_ptr->info && dir_ptr->info->owner != info->owner))
	    (void) fputs(dir_ptr->info->owner->data, fp);
	(void) putc(':', fp);
	if (info == NULL ||
	    (dir_ptr->info && dir_ptr->info->group != info->group))
	    (void) fputs(dir_ptr->info->group->data, fp);
	(void) putc(':', fp);
	if (info == NULL ||
	    (dir_ptr->info && dir_ptr->info->mode != info->mode))
	    (void) fprintf(fp, "%o", dir_ptr->info->mode);
	(void) putc(':', fp);
	(void) putc(':', fp);
	(void) putc('\n', fp);
	info = dir_ptr->info;
    }
    for (i = 0; i < dir_ptr->ftable.cur; i++)
	write_file(fp, dir_ptr->ftable.ftab[i],
		   info, path, endp);
    for (i = 0; i < dir_ptr->dtable.cur; i++)
	write_dir(fp, dir_ptr->dtable.dtab[i],
		  info, path, endp);
}

write_database_description(db_path)
char *db_path;
{
    FILE *fp;
    int errors;
    char path[BUFSIZ];

    if (lock_fd == -1) {
	warn("Database must be locked during modifications");
	return(-1);
    }
    if ((fp = fopen(db_path, "w")) == NULL) {
	ewarn("fopen %s", db_path);
	return(-1);
    }
    errors = 0;
    path[0] = '\0';
    write_dir(fp, dir_table.dtab[0],
	      (struct file_info *)NULL, path, path);
    (void) fclose(fp);
    return(errors);
}

restore_strings(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct string *s;
    struct strhdr *sh;
    int ahash, xhash;
    int i;

    for (i = 0; i < string_table.cur; i++) {
	s = string_table.stab[i];
	INCREF(s);
	s->index = i;
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	s->len = _getshort(buf);
	if ((s->data = malloc(s->len + 1)) == NULL) {
	    warn("malloc failed");
	    return(-1);
	}
	if (fread(s->data, sizeof(char), (int)s->len, fp) != s->len) {
	    ewarn("fread");
	    return(-1);
	}
	(void)hashstring(s->data, &ahash, &xhash);
	s->ahash = ahash;
	s->xhash = xhash;
	sh = prevstring(s->data, (int)s->len, (int)s->ahash, (int)s->xhash,
			(struct string **)NULL);
	if (sh == NULL) {
	    warn("duplicate string found in string table");
	    return(-1);
	}
	if (linkstring((struct strhdr *)s, sh) < 0) {
	    warn("linkstring failed");
	    return(-1);
	}
    }
    return(0);
}

restore_file_info(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct file_info *fi;
    int i;
    short index;

    for (i = 0; i < file_info_table.cur; i++) {
	fi = file_info_table.fitab[i];
	INCREF(fi);
	fi->index = i;
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	fi->owner = string_table.stab[index];
	INCREF(fi->owner);
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	fi->group = string_table.stab[index];
	INCREF(fi->group);
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	fi->mode = _getshort(buf);
    }
    return(0);
}

restore_files(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct file *f;
    int i;
    short index;

    for (i = 0; i < file_table.cur; i++) {
	f = file_table.ftab[i];
	INCREF(f);
	f->index = i;
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	f->parent = dir_table.dtab[index];
	INCREF(f->parent);
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	if (index == -1)
	    f->links = NULL;
	else {
	    f->links = file_table.ftab[index];
	    INCREF(f->links);
	}
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	if (index == -1)
	    f->srcdir = NULL;
	else {
	    f->srcdir = dir_table.dtab[index];
	    INCREF(f->srcdir);
	}
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	f->name = string_table.stab[index];
	INCREF(f->name);
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	if (index == -1)
	    f->info = NULL;
	else {
	    f->info = file_info_table.fitab[index];
	    INCREF(f->info);
	}
    }
    return(0);
}

restore_dirs(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct dir *d, *dp;
    struct file *f;
    int i, j;
    short index;

    for (i = 0; i < dir_table.cur; i++) {
	d = dir_table.dtab[i];
	INCREF(d);
	d->index = i;
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	d->parent = dir_table.dtab[index];
	INCREF(d->parent);
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	d->name = string_table.stab[index];
	INCREF(d->name);
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	index = _getshort(buf);
	if (index == -1)
	    d->info = NULL;
	else {
	    d->info = file_info_table.fitab[index];
	    INCREF(d->info);
	}
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	d->ftable.cur = _getshort(buf);
	for (d->ftable.max = 4;
	     d->ftable.max < d->ftable.cur;
	     d->ftable.max <<= 1);
	d->ftable.ftab =
	    (struct file **)calloc(d->ftable.max, sizeof(struct file *));
	if (d->ftable.ftab == NULL) {
	    warn("calloc failed");
	    return(-1);
	}
	for (j = 0; j < d->ftable.cur; j++) {
	    if (fread(buf, sizeof(short), 1, fp) != 1) {
		ewarn("fread");
		return(-1);
	    }
	    index = _getshort(buf);
	    f = file_table.ftab[index];
	    d->ftable.ftab[j] = f;
	    INCREF(f);
	}
	if (fread(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fread");
	    return(-1);
	}
	d->dtable.cur = _getshort(buf);
	for (d->dtable.max = 4;
	     d->dtable.max < d->dtable.cur;
	     d->dtable.max <<= 1);
	d->dtable.dtab =
	    (struct dir **)calloc(d->dtable.max, sizeof(struct dir *));
	if (d->dtable.dtab == NULL) {
	    warn("calloc failed");
	    return(-1);
	}
	for (j = 0; j < d->dtable.cur; j++) {
	    if (fread(buf, sizeof(short), 1, fp) != 1) {
		ewarn("fread");
		return(-1);
	    }
	    index = _getshort(buf);
	    dp = dir_table.dtab[index];
	    d->dtable.dtab[j] = dp;
	    INCREF(dp);
	}
    }
    return(0);
}

restore_database(db_path)
char *db_path;
{
    char buf[BUFSIZ];
    char *buffer;
    FILE *fp;
    int errors;
    int i;

    if ((fp = fopen(db_path, "r")) == NULL) {
	ewarn("fopen %s", db_path);
	return(-1);
    }
    if (fread(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fread");
	return(-1);
    }
    string_table.cur = _getshort(buf);
    if (fread(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fread");
	return(-1);
    }
    file_info_table.cur = _getshort(buf);
    if (fread(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fread");
	return(-1);
    }
    file_table.cur = _getshort(buf);
    if (fread(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fread");
	return(-1);
    }
    dir_table.cur = _getshort(buf);
    for (string_table.max = 128;
	 string_table.max < string_table.cur;
	 string_table.max <<= 1);
    string_table.stab =
	(struct string **)calloc(string_table.max, sizeof(struct string *));
    if (string_table.stab == NULL) {
	warn("calloc failed");
	return(-1);
    }
    for (file_info_table.max = 32;
	 file_info_table.max < file_info_table.cur;
	 file_info_table.max <<= 1);
    file_info_table.fitab =
	(struct file_info **)calloc(file_info_table.max,
				    sizeof(struct file_info *));
    if (file_info_table.fitab == NULL) {
	warn("calloc failed");
	return(-1);
    }
    for (file_table.max = 64;
	 file_table.max < file_table.cur;
	 file_table.max <<= 1);
    file_table.ftab =
	(struct file **)calloc(file_table.max, sizeof(struct file *));
    if (file_table.ftab == NULL) {
	warn("calloc failed");
	return(-1);
    }
    for (dir_table.max = 64;
	 dir_table.max < dir_table.cur;
	 dir_table.max <<= 1);
    dir_table.dtab =
	(struct dir **)calloc(dir_table.max, sizeof(struct dir *));
    if (dir_table.dtab == NULL) {
	warn("calloc failed");
	return(-1);
    }
    buffer = malloc(string_table.cur * sizeof(struct string));
    if (buffer == NULL) {
	warn("malloc failed");
	return(-1);
    }
    for (i = 0; i < string_table.cur; i++) {
	string_table.stab[i] = (struct string *)buffer;
	buffer += sizeof(struct string);
    }
    buffer = malloc(file_info_table.cur * sizeof(struct file_info));
    if (buffer == NULL) {
	warn("malloc failed");
	return(-1);
    }
    for (i = 0; i < file_info_table.cur; i++) {
	file_info_table.fitab[i] = (struct file_info *)buffer;
	buffer += sizeof(struct file_info);
    }
    buffer = malloc(file_table.cur * sizeof(struct file));
    if (buffer == NULL) {
	warn("malloc failed");
	return(-1);
    }
    for (i = 0; i < file_table.cur; i++) {
	file_table.ftab[i] = (struct file *)buffer;
	buffer += sizeof(struct file);
    }
    buffer = malloc(dir_table.cur * sizeof(struct dir));
    if (buffer == NULL) {
	warn("malloc failed");
	return(-1);
    }
    for (i = 0; i < dir_table.cur; i++) {
	dir_table.dtab[i] = (struct dir *)buffer;
	buffer += sizeof(struct dir);
    }
    errors = restore_strings(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d strings ]", string_table.cur);
    errors = restore_file_info(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d file_info ]", file_info_table.cur);
    errors = restore_files(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d files ]", file_table.cur);
    errors = restore_dirs(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d dirs ]", dir_table.cur);
    (void) fclose(fp);
    return(0);
}

save_strings(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct string *s;
    int i;

    for (i = 0; i < string_table.cur; i++) {
	s = string_table.stab[i];
	putshort(s->len, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	if (fwrite(s->data, sizeof(char), (int)s->len, fp) != s->len) {
	    ewarn("fwrite");
	    return(-1);
	}
    }
    return(0);
}

save_file_info(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct file_info *fi;
    int i;

    for (i = 0; i < file_info_table.cur; i++) {
	fi = file_info_table.fitab[i];
	putshort(fi->owner->index, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	putshort(fi->group->index, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	putshort(fi->mode, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
    }
    return(0);
}

save_files(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct file *f;
    int i;
    short short_val;

    for (i = 0; i < file_table.cur; i++) {
	f = file_table.ftab[i];
	putshort(f->parent->index, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	if (f->links == NULL)
	    short_val = -1;
	else
	    short_val = f->links->index;
	putshort((u_short)short_val, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	if (f->srcdir == NULL)
	    short_val = -1;
	else
	    short_val = f->srcdir->index;
	putshort((u_short)short_val, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	putshort(f->name->index, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	if (f->info == NULL)
	    short_val = -1;
	else
	    short_val = f->info->index;
	putshort((u_short)short_val, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
    }
    return(0);
}

save_dirs(fp)
FILE *fp;
{
    char buf[BUFSIZ];
    struct dir *d, *dp;
    struct file *f;
    int i, j;
    short short_val;

    for (i = 0; i < dir_table.cur; i++) {
	d = dir_table.dtab[i];
	putshort(d->parent->index, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	putshort(d->name->index, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	if (d->info == NULL)
	    short_val = -1;
	else
	    short_val = d->info->index;
	putshort((u_short)short_val, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	putshort(d->ftable.cur, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	for (j = 0; j < d->ftable.cur; j++) {
	    f = d->ftable.ftab[j];
	    putshort(f->index, buf);
	    if (fwrite(buf, sizeof(short), 1, fp) != 1) {
		ewarn("fwrite");
		return(-1);
	    }
	}
	putshort(d->dtable.cur, buf);
	if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	    ewarn("fwrite");
	    return(-1);
	}
	for (j = 0; j < d->dtable.cur; j++) {
	    dp = d->dtable.dtab[j];
	    putshort(dp->index, buf);
	    if (fwrite(buf, sizeof(short), 1, fp) != 1) {
		ewarn("fwrite");
		return(-1);
	    }
	}
    }
    return(0);
}

save_database(db_path)
char *db_path;
{
    FILE *fp;
    int errors;
    char buf[BUFSIZ];

    if (lock_fd == -1) {
	warn("Database must be locked during modifications");
	return(-1);
    }
    if ((fp = fopen(db_path, "w")) == NULL) {
	ewarn("fopen %s", db_path);
	return(-1);
    }
    putshort(string_table.cur, buf);
    if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fwrite");
	return(-1);
    }
    putshort(file_info_table.cur, buf);
    if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fwrite");
	return(-1);
    }
    putshort(file_table.cur, buf);
    if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fwrite");
	return(-1);
    }
    putshort(dir_table.cur, buf);
    if (fwrite(buf, sizeof(short), 1, fp) != 1) {
	ewarn("fwrite");
	return(-1);
    }
    errors = save_strings(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d strings ]", string_table.cur);
    errors = save_file_info(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d file_info ]", file_info_table.cur);
    errors = save_files(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d files ]", file_table.cur);
    errors = save_dirs(fp);
    if (errors != 0) {
	(void) fclose(fp);
	return(errors);
    }
    if (debug)
	diag("[ %d dirs ]", dir_table.cur);
    (void) fclose(fp);
    return(0);
}

lookup_component(filename, file_ptr_ptr, file_info_ptr_ptr)
char *filename;
struct file **file_ptr_ptr;
struct file_info **file_info_ptr_ptr;
{
    char file[BUFSIZ];
    char *p;
    char *part;
    struct string *part_string;
    struct file *file_ptr;
    struct dir *dir_ptr;
    struct file_info *file_info_ptr;
    int i;

    dir_ptr = dir_table.dtab[0];
    file_info_ptr = dir_ptr->info;
    *file_ptr_ptr = NULL;
    *file_info_ptr_ptr = NULL;
    (void) strcpy(file, filename);
    p = file;
    part = nxtarg(&p, "/");
    if (*part != '\0') {
	warn("%s: not absolute", filename);
	return(-1);
    }
    if (_argbreak == '\0') {
	warn("%s: is a directory", filename);
	return(-1);
    }
    for (;;) {
	part = nxtarg(&p, "/");
	if (create_string(part, &part_string) != 0) {
	    warn("Unable to create string '%s'", part);
	    return(-1);
	}
	if (_argbreak == '\0') {
	    if (dir_ptr->ftable.cur == 0) {
		warn("%s: not found", filename);
		return(-1);
	    }
	    i = binsearch((char *)dir_ptr->ftable.ftab,
			  dir_ptr->ftable.cur,
			  sizeof(struct file *),
			  part_string, check_file_name);
	    if (i == -1 || check_file_name(part_string,
					   &dir_ptr->ftable.ftab[i]) != 0) {
		warn("%s: not found", filename);
		return(-1);
	    }
	    file_ptr = dir_ptr->ftable.ftab[i];
	    if (file_ptr->info != NULL)
		file_info_ptr = file_ptr->info;
	    *file_ptr_ptr = file_ptr;
	    *file_info_ptr_ptr = file_info_ptr;
	    return(0);
	}
	if (dir_ptr->dtable.cur == 0) {
	    warn("%s: not found", filename);
	    return(-1);
	}
	i = binsearch((char *)dir_ptr->dtable.dtab,
		      dir_ptr->dtable.cur,
		      sizeof(struct dir *),
		      part_string, check_dir_name);
	if (i == -1 || check_dir_name(part_string,
				      &dir_ptr->dtable.dtab[i]) != 0) {
	    warn("%s: not found", filename);
	    return(-1);
	}
	dir_ptr = dir_ptr->dtable.dtab[i];
	if (dir_ptr->info != NULL)
	    file_info_ptr = dir_ptr->info;
    }
}

create_component(file, file_ptr_ptr, file_info_ptr_ptr)
char *file;
struct file **file_ptr_ptr;
struct file_info **file_info_ptr_ptr;
{
    return(insert_file(file, file_ptr_ptr, (struct dir **)NULL,
		       file_info_ptr_ptr));
}

set_component_info(file_ptr, file_info_ptr_ptr, srcdir, nlinks, links,
		   owner, group, mode)
struct file *file_ptr;
struct file_info **file_info_ptr_ptr;
char *srcdir;
int nlinks;
char **links;
struct string *owner, *group;
u_short mode;
{
    char buf[MAXPATHLEN];
    char *ptr;
    struct dir *dir_ptr;
    struct file *link_file_ptr;
    struct file_info *file_info_ptr;
    int i;

    ptr = concat(buf, sizeof(buf), srcdir, NULL);
    if (ptr == buf || *(ptr-1) != '/') {
	*ptr++ = '/';
	*ptr = '\0';
    }
    if (insert_file(buf, (struct file **)NULL, &dir_ptr,
		    (struct file_info **)NULL) < 0) {
	warn("Error inserting source directory");
	return(1);
    }
    file_ptr->srcdir = dir_ptr;
    if (dir_ptr != NULL)
	INCREF(dir_ptr);
    file_info_ptr = *file_info_ptr_ptr;
    if (file_info_ptr->owner != owner ||
	file_info_ptr->group != group ||
	file_info_ptr->mode != mode) {
	if (insert_info(owner, group, mode, &file_info_ptr) < 0) {
	    warn("Error inserting file info");
	    return(1);
	}
	if (file_ptr->info != NULL)
	    DECREF(file_ptr->info);
	file_ptr->info = file_info_ptr;
	if (file_info_ptr != NULL)
	    INCREF(file_info_ptr);
	*file_info_ptr_ptr = file_info_ptr;
    }
    while ((link_file_ptr = file_ptr->links) != NULL) {
	if (link_file_ptr == file_ptr) {
	    file_ptr->links = NULL;
	    break;
	}
	file_ptr->links = link_file_ptr->links;
	if (link_file_ptr->info != NULL) {
	    DECREF(link_file_ptr->info);
	    link_file_ptr->info = NULL;
	}
	DECREF(link_file_ptr);
    }
    for (i = 1; i < nlinks; i++) {
	if (insert_file(links[i], &link_file_ptr, (struct dir **)NULL,
			&file_info_ptr) != 0) {
	    warn("Error inserting file link");
	    return(1);
	}
	if (file_ptr->links == NULL) {
	    link_file_ptr->links = file_ptr;
	    INCREF(link_file_ptr->links);
	} else
	    link_file_ptr->links = file_ptr->links;
	file_ptr->links = link_file_ptr;
	INCREF(link_file_ptr);
	if (file_info_ptr->owner != owner ||
	    file_info_ptr->group != group ||
	    file_info_ptr->mode != mode) {
	    if (insert_info(owner, group, mode, &file_info_ptr) < 0) {
		warn("Error inserting file link info");
		return(1);
	    }
	    link_file_ptr->info = file_info_ptr;
	    if (file_info_ptr != NULL)
		INCREF(file_info_ptr);
	}
	file_ptr = link_file_ptr;
    }
    return(0);
}

initialize_database()
{
    int i;

    for (i = 0; i < HASHSIZE; i++)
	string_hashtable[i].sh_first = string_hashtable[i].sh_last =
	    &string_hashtable[i];
    (void) bzero((char *)&string_table, sizeof(string_table));
    (void) bzero((char *)&file_info_table, sizeof(file_info_table));
    (void) bzero((char *)&file_table, sizeof(file_table));
    (void) bzero((char *)&dir_table, sizeof(dir_table));
}

lock_database(db_path)
char *db_path;
{
    char buf[MAXPATHLEN];
    char *user;
    char *tty;
    char datebuf[MAXPATHLEN];
    char *ptr;
    struct tm *tm;
    time_t now;

    (void) concat(buf, sizeof(buf), buf, db_path, ".lock", NULL);
    for (;;) {
	if ((lock_fd = open(buf, O_RDWR|O_CREAT, 0600)) < 0)
	    ewarn("open %s", buf);
	else {
	    if (flock(lock_fd, LOCK_EX|LOCK_NB) == 0)
		break;
	    if (errno != EWOULDBLOCK)
		ewarn("flock");
	    else {
		diag("release database locked:");
		(void) putc('\t', stderr);
		(void) fflush(stderr);
		(void) filecopy(lock_fd, fileno(stderr));
		if (!getbool("Do you want to wait for the lock?", TRUE)) {
		    (void) close(lock_fd);
		    lock_fd = -1;
		    return(1);
		}
		diag("[ waiting for exclusive access lock ]");
		if (flock(lock_fd, LOCK_EX) == 0) {
		    diag("[ release database locked ]");
		    break;
		}
		ewarn("flock");
	    }
	    (void) close(lock_fd);
	    lock_fd = -1;
	}
	if (!getbool("Retry?", FALSE))
	    return(1);
    }
    if ((user = getenv("USER")) == NULL)
	user = "unknown";
    if ((tty = ttyname(0)) == NULL)
	tty = "tty??";
    now = time((time_t *) 0);
    tm = localtime(&now);
    fdate(datebuf, "%3Month %02day %year %02hour:%02min:%02sec", tm);
    ptr = concat(buf, sizeof(buf),
		 user, " on ", tty, " at ", datebuf, "\n", NULL);
    errno = 0;
    if (lseek(lock_fd, (long)0, L_SET) == -1 && errno != 0)
	ewarn("lseek");
    if (write(lock_fd, buf, ptr-buf) != ptr-buf)
	ewarn("write");
    if (ftruncate(lock_fd, (off_t)(ptr-buf)) < 0)
	ewarn("ftruncate");
    return(0);
}

unlock_database()
{
    (void) close(lock_fd);
    lock_fd = -1;
}
