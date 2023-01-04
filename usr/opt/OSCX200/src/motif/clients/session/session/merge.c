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
#include "smdata.h"
#include "smconstants.h"
#include <X11/Xatom.h>
#include <stdio.h>
#include <strings.h>
#include "merge.h"

Boolean mergedatabase = TRUE ;
#define free	XtFree
#define malloc  XtMalloc
#define realloc XtRealloc

typedef struct _Entry {
    char *tag, *value;
    Boolean usable;
    int index;
} Entry;
typedef struct _Buffer {
    char *buff;
    int  room, used;
} Buffer;
typedef struct _Entries {
    Entry *entry;
    int   room, used;
} Entries;

#define INIT_BUFFER_SIZE 100
#define INIT_ENTRY_SIZE 5

void InitBuffer(b)
    Buffer *b;
{
    b->room = INIT_BUFFER_SIZE;
    b->used = 0;
    b->buff = (char *)malloc(INIT_BUFFER_SIZE*sizeof(char));
}

void AppendToBuffer(b, str, len)
    register Buffer *b;
    char *str;
    register int len;
{
    while (b->used + len > b->room) {
	b->buff = (char *)realloc(b->buff, 2*b->room*(sizeof(char)));
	b->room *= 2;
    }
    strncpy(b->buff + b->used, str, len);
    b->used += len;
}

void InitEntries(e)
    Entries *e;
{
    e->room = INIT_ENTRY_SIZE;
    e->used = 0;
    e->entry = (Entry *)malloc(INIT_ENTRY_SIZE*sizeof(Entry));
}

void FreeEntries (e)
    Entries *e;
{
    int i;

    if (e == NULL) return;

    for(i = 0; i < e->used; i++) {
	XtFree(e->entry[i].tag);
	XtFree(e->entry[i].value);
    }

    e->room = INIT_ENTRY_SIZE;
    e->used = 0;
    e->entry = (Entry *)realloc((char *)e->entry, INIT_ENTRY_SIZE*sizeof(Entry));
}
 

void AddEntry(e, entry)
    register Entries *e;
    Entry entry;
{
    if (e->used == e->room) {
	e->entry = (Entry *)realloc((char *)e->entry, 2*e->room*(sizeof(Entry)));
	e->room *= 2;
    }
    entry.usable = TRUE;
    e->entry[e->used++] = entry;
}

void MikeyRemoveEntry(e, entry)  
    register Entries *e;
    Entry entry;
{
  int i, j;

  if (!e->used) return;

  i = 0;
  for (i=0; i<e->used; i++) {
   if (e->entry[i].usable && !strcmp(e->entry[i].tag, entry.tag)) {
     XtFree(e->entry[i].tag);
     XtFree(e->entry[i].value);
     for(j=i+1; j<e->used; j++) {
       e->entry[j-1] = e->entry[j];
     }
     e->used--;
     return;
    }
  }
}

int CompareEntries(e1, e2)
    Entry *e1, *e2;
{
    return strcmp(e1->tag, e2->tag);
}

static Entries MikeyRemoveList = {NULL, 0, 0};

void AppendEntryToBuffer(buffer, entry)
    register Buffer *buffer;
    Entry entry;
{
  int index;

  for(index = 0;index < MikeyRemoveList.used ; index++) {
    if(!strcmp(entry.tag, MikeyRemoveList.entry[index].tag)) {
      return;
    }
  }

    AppendToBuffer(buffer, entry.tag, strlen(entry.tag));
    AppendToBuffer(buffer, ":\t", 2);
    AppendToBuffer(buffer, entry.value, strlen(entry.value));
    AppendToBuffer(buffer, "\n", 1);
}

char *FindFirst(string, dest)
    register char *string;
    register char dest;
{
    for (;;) {
	if (*string == '\0')
	    return NULL;
	if (*string == '\\') {
	    if (*++string == '\0')
		return NULL;
	}
	else if (*string == dest)
	    return string;
	string++;
    }
}
static Entries *file_entries;

static 
void doEntry(line, flag, state)
register char *line;
int flag;
int state;
{
	register char *str = line;
	register char *colon, *temp;
	register int length;
	Entry entry;

	if(!flag) return; /* not visable */
	if (str[0] == '!') return;
	if (str[0] == '\n') return;
	if (str[0] == '#') return;
	colon = FindFirst(str, ':');
	if (colon == NULL) return;

	while(str[0] == ' ' || str[0] == '\t') str++;
	temp = (char *)malloc((length = colon - str) + 1);
	strncpy(temp, str, length);
	temp[length] = '\0';
	while (temp[length-1] == ' ' || temp[length-1] == '\t')
	    temp[--length] = '\0';
	entry.tag = temp;

	colon++;
	while(colon[0] == ' ' || colon[0] == '\t') colon++;
	
	temp = (char *)malloc((length = strlen(colon)) + 1);
	strncpy(temp, colon , length);
	temp[length] = '\0';
	while (temp[length-1] == ' ' || temp[length-1] == '\t')
	    temp[--length] = '\0';
	entry.value = temp;

	AddEntry(file_entries, entry);
}

void GetEntries(entries, buff)
    register Entries *entries;
    Buffer *buff;
{
    register char *line, *colon, *temp, *str;
    Entry entry;
    register int length;

    str = buff->buff;
    for (str = buff->buff; str < buff->buff + buff->used ;str = line + 1) {
	line = FindFirst(str, '\n');
	if (line == NULL)
	    break; 
	if (str[0] == '!')
	    continue;
	if (str[0] == '\n')
	    continue;
	if (str[0] == '#')
	    continue;
	colon = FindFirst(str, ':');
	if (colon == NULL)
	    break;
	if (colon > line) {
	    line[0] = '\0';
	    fprintf(stderr, "line missing colon ignored: %s\n", str);

	    continue;
	}

	while(str[0] == ' ' || str[0] == '\t') str++;
	temp = (char *)malloc((length = colon - str) + 1);
	strncpy(temp, str, length);
	temp[length] = '\0';
	while (temp[length-1] == ' ' || temp[length-1] == '\t')
	    temp[--length] = '\0';
	entry.tag = temp;

	colon++;
	while(colon[0] == ' ' || colon[0] == '\t') colon++;
	temp = (char *)malloc((length = line - colon) + 1);
	strncpy(temp, colon , length);
	temp[length] = '\0';
	while (temp[length-1] == ' ' || temp[length-1] == '\t')
	    temp[--length] = '\0';
	entry.value = temp;

	AddEntry(entries, entry);
    }
    qsort(entries->entry, entries->used, sizeof(Entry), CompareEntries);
}

int MergeEntries(buffer, new, old)
    Entries new, old;
    register Buffer *buffer;
{
    register int n, o, cmp;

    n = o = 0;
    while ((n < new.used) && (o < old.used)) {
	cmp = strcmp(new.entry[n].tag, old.entry[o].tag);
	if (cmp > 0)
	    AppendEntryToBuffer(buffer, old.entry[o++]);
	else {
	    AppendEntryToBuffer(buffer, new.entry[n++]);
	    if (cmp == 0)
		o++;
	}
    }
    while (n < new.used)
	AppendEntryToBuffer(buffer, new.entry[n++]);
    while (o < old.used)
	AppendEntryToBuffer(buffer, old.entry[o++]);
    AppendToBuffer(buffer, "", 1);
}

static Entries SaveList = {NULL, 0, 0};
#define NEWCOPY(s)   (strcpy(XtMalloc(strlen(s)+1),s))
void
PhilSave(name, value, Index)
register char *name;
char *value;
int Index;
{
	register int index;
	Entry entry;
	static Boolean first = TRUE;

	if(first == TRUE) {
		InitEntries(&SaveList);
		first = FALSE;
	}
	for(index = 0;index < SaveList.used ; index++) {
		if(!strcmp(name,SaveList.entry[index].tag)) {
			XtFree(SaveList.entry[index].value);
			SaveList.entry[index].value = NEWCOPY(value);
			return;
		}
	}
	entry.tag = NEWCOPY(name);
	entry.value = NEWCOPY(value);
	entry.index = Index;
	AddEntry(&SaveList, entry);
	MikeyRemoveEntry(&MikeyRemoveList, entry);
}

MikeyRemove(name, Index)
register char *name;
int Index;
{
	char *value = "";
	register int index;
	Entry entry;
	static Boolean first = TRUE;

	if(first == TRUE) {
		InitEntries(&MikeyRemoveList);
		first = FALSE;
	}
	for(index = 0;index < MikeyRemoveList.used ; index++) {
		if(!strcmp(name,MikeyRemoveList.entry[index].tag)) {
			XtFree(MikeyRemoveList.entry[index].value);
			MikeyRemoveList.entry[index].value = NEWCOPY(value);
			return;
		}
	}
	entry.tag = NEWCOPY(name);
	entry.value = NEWCOPY(value);
	entry.index = Index;
	AddEntry(&MikeyRemoveList, entry);
	MikeyRemoveEntry(&SaveList, entry);
}

void 
FreeSaveList ()
{
    FreeEntries (&SaveList);
}

void 
FreeRemoveList ()
{
    FreeEntries (&MikeyRemoveList);
}


#define SINGLEREAD	100000
do_merge_to_server(dpy)
Display *dpy;
{
        unsigned char *buffer;
	Buffer buff;
	Buffer result;
	Entries entries;
	Atom actual_type;
	int format;
	unsigned long len, remainder;

	XGetWindowProperty(dpy, DefaultRootWindow(dpy), XA_RESOURCE_MANAGER,
			   0, SINGLEREAD, FALSE, XA_STRING, &actual_type,
			   &format, &len, &remainder, &buffer);
	if(remainder != 0) {
		fprintf(stderr, "You lost truncating property at %d\n", len);
	}
	if(actual_type == None) {
		len = 0;
	}
	InitEntries(&entries);
	buff.buff = (char *)buffer;
	buff.room = buff.used = len;
	GetEntries(&entries, &buff);  /* makes string in to entries */
	qsort(SaveList.entry, SaveList.used, sizeof(Entry), CompareEntries);
	InitBuffer(&result);
	MergeEntries(&result, SaveList, entries );
	/* should be freeing here */
	XChangeProperty (dpy ,DefaultRootWindow(dpy), XA_RESOURCE_MANAGER,
			 XA_STRING, 8, PropModeReplace,
			 (unsigned char *)result.buff, result.used);
	XtFree(result.buff);
	XtFree((char *)entries.entry);
	XFree(buffer);

}

Entry *FindEntry(db, b)
    register Entries *db;
    Buffer *b;
{
    register char *colon;
    register int i;
    register Entry *e;
    Entries phoney;
    Entry entry;

    phoney.used = 0;
    phoney.room = 1;
    phoney.entry = &entry;
    GetEntries(&phoney, b);
    if (phoney.used == 0) return (NULL);

    for (i = 0; i < db->used; i++) {
	e = &db->entry[i];
	if (!e->usable)
	    continue;
	if (strcmp(e->tag, entry.tag))
	    continue;
	e->usable = False;
	if (strcmp(e->value, entry.value))
	    return e;
	return NULL;
    }
    return NULL;
}

static FILE *out;
static void doEdit(name, flag, state)
char *name;
int flag;
int state;
{
	static Buffer b;
	static int first = 1;
	register Entries *new;
	register Entry *e;
	register int index;

	if(!flag) {
		fprintf(out, "%s\n",name);
		return;
	}
	if(first) {
		InitBuffer(&b);
		first = 0;
	}

	for(index = 0;index < MikeyRemoveList.used ; index++) {
	  char *colon;

	  colon = FindFirst(name, ':');
	  if(colon && !strncmp(name, MikeyRemoveList.entry[index].tag,
			       colon-name)) {
	    return;
	  }
	}

	new = &SaveList;
	b.used = 0;
	AppendToBuffer(&b, name, index = strlen(name));
	if(name[index-1] != '\n')
	  AppendToBuffer(&b,"\n",1);

	if (e = FindEntry(new, &b)) {
		fprintf(out, "%s:\t%s\n", e->tag, e->value);
		e->usable = FALSE; /* prevent rematching on this */
	}
	else
	  fwrite(b.buff, 1, b.used, out);
	if(b.buff[b.used-1] != '\n')
	  fwrite("\n",1,1,out);
}

do_merge_database()
{
	char *tmplate = ".XdefaultsXXXXXX";
	char buf[100];
	register int i;
	register Entry *e;
	register int first = 1;

	out = fopen(mktemp(strcpy(buf,tmplate)), "w");
	if (out == NULL) return;

	qsort(SaveList.entry, SaveList.used, sizeof(Entry), CompareEntries);
	readfile(".Xdefaults", doEdit, 0);
	for (i = 0; i < SaveList.used; i++) {/* color or non color entries */
		e = &SaveList.entry[i];
		if (e->usable && e->index == rdb_color) {
			if(first) {
				if(system_color_type == color_system) 
				  fprintf(out,"#ifdef COLOR\n");
				else
				  fprintf(out, "#ifndef COLOR\n");
				first = 0;
			}
			e->usable = FALSE;
			fprintf(out, "%s:\t%s\n", e->tag, e->value);
		}
	}
	if(first == 0)
	  fprintf(out, "#endif\n");
	for (i = 0; i < SaveList.used; i++) { /* generic entries */
		e = &SaveList.entry[i];
		if (e->usable) {
			fprintf(out, "%s:\t%s\n", e->tag, e->value);
		} else {
			e->usable = TRUE;
		}
	}
	fclose(out);
	rename(".Xdefaults", ".Xdefaults.old");
	rename(buf, ".Xdefaults");
}

reset_property(dpy, filename)
Display *dpy;
char *filename;
{
	FILE *fp;
	Buffer buffer;
	register int i; 
	Entries newDB;
	Entries oldDB;

	InitBuffer(&buffer);
	InitEntries(&newDB);
	InitEntries(&oldDB);

	file_entries = &newDB;
	if(readfile(filename, doEntry, QUIET) == 0)
	  return(0);
	qsort(newDB.entry, newDB.used, sizeof(Entry),  CompareEntries);
	buffer.used = 0;
	MergeEntries(&buffer, newDB, oldDB);
	XChangeProperty (dpy, RootWindow(dpy, 0), XA_RESOURCE_MANAGER,
			 XA_STRING, 8, PropModeReplace,
			 (unsigned char *)buffer.buff, buffer.used);
	XtFree(buffer.buff);
	for(i = 0; i < newDB.used; i++) {
		XtFree(newDB.entry[i].tag);
		XtFree(newDB.entry[i].value);
	}
	XtFree((char *)newDB.entry);
	XtFree((char *)oldDB.entry);
	return(1);
}
