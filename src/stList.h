/* DannyNiu/NJF, 2023-07-16. Public Domain. */

#ifndef SafeTypes_List_H
#define SafeTypes_List_H 1

#include "stData.h"
#include "stContainer.h"

typedef struct stList stList_t;

struct st_list_member {
    stList_t *collection;
    stObj_t *value;
    struct st_list_member *prev;
    struct st_list_member *next;
};

struct stList {
    stObj_t hdrObj;
    size_t len;
    size_t pos;
    struct st_list_member *cursor;
    struct st_list_member anch_head;
    struct st_list_member anch_tail;
};

stList_t *stListInit(stList_t *list);

// - Inserts an element at the cursor without advancing it.
int stListInsert(stList_t *list, stObj_t *obj, int semantic);

// - Inserts an element at the cursor and advance it.
int stListPush(stList_t *list, stObj_t *obj, int semantic);

// for ``stListShift'' and ``stListPop'':
// ``refcnt'' is incremented and ``keptcnt'' decremented,
// both by 1 - because they're no longer in the list
// (at least not from where they used to be).

// - Removes the item at the cursor from the list and place it in ``*out''.
int stListShift(stList_t *list, stObj_t **out);

// - Removes the item just before the cursor and place it in ``*out''.
// - Implementation note: this function is found redundant and anti-logical,
//   adding the fact that it's implemented in terms of ``stListShift'', this
//   function is probably inefficient and its use is better avoided.
int stListPop(stList_t *list, stObj_t **out);

int stListIterate(stList_t *list);

// Repositions the cursor.
#define ST_LIST_SEEK_SET 1
#define ST_LIST_SEEK_END 2
#define ST_LIST_SEEK_CUR 3
ptrdiff_t stListSeek(stList_t *list, ptrdiff_t offset, int whence);

typedef int (*stListSort_CmpFunc_t)(stObj_t *a, stObj_t *b);
stList_t *stListSort(stList_t *list, stListSort_CmpFunc_t cmpfunc);

void stListFinal(stList_t *list);

#endif /* SafeTypes_Dict_H */
